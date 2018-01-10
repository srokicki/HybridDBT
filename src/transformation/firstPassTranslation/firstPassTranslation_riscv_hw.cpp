/*
 * firstPassTranslatorRISCV.cpp
 *
 *  Created on: 3 déc. 2016
 *      Author: Simon Rokicki
 *
 * This file contains the description of the first pass translator targeting RISCV ISA
 * This file is made to work in several platofm (x86, nios, and Catapult).
 * It use several ifdef:
 * 	__CATAPULT is set when opened with catapult
 * 	__NIOS is set when executed on Nios
 */

#ifndef __CATAPULT
//Includes that are not used in catapult
#include <cstdio>
#include <cstdlib>

#include <lib/endianness.h>
#include <lib/tools.h>
#include <dbt/insertions.h>
#endif

//Includes used in Catapult
#include <isa/riscvISA.h>
#include <isa/vexISA.h>
#include <isa/riscvToVexISA.h>
#include <types.h>

#include <transformation/reconfigureVLIW.h>
#include <transformation/firstPassTranslation.h>



#ifndef __NIOS

int firstPassTranslator_riscv_hw(ac_int<32, false> code[1024],
		ac_int<32, false> size,
		ac_int<8, false> conf,
		ac_int<32, false> addressStart,
		ac_int<32, false> codeSectionStart,
		ac_int<128, false> destinationBinaries[1024],
		ac_int<32, false> placeCode,
		ac_int<32, false> insertions[256],
		ac_int<1, false> blocksBoundaries[65536],
		ac_int<32, false> unresolvedJumps_src[512],
		ac_int<32, false> unresolvedJumps_type[512],
		ac_int<32, true> unresolvedJumps[512]){


	/**
	 * Idea of the procedure:
	 * The procedure will go through all instruction of the MIPS binaries and translate them into VLIW binaries.
	 * This translation is naive and do not try to exploit ILP.
	 *
	 * There are two important variables used in the main loop:
	 *   - indexInSourceBinaries: index of the MIPS binary instruction being treated. This variable is not increased
	 *     when we meet a duplicated instruction (eg. a MIPS instruction turned into two VLIW instructions).
	 *     Initial value is zero.
	 *   - indexInDestinationBinaries: index if the written VLIW instruction. This variable is not updated when we meet
	 *     a dropped instruction (eg. an instruction that will not be translated in the VLIW binaries, like the
	 *     mult instruction which will be translated when a mfhi/mflo is met). Initial value is zero.
	 *
	 * Loop also compute other information:
	 *   - Data on the loop and procedure boundaries: This is information on the limits of all blocks and procedures.
	 *     Effectively, each time we meet a jump/branch, we store the possible destinations as starting points of
	 *     blocks. For example, a JUMP instruction will lead to a single starting point at the targeted address.
	 *     On the contrary, a conditional branch will generate two starting points, one at the destination address and
	 *     the other one after the branch.
	 *     WARNING: on MIPS binaries, the instruction following the jump is executed so we have to place the limit at
	 *     the correct place.
	 *     This data is collected using the variables 'blocksBoundaries' and 'proceduresBoundaries'.
	 *     NOTE: the place where the block boundary is placed use the index in SOURCE binaries.
	 *
	 *   - Data on the unresolved jumps: As we cannot know the destination of jumps instruction we meet, we will
	 *     mark them as unresolved during generation. When the complete binaries are generated, we will go through
	 *     these jumps and solved the address, taking into account inserted and suppressed instructions.
	 *     Two variables are used to keep track of insertions and suppressions: 'insertions' and 'numberInsertions'.
	 *     Note that a negative value in 'insertion' is the place of a suppressed instruction.
	 *     NOTE: values in insertions are the place of the inserted instruction in DESTINATION binaries.
	 *     Four other variables are used to register unresolved jumps: 'unresolvedJumps_src' is the address in DESTINATION
	 *     binaries of the jump to solve, 'unresolvedJumps_type' contain information on the jump type (absolute or
	 *     relative), 'unresolvedJumps' contains the destination address in the SOURCE binaries and numberUnresolvedJumps
	 *     contains the number of jumps being registered.
	 *
	 */
	unsigned int numberUnresolvedJumps = 0;
	char issueWidth = getIssueWidth(conf);

	char stageMem = ((conf & 0xf) == 1) ? 2 : ((conf & 0xf) == 3) ? 6 : 1;
	char stageMult = ((conf & 0xf) == 1) ? 3 : ((conf & 0xf) == 3) ? 2 : 1;


	int indexInSourceBinaries = 0;
	int indexInDestinationBinaries = placeCode;


	ac_int<32, 0> nextInstruction, nextInstruction_stage;
	ac_int<6, false> nextInstruction_rs1, nextInstruction_rs2, nextInstruction_rd;
	ac_int<6, false> secondNextInstruction_rs1, secondNextInstuction_rs2, secondNextInstruction_rd;

	ac_int<32, 0> secondNextInstruction, secondNextInstruction_stage;

	unsigned char enableNextInstruction = 0;
	unsigned char enableSecondNextInstruction = 0;

	unsigned char nextInstructionNop = 0;


	ac_int<5, false> reg1_mul = 0, reg2_mul = 0;
	ac_int<16, false> imm_mul = 0;
	ac_int<1, false> is_imm_mul = 0;


	blocksBoundaries[(codeSectionStart-addressStart)>>2] = 1;

	ac_int<256,false> previousBinaries = 0;
	ac_int<32, false> previousIndex = 0;
	ac_int<8, true> previousStage = 0;
	ac_int<32, false> localNumberInsertions = 0;

	ac_int<1, false> currentBoundaryJustSet = 0;

	ac_int<1, false> setNextBoundaries = 0;
	ac_int<32, true> nextBoundaries;

	ac_int<7, false> previousWrittenRegister = 0;
	ac_int<7, false> lastWrittenRegister = 0;
	ac_int<3, false> lastLatency = 0;
	ac_int<3, false> previousLatency = 0;

	char incrementInDest = (issueWidth>4) ? 2:1;


	while (indexInSourceBinaries < size || nextInstructionNop || enableNextInstruction){

		ac_int<1, false> setBoundaries1 = 0, setBoundaries2 = 0, setUnresolvedJump = 0;
		ac_int<32, true> boundary1, boundary2, unresolved_jump_src, unresolved_jump_type;

		ac_int<1, false> isInsertion = 0;

		char stage = 0;
		ac_int<32, false> binaries = 0;

		char wasExternalInstr = 0;
		char droppedInstruction = 0;


		/*
		 * First we load the instruction to be analyzed at this cycle.
		 * This instruction can come from previous iteration which may need two VEX instruction to
		 * be equivalent to the original instruction or simply an instruction from the source binaries.
		 *
		 * Code also authorize to insert NOP instruction in order to handle cases where latency is too high.
		 * Currently this will never happen...
		 */

		if (enableNextInstruction){

			if ((nextInstruction_rs1 == lastWrittenRegister && lastLatency != 0) || (nextInstruction_rs2 == lastWrittenRegister && lastLatency != 0)
					|| (nextInstruction_rs1 == previousWrittenRegister && previousLatency != 0) || (nextInstruction_rs2 == previousWrittenRegister && previousLatency != 0)){
				binaries = 0;
				enableNextInstruction = 1;
				wasExternalInstr = 1;
				isInsertion = 1;
			}
			else{
				binaries = nextInstruction;
				stage = nextInstruction_stage;
				wasExternalInstr = 1;
				enableNextInstruction = 0;
				isInsertion = 1;

				lastWrittenRegister = nextInstruction_rd;
				lastLatency = SIMPLE_LATENCY;

				nextInstruction_rd = 0;
				nextInstruction_rs1 = 0;
				nextInstruction_rs2 = 0;

				if (enableSecondNextInstruction){
					enableSecondNextInstruction = 0;
					enableNextInstruction = 1;
					nextInstruction = secondNextInstruction;
					nextInstruction_rd = secondNextInstruction_rd;
					nextInstruction_rs1 = secondNextInstruction_rs1;
					nextInstruction_rs2 = secondNextInstuction_rs2;


					nextInstruction_stage = secondNextInstruction_stage;
				}
			}
		}
		else if (nextInstructionNop){
			binaries = 0;
			stage = 0;
			wasExternalInstr = 1;
			nextInstructionNop = 0;
			isInsertion = 1; //A nop instruction is an insertion...
			setBoundaries1 = setNextBoundaries;
			boundary1 = nextBoundaries;
			setNextBoundaries = 0;
		}
		else{

			ac_int<32, false> oneInstruction = code[indexInSourceBinaries];

			/**************************************************************
			*  Instruction decoding
			*
			*  First step of the process is the instruction decoding where we build
			*  each part of the instruction. These part will be used later on in
			*  order to build the new instruction.
			***************************************************************/
			ac_int<7, false> opcode = oneInstruction.slc<7>(0);
			ac_int<7, false> rs1 = oneInstruction.slc<5>(15);
			ac_int<7, false> rs2 = oneInstruction.slc<5>(20);
			ac_int<7, false> rs3 = oneInstruction.slc<5>(27);

			ac_int<6, false> rd = oneInstruction.slc<5>(7);
			ac_int<7, false> funct7 = oneInstruction.slc<7>(25);
			ac_int<7, false> funct7_smaller = 0;
			funct7_smaller.set_slc(1, oneInstruction.slc<6>(26));



			ac_int<3, false> funct3 = oneInstruction.slc<3>(12);
			ac_int<12, false> imm12_I = oneInstruction.slc<12>(20);
			ac_int<12, false> imm12_S = 0;
			imm12_S.set_slc(5, oneInstruction.slc<7>(25));
			imm12_S.set_slc(0, oneInstruction.slc<5>(7));

			ac_int<12, true> imm12_I_signed = oneInstruction.slc<12>(20);
			ac_int<12, true> imm12_S_signed = 0;
			imm12_S_signed.set_slc(0, imm12_S.slc<12>(0));


			ac_int<13, false> imm13 = 0;
			imm13[12] = oneInstruction[31];
			imm13.set_slc(5, oneInstruction.slc<6>(25));
			imm13.set_slc(1, oneInstruction.slc<4>(8));
			imm13[11] = oneInstruction[7];

			ac_int<13, true> imm13_signed = 0;
			imm13_signed.set_slc(0, imm13);

			ac_int<32, false> imm31_12 = 0;
			imm31_12.set_slc(12, oneInstruction.slc<20>(12));

			ac_int<32, true> imm31_12_signed = 0;
			imm31_12_signed.set_slc(0, imm31_12);

			ac_int<21, false> imm21_1 = 0;
			imm21_1.set_slc(12, oneInstruction.slc<8>(12));
			imm21_1[11] = oneInstruction[20];
			imm21_1.set_slc(1, oneInstruction.slc<10>(21));
			imm21_1[20] = oneInstruction[31];
			ac_int<21, true> imm21_1_signed = 0;
			imm21_1_signed.set_slc(0, imm21_1);

			ac_int<6, false> shamt = oneInstruction.slc<6>(20);

			ac_int<26, false> correctedTgtadr = imm21_1 - (addressStart>>2);


			if (rs1 == 1)
				rs1 = 63;

			if (rs2==1)
				rs2=63;

			if (rd==1)
				rd=63;

			if (opcode == RISCV_FMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMADD || opcode == RISCV_FNMSUB ||
					opcode == RISCV_FP && (funct7 != RISCV_FP_FCVTW && funct7 != RISCV_FP_FMVW)){
				rs1 += 64;
			}

			/***************************************************************/
			/*  We assemble the instruction  							   */
			/***************************************************************/


			//We compute a bit saying if previous instruction is at BB boundary
			ac_int<1, false> previousIsBoundary = 0;
			ac_int<32, false> currentAddress = (indexInSourceBinaries + ((codeSectionStart - addressStart)>>2));
			ac_int<16, false> offset = currentAddress.slc<16>(0);
			previousIsBoundary = blocksBoundaries[offset];
			if (currentBoundaryJustSet)
				previousIsBoundary = currentBoundaryJustSet;

			previousIsBoundary = previousIsBoundary || (indexInSourceBinaries == 0);
			currentBoundaryJustSet = 0;

			if ((rs1 == lastWrittenRegister && lastLatency != 0) || (rs2 == lastWrittenRegister && lastLatency != 0)
					|| (rs1 == previousWrittenRegister && previousLatency != 0) || (rs2 == previousWrittenRegister && previousLatency != 0)){
				binaries = 0;
				enableNextInstruction = wasExternalInstr;
				wasExternalInstr = 1;
				isInsertion = 1;
			}
			else if (opcode == RISCV_OP){
				//Instrucion io OP type: it needs two registers operand (except for rol/sll/sr)

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;

				if (funct7 == RISCV_OP_M){
					//We are in the part dedicated to RV32M extension
					binaries =  assembleRInstruction(functBindingMULT[funct3], rd, rs1, rs2);
					stage = stageMult;

					//nextInstructionNop = 1;
					//TODO: should certainly insert a nop
					lastLatency = MULT_LATENCY;
				}
				else if (funct3 == RISCV_OP_SLL || funct3 == RISCV_OP_SR){
					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					ac_int<7, false> vexOpcode = (funct3==RISCV_OP_SLL) ? VEX_SLL :
							(funct7==RISCV_OP_SR_SRA) ? VEX_SRA : VEX_SRL;

					binaries = assembleRInstruction(vexOpcode, rd, rs1, rs2);
					lastLatency = SIMPLE_LATENCY;
				}
				else {
					ac_int<7, false> subOpcode = VEX_SUB;
					ac_int<7, false> vexOpcode = (funct7==RISCV_OP_ADD_SUB) ? subOpcode : functBindingOP[funct3];
					binaries =  assembleRInstruction(vexOpcode, rd, rs1, rs2);
					lastLatency = SIMPLE_LATENCY;
				}


			}
			else if (opcode == RISCV_AUIPC){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				ac_int<32, false> value = codeSectionStart + (indexInSourceBinaries<<2) + imm31_12_signed;

				if (previousIsBoundary){

					binaries = assembleIInstruction(VEX_MOVI, value.slc<19>(12), rd);
					nextInstruction = assembleRiInstruction(VEX_SLLi, rd, rd, 12);
					secondNextInstruction = assembleRiInstruction(VEX_ADDi, rd, rd, value.slc<12>(0));

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					secondNextInstruction_rd = rd;
					secondNextInstruction_rs1 = rd;
					secondNextInstuction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = 1;
					enableSecondNextInstruction = 1;
					secondNextInstruction_stage = 0;

				}
				else{
					binaries = assembleIInstruction(VEX_MOVI, value.slc<19>(12), rd);
					nextInstruction = assembleRiInstruction(VEX_SLLi, rd, rd, 12);
					secondNextInstruction = assembleRiInstruction(VEX_ADDi, rd, rd, value.slc<12>(0));

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					secondNextInstruction_rd = rd;
					secondNextInstruction_rs1 = rd;
					secondNextInstuction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = 1;
					enableSecondNextInstruction = 1;
					secondNextInstruction_stage = 0;
				}

			}
			else if (opcode == RISCV_LUI){
				//Operation Load Upper Immediate is turned into a MOVI and a SLLi instructions.
				//If the instruction is not at the start point of a block, we can add the movi at the previous
				//cycle to prevent an insertion. Otherwise, we have to do the insertion...

				//TODO RISCV immediates are on 20 bits whereas we only handle 19-bit immediates...

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				ac_int<1, false> keepHigher = 0;
				ac_int<1, false> keepBoth = 0;

				if (imm31_12[31] != imm31_12[30]){
					keepHigher = 1;
					if (imm31_12[12])
						keepBoth = 1;
				}


				ac_int<19, false> immediateValue = keepHigher ? imm31_12.slc<19>(13): imm31_12.slc<19>(12);
				ac_int<5, false> shiftValue = keepHigher ? 13 : 12;

				ac_int<32, false> instr1 = assembleIInstruction(VEX_MOVI, immediateValue, rd);
				ac_int<32, false> instr2 = assembleRiInstruction(VEX_SLLi, rd, rd, shiftValue);
				ac_int<32, false> instr3 = assembleRiInstruction(VEX_ADDi, rd, rd, 0x1000);


				if (previousIsBoundary){
					binaries = instr1;
					nextInstruction = instr2;

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = 1;

					if (keepBoth){
						secondNextInstruction_rd = rd;
						secondNextInstruction_rs1 = rd;
						secondNextInstuction_rs2 = 0;

						secondNextInstruction = instr3;
						enableSecondNextInstruction = 1;
						secondNextInstruction_stage = 0;
					}

				}
				else{
					binaries = instr1;
					nextInstruction = instr2;

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = 1;

					if (keepBoth){
						secondNextInstruction_rd = rd;
						secondNextInstruction_rs1 = rd;
						secondNextInstuction_rs2 = 0;

						secondNextInstruction = instr3;
						enableSecondNextInstruction = 1;
						secondNextInstruction_stage = 0;
					}
				}

			}
			else if (opcode == RISCV_LD){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = MEMORY_LATENCY;

				//Memory access operations.
				binaries = assembleRiInstruction(functBindingLD[funct3], rd, rs1, imm12_I_signed);
				stage = stageMem;


			}
			else if (opcode == RISCV_ST){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = 0;

				binaries = assembleRiInstruction(functBindingST[funct3], rs2, rs1, imm12_S_signed);
				stage = stageMem;
			}
			else if (opcode == RISCV_JAL){

				//If rsd is equal to zero, then we are in a simple J instruction
				ac_int<1, false> isSimpleJ = (rd==0);
				binaries= assembleIInstruction(isSimpleJ ? VEX_GOTO : VEX_CALL, 0, rd);

				//We fill information on block boundaries
				setBoundaries1 = 1;
				boundary1 = indexInSourceBinaries + (imm21_1_signed>>2);
				setBoundaries2 = 1;
				boundary2 = indexInSourceBinaries + 1; //Only plus one because in riscv next instr is not executed

				unresolved_jump_src = indexInDestinationBinaries;
				unresolved_jump_type = binaries;
				setUnresolvedJump = 1;

				//In order to deal with the fact that RISCV do not execute the isntruction following a branch,
				//we have to insert nop
				nextInstructionNop = 1;

			}
			else if (opcode == RISCV_JALR){

				//JALR instruction is a bit particular: it load a value from a register and add it an immediate value.
				//Once done, it jump and store return address in rd.

				setBoundaries1 = 1;
				boundary1 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed

				if (rs1 == 63 && rd == 0){
					//We are in a simple return
					binaries = assembleIInstruction(VEX_GOTOR, imm12_I_signed, 63);

				}
				else{
					//FIXME should be able to add two instr at the same cycle... This would remove an insertion
					binaries = assembleRiInstruction(VEX_ADDi, 33, rs1, imm12_I_signed);

					nextInstruction = assembleIInstruction((rd == 63) ? VEX_CALL : VEX_GOTO, 4*incrementInDest, rd);
					enableNextInstruction = 1;
					nextInstruction_rd = 0;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;
				}


				//In order to deal with the fact that RISCV do not execute the isntruction following a branch,
				//we have to insert nop
				nextInstructionNop = 1;


			}
			else if (opcode == RISCV_BR){

				//While handling BR instruction we distinguish the case where we compare to zero cause VEX has
				// some special instruction to handle this...

				//First we fill information on block boundaries

				if (rs2 == 0 && (funct3 == RISCV_BR_BEQ || funct3 == RISCV_BR_BNE)){
					//This is a comparison to zero
					setBoundaries1 = 1;
					boundary1 = ((imm13_signed>>2)+indexInSourceBinaries);
					setBoundaries2 = 1;
					boundary2 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed

					binaries = assembleIInstruction((funct3 == RISCV_BR_BEQ) ? VEX_BRF : VEX_BR, rs2, rs1);

					unresolved_jump_src = indexInDestinationBinaries;
					unresolved_jump_type = binaries;
					setUnresolvedJump = 1;
				}
				else{

					previousLatency = lastLatency;
					previousWrittenRegister = lastWrittenRegister;
					lastWrittenRegister = 32;
					lastLatency = SIMPLE_LATENCY;

					setBoundaries1 = 1;
					boundary1 = ((imm13_signed>>2)+indexInSourceBinaries);
					setBoundaries2 = 1;
					boundary2 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed


					binaries = assembleRInstruction(functBindingBR[funct3], 32, rs1, rs2); //TODO check order

					nextInstruction = assembleIInstruction(VEX_BR, 0, 32);


					nextInstruction_stage = 0;
					enableNextInstruction = 1;
					nextInstruction_rd = 0;
					nextInstruction_rs1 = 32;
					nextInstruction_rs2 = 0;

					unresolved_jump_src = indexInDestinationBinaries+2*incrementInDest;
					unresolved_jump_type = nextInstruction;
					setUnresolvedJump = 1;


				}

				//In order to deal with the fact that RISCV do not execute the isntruction following a branch,
				//we have to insert nop
				nextInstructionNop = 1;


			}
			else if (opcode == RISCV_OPI){ //For all other instructions

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				ac_int<13, true> extendedImm = imm12_I_signed;
				if (funct3 == RISCV_OPI_SLTIU)
					extendedImm = imm12_I;


				if (funct3 == RISCV_OPI_SLLI || funct3 == RISCV_OPI_SRI){

					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					ac_int<7, false> vexOpcode = (funct3==RISCV_OPI_SLLI) ? VEX_SLLi :
							(funct7_smaller==RISCV_OPI_SRI_SRAI) ? VEX_SRAi : VEX_SRLi;

					binaries = assembleRiInstruction(vexOpcode, rd, rs1, shamt);


				}
				else {
					binaries = assembleRiInstruction(functBindingOPI[funct3], rd, rs1, extendedImm);
				}
			}
			else if (opcode == RISCV_OPW){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				if (funct7 == RISCV_OP_M){
					//We are in the part dedicated to RV64M extension
					binaries =  assembleRInstruction(functBindingMULTW[funct3], rd, rs1, rs2);
					stage = stageMult;

					nextInstructionNop = 1;

				}
				else if (funct3 == RISCV_OPW_SLLW || funct3 == RISCV_OPW_SRW){
					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					ac_int<7, false> vexOpcode = (funct3==RISCV_OPW_SLLW) ? VEX_SLLW :
							(funct7==RISCV_OPW_SRW_SRAW) ? VEX_SRAW : VEX_SRLW;

					binaries = assembleRInstruction(vexOpcode, rd, rs1, rs2);

				}
				else {


					ac_int<7, false> vexOpcode = (funct7==RISCV_OPW_ADDSUBW_SUBW) ? VEX_SUBW : VEX_ADDW;
					binaries =  assembleRInstruction(vexOpcode, rd, rs1, rs2);
				}
			}
			else if (opcode == RISCV_OPIW){ //For 32-bits instructions (labelled with a W)

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				ac_int<13, true> extendedImm = imm12_I_signed;
				if (funct3 == RISCV_OPI_SLTIU)
					extendedImm = imm12_I;


				if (funct3 == RISCV_OPIW_SLLIW || funct3 == RISCV_OPIW_SRW){

					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					ac_int<7, false> vexOpcode = (funct3==RISCV_OPIW_SLLIW) ? VEX_SLLWi :
							(funct7==RISCV_OPIW_SRW_SRAIW) ? VEX_SRAWi : VEX_SRLWi;

					binaries = assembleRiInstruction(vexOpcode, rd, rs1, shamt.slc<5>(0));


				}
				else {
					binaries = assembleRiInstruction(VEX_ADDWi, rd, rs1, extendedImm);
				}
			}
			else if (opcode == RISCV_SYSTEM){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = 10;
				lastLatency = SIMPLE_LATENCY;

				if (funct3 == RISCV_SYSTEM_ENV){
					binaries = assembleIInstruction(VEX_ECALL, 0,0);
				}
				else {

					#ifndef __CATAPULT
					binaries = assembleIInstruction(VEX_NOP, 0,0);

					#endif
				}
			}
			else if (opcode == RISCV_FLW){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = 64+rd; //the written reg is a float reg
				lastLatency = MEMORY_LATENCY;
//TODO
				if (funct3 == 4)
					binaries = assembleRiInstruction(VEX_FLW, rd, rs1, imm12_I_signed);
				else if (funct3 == 4)
					binaries = assembleRiInstruction(VEX_FLH, rd, rs1, imm12_I_signed);
				else
					binaries = assembleRiInstruction(VEX_FLB, rd, rs1, imm12_I_signed);

				stage = stageMem;
			}
			else if (opcode == RISCV_FSW){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = 0;
//TODO
				if (funct3 == 4)
					binaries = assembleRiInstruction(VEX_FSW, rs2, rs1, imm12_S_signed);
				else if (funct3 == 2)
					binaries = assembleRiInstruction(VEX_FSH, rs2, rs1, imm12_S_signed);
				else
					binaries = assembleRiInstruction(VEX_FSB, rs2, rs1, imm12_S_signed);

				stage = stageMem;

			}
			else if (opcode == RISCV_FMADD || opcode == RISCV_FNMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMSUB){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = 64+rd;
				lastLatency = MULT_LATENCY;

				if (opcode == RISCV_FMADD)
					binaries = assembleRRInstruction(VEX_FMADD, rd, rs1, rs2, rs3);
				else if (opcode == RISCV_FNMADD)
					binaries = assembleRRInstruction(VEX_FNMADD, rd, rs1, rs2, rs3);
				else if (opcode == RISCV_FMSUB)
					binaries = assembleRRInstruction(VEX_FMSUB, rd, rs1, rs2, rs3);
				else
					binaries = assembleRRInstruction(VEX_FNMSUB, rd, rs1, rs2, rs3);

				stage = stageMult;
			}
			else if (opcode == RISCV_FP){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd+64;
				lastLatency = MULT_LATENCY;

				ac_int<5, false> funct = 0;
				switch (funct7)
				{
					case  RISCV_FP_ADD:
						funct = VEX_FP_FADD;
						break;
					case  RISCV_FP_SUB:
						funct = VEX_FP_FSUB;
						break;
					case  RISCV_FP_MUL:
						funct = VEX_FP_FMUL;
						break;
					case  RISCV_FP_DIV:
						funct = VEX_FP_FDIV;
						break;
					case  RISCV_FP_SQRT:
						funct = VEX_FP_FSQRT;
						break;
					case  RISCV_FP_FSGN:
						if (funct3 == RISCV_FP_FSGN_J)
							funct = VEX_FP_FSGNJ;
						else if (funct3 == RISCV_FP_FSGN_JN)
							funct = VEX_FP_FSGNJN;
						else //JX
							funct = VEX_FP_FSGNJNX;
						break;
					case  RISCV_FP_MINMAX:
						if (funct3 == RISCV_FP_MINMAX_MIN)
							funct = VEX_FP_FMIN;
						else
							funct = VEX_FP_FMAX;
						break;
					case  RISCV_FP_FCVTW:
						if (rs2 == RISCV_FP_FCVTW_W)
							funct = VEX_FP_FCVTWS;
						else
							funct = VEX_FP_FCVTWUS;
						break;
					case  RISCV_FP_FMVXFCLASS:
						if (funct3 == RISCV_FP_FMVXFCLASS_FMVX){
							lastWrittenRegister = rd;
							funct = VEX_FP_FMVXW;
						}
						else
							funct = VEX_FP_FCLASS;
						break;
					case  RISCV_FP_FCMP:
						lastWrittenRegister = rd;
						if (funct3 == RISCV_FP_FCMP_FEQ)
							funct = VEX_FP_FEQ;
						else if (funct3 == RISCV_FP_FCMP_FLT)
							funct = VEX_FP_FLT;
						else
							funct = VEX_FP_FLE;
						break;
					case  RISCV_FP_FCVTS:
						lastWrittenRegister = rd;
						if (rs2 == RISCV_FP_FCVTS_W)
							funct = VEX_FP_FCVTSW;
						else
							funct = VEX_FP_FCVTSWU;
						break;
					case  RISCV_FP_FMVW:
						funct = VEX_FP_FMVWX;
						break;
				}


				binaries = assembleFPInstruction(VEX_FP, funct, rd, rs2, rs1);
				stage=stageMult;

			}
			else if (opcode == 0){

							previousLatency = lastLatency;
							previousWrittenRegister = lastWrittenRegister;
							lastWrittenRegister = 0;
							lastLatency = 0;
							binaries = 0;
			}
			else{
				#ifndef __CATAPULT
				printf("In first pass translator, instr %x is not handled yet...\n", oneInstruction);
				exit(-1);
				#endif
			}
		}




		if (indexInSourceBinaries != 0){
			destinationBinaries[previousIndex] = previousBinaries.slc<128>(0);
			destinationBinaries[previousIndex+1] = previousBinaries.slc<128>(128);

		}

		ac_int<32, false> const0_long = 0;
		previousBinaries = 0;
		previousBinaries.set_slc(96, (stage == 0) ? binaries : const0_long);
		previousBinaries.set_slc(64, (stage == 1) ? binaries : const0_long);
		previousBinaries.set_slc(32, (stage == 2) ? binaries : const0_long);
		previousBinaries.set_slc(0, (stage == 3) ? binaries : const0_long);
		previousBinaries.set_slc(32+128, (stage == 6) ? binaries : const0_long);

		previousIndex = indexInDestinationBinaries;
		previousStage = stage;

		if (isInsertion)
			insertions[1+localNumberInsertions++] = indexInDestinationBinaries;

		if (droppedInstruction)
			insertions[1+localNumberInsertions++] = -indexInDestinationBinaries;


		if (setBoundaries1){
			ac_int<1, false> const1 = 1;
			ac_int<32, false> boundaryAddress = (boundary1 + ((codeSectionStart - addressStart)>>2));
			ac_int<16, false> offset = boundaryAddress.slc<16>(0);
			//ac_int<3, false> bitOffset = boundaryAddress.slc<3>(0);
			blocksBoundaries[offset] = 1; //.set_slc(bitOffset, const1);

			if (boundary1 == indexInSourceBinaries)
				currentBoundaryJustSet=1;


		}

		if (setUnresolvedJump){
			//We add the instruction to the list of unresolved destinations
			unresolvedJumps_src[numberUnresolvedJumps] = unresolved_jump_src;
			unresolvedJumps_type[numberUnresolvedJumps] = unresolved_jump_type;
			unresolvedJumps[numberUnresolvedJumps++] = boundary1;
		}


		//Note: setBoundaries2 => nextInstructionNop
		//Thus we can solve the second boundary at the next cycle

		if (setBoundaries2){
			nextBoundaries = boundary2;
			setNextBoundaries = 1;

			currentBoundaryJustSet = 1;
		}


		if (!wasExternalInstr)
			indexInSourceBinaries++;

		if (!droppedInstruction)
			indexInDestinationBinaries+=incrementInDest;

		if (lastLatency != 0)
			lastLatency--;

		if (previousLatency != 0)
			previousLatency--;

	}


	char remainingLatency = (lastLatency > previousLatency) ? lastLatency : previousLatency;
	indexInDestinationBinaries += remainingLatency * incrementInDest;
	for (int oneRemainingLatency = 0; oneRemainingLatency<remainingLatency; oneRemainingLatency++){
		insertions[1+localNumberInsertions++] = indexInDestinationBinaries;
		indexInDestinationBinaries += incrementInDest;
	}




	destinationBinaries[previousIndex] = previousBinaries.slc<128>(0);
	destinationBinaries[previousIndex+1] = previousBinaries.slc<128>(128);

	insertions[0] = localNumberInsertions;

	return (indexInDestinationBinaries-placeCode) + (numberUnresolvedJumps<<18);
}

#endif
