/*
 * firstPassTranslatorRISCV.cpp
 *
 *  Created on: 3 déc. 2016
 *      Author: Simon Rokicki
 *
 * This file contains the description of the first pass translator targeting RISCV ISA
 */

#include <cstdio>
#include <cstdlib>

#include <frontend.h>
#include <lib/elfFile.h>
#include <lib/endianness.h>
#include <lib/tools.h>
#include <isa/riscvISA.h>
#include <isa/vexISA.h>
#include <isa/riscvToVexISA.h>
#include <types.h>

#include <dbt/insertions.h>



const unsigned int debugLevel = 0;

using namespace std;

int firstPassTranslatorRISCV_hw(uint32 code[1024],
		uint32 size,
		uint32 addressStart,
		uint128 destinationBinaries[1024],
		uint32 placeCode,
		uint32 insertions[256],
		int16 blocksBoundaries[65536],
		int16 proceduresBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned char unresolvedJumps_type[512],
		int unresolvedJumps[512]);

/**************************************************************
 *  Function firstPassTranslator will translate a piece of MIPS binaries from the memory 'code' and
 *  write the result in VLIW binaries. At the same time, it collects and solve unresolved jumps (indeed jumps
 *  destination may change because of insertions during the translation); it also keep trace of the basic block
 *  and procedures boundaries.
 *
 **************************************************************/
int firstPassTranslator_RISCV(uint32 *riscvBinaries,
		uint32 *size,
		uint32 codeSectionStart,
		uint32 addressStart,
		uint128 *vliwBinaries,
		uint32 *placeCode,
		uint32 *insertions,
		int16 *blocksBoundaries,
		int16 *proceduresBoundaries){



	unsigned int localUnresolvedJumps_src[65536];
	unsigned char localUnresolvedJumps_type[65536];
	int localUnresolvedJumps[65536];

	insertions[0] = 0;

	//We call the accelerator (or the software counterpart if no accelerator)
	int returnedValue = firstPassTranslatorRISCV_hw(riscvBinaries,
			*size,
			addressStart,
			vliwBinaries,
			*placeCode,
			insertions,
			blocksBoundaries,
			proceduresBoundaries,
			localUnresolvedJumps_src,
			localUnresolvedJumps_type,
			localUnresolvedJumps);

	//We translate the result
	unsigned int destinationIndex = returnedValue & 0x3ffff;
	unsigned int numberUnresolvedJumps = returnedValue >> 18;

	//We copy insertions

	addInsertions((addressStart-codeSectionStart)>>2, *placeCode, insertions, insertions[0]);

	/************************************************************
	* Resolution of unresolved jumps
	*************************************************************
	*
	* Idea: The first pass translator will return a list with three values:
	*    -> source: index in the vliwBinaries of the unresolved jump (ie. place where we have to modify the instr.)
	*    -> initialDestination: address where the jump led in MIPS binaries
	*    -> type: is it an absolute or a relative jump
	*
	* We need to find the correct immediate value to insert in the instruction. For this, we need to find the index
	* of the new destination. We will process iteratively by considering the initial address and increasing its
	* value for each insertion done by the translator (and stored in array called 'insertions'.
	*
	* TODO: if we allow the splitting of the binaries to translate into several smaller pieces, we will need to
	* check here if the destination is translated or not.
	*
	* Note: Insertions from memory 'insertions' contains addresses with the offset placeCode already applied
	************************************************************/

	for (int oneUnresolvedJump = 0; oneUnresolvedJump<numberUnresolvedJumps; oneUnresolvedJump++){

		unsigned int source = localUnresolvedJumps_src[oneUnresolvedJump];
		unsigned int initialDestination = localUnresolvedJumps[oneUnresolvedJump];
		unsigned char type = localUnresolvedJumps_type[oneUnresolvedJump];

		unsigned int oldJump = readInt(vliwBinaries, 16*(source));
		unsigned int indexOfDestination = 0;


		unsigned int destinationInVLIWFromNewMethod = solveUnresolvedJump(initialDestination + ((addressStart-codeSectionStart)>>2));
		if (destinationInVLIWFromNewMethod == -1){

			//In this case, the jump cannot be resolved because the destination block is not translated yet.
			//We store information concerning the destination and it will be resolved later

			int numberUnresolvedJumps = unresolvedJumpsArray[0];
			unresolvedJumpsArray[1+numberUnresolvedJumps] = initialDestination + ((addressStart-codeSectionStart)>>2);
			unresolvedJumpsTypeArray[1+numberUnresolvedJumps] = type;
			unresolvedJumpsSourceArray[1+numberUnresolvedJumps] = source;

			unresolvedJumpsArray[0] = numberUnresolvedJumps+1;
		}
		else if (type == UNRESOLVED_JUMP_ABSOLUTE){
			//In here we solve an absolute jump

			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;

			writeInt(vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}
		else{
			//In here we solve a relative jump

			indexOfDestination = destinationInVLIWFromNewMethod;
			initialDestination = destinationInVLIWFromNewMethod;
			initialDestination = initialDestination  - (source) ;


			//We modify the jump instruction to make it jump at the correct place
			writeInt(vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}

		unsigned int instructionBeforePreviousDestination = readInt(vliwBinaries, 16*(indexOfDestination-1)+12);
		if (instructionBeforePreviousDestination != 0)
			writeInt(vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
	}


	numberUnresolvedJumps = 0;
	*placeCode += destinationIndex;

	/************************************************************/
	/************************************************************/

}

#include <lib/ac_int.h>
typedef ac_int<7,false> acu7;

typedef ac_int<5,false> acu5;
typedef ac_int<32,false> acu32;
typedef ac_int<6,false> acu6;
typedef ac_int<16,true> acs16;
typedef ac_int<26,true> acs26;
typedef ac_int<1,false> acu1;

int firstPassTranslatorRISCV_hw(uint32 code[1024],
		uint32 size,
		uint32 addressStart,
		uint128 destinationBinaries[1024],
		uint32 placeCode,
		uint32 insertions[256],
		int16 blocksBoundaries[65536],
		int16 proceduresBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned char unresolvedJumps_type[512],
		int unresolvedJumps[512]){


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

	int indexInSourceBinaries = 0;
	int indexInDestinationBinaries = placeCode;


	ac_int<32, 0> nextInstruction, nextInstruction_stage;
	ac_int<32, 0> secondNextInstruction, secondNextInstruction_stage;

	unsigned char enableNextInstruction;
	unsigned char enableSecondNextInstruction;

	unsigned char nextInstructionNop = 0;


	acu5 reg1_mul = 0, reg2_mul = 0;
	acs16 imm_mul = 0;
	acu1 is_imm_mul = 0;


	blocksBoundaries[0] |= 1;
	proceduresBoundaries[0] = 1;

	ac_int<128,false> previousBinaries = 0;
	uint32 previousIndex = 0;
	char previousStage = 0;

	ac_int<32, false> localNumberInsertions = 0;

	while (indexInSourceBinaries < size){

		ac_int<1, false> setBoundaries1 = 0, setBoundaries2 = 0, setUnresolvedJump = 0;
		ac_int<32, true> boundary1, boundary2, unresolved_jump_src, unresolved_jump_type;


		ac_int<1, false> isInsertion = 0;

		char stage = 0;
		uint32 binaries = 0;

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
			binaries = nextInstruction;
			stage = nextInstruction_stage;
			wasExternalInstr = 1;
			enableNextInstruction = 0;
			isInsertion = 1;

			if (enableSecondNextInstruction){
				enableSecondNextInstruction = 0;
				enableNextInstruction = 1;
				nextInstruction = secondNextInstruction;
				nextInstruction_stage = secondNextInstruction_stage;
			}
		}
		else if (nextInstructionNop){
			binaries = 0;
			stage = 0;
			wasExternalInstr = 1;
			nextInstructionNop = 0;
			isInsertion = 1; //A nop instruction is an insertion...
		}
		else{

			ac_int<32, false> oneInstruction = code[indexInSourceBinaries];

			if (debugLevel >= 1)
				printf("Source instr %x\n", (int) oneInstruction);

			/**************************************************************
			*  Instruction decoding
			*
			*  First step of the process is the instruction decoding where we build
			*  each part of the instruction. These part will be used later on in
			*  order to build the new instruction.
			***************************************************************/
			ac_int<7, false> opcode = oneInstruction.slc<7>(0);
			ac_int<6, false> rs1 = oneInstruction.slc<5>(15);
			ac_int<6, false> rs2 = oneInstruction.slc<5>(20);
			ac_int<6, false> rd = oneInstruction.slc<5>(7);
			ac_int<7, false> funct7 = oneInstruction.slc<7>(25);
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

			acs26 correctedTgtadr = imm21_1 - (addressStart>>2);


			if (rs1 == 1)
				rs1 = 63;

			if (rs2==1)
				rs2=63;

			if (rd==1)
				rd=63;

			/***************************************************************/
			/*  We assemble the instruction  							   */
			/***************************************************************/

			if (opcode == RISCV_OP){
				//Instrucion io OP type: it needs two registers operand (except for rol/sll/sr)

				if (funct7 == RISCV_OP_M){
					//We are in the part dedicated to RV32M extension
					binaries =  assembleRInstruction(functBindingMULT[funct3], rd, rs1, rs2);
					stage = 2;

					nextInstructionNop = 1;
					//TODO: should certainly insert a nop

				}
				else if (funct3 == RISCV_OP_SLL || funct3 == RISCV_OP_SR){
					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					ac_int<7, false> vexOpcode = (funct3==RISCV_OP_SLL) ? VEX_SLL :
							(funct7==RISCV_OP_SR_SRA) ? VEX_SRA : VEX_SRL;

					binaries = assembleRInstruction(vexOpcode, rd, rs1, rs2);

				}
				else {
					ac_int<7, false> subOpcode = VEX_SUB;
					ac_int<7, false> vexOpcode = (funct7==RISCV_OP_ADD_SUB) ? subOpcode : functBindingOP[funct3];
					binaries =  assembleRInstruction(vexOpcode, rd, rs1, rs2);
				}
			}
			else if (opcode == RISCV_AUIPC){

				ac_int<32, false> value = addressStart + (indexInSourceBinaries<<2) + imm31_12_signed;

				if (blocksBoundaries[indexInSourceBinaries]){

					binaries = assembleIInstruction(VEX_MOVI, value.slc<19>(12), rd);
					nextInstruction = assembleRiInstruction(VEX_SLLi, rd, rd, 12);
					secondNextInstruction = assembleRiInstruction(VEX_ADDi, rd, rd, value.slc<12>(0));

					//Mark the insertion
					nextInstruction_stage = 0;
					enableNextInstruction = 1;
					enableSecondNextInstruction = 1;
					secondNextInstruction_stage = 0;

				}
				else{
					previousBinaries.set_slc(0, assembleIInstruction(VEX_MOVI, value.slc<19>(12), 33));
					binaries = assembleRiInstruction(VEX_SLLi, rd, 33, 12);
					nextInstruction = assembleRiInstruction(VEX_ADDi, rd, rd, value.slc<12>(0));

					//Mark the insertion
					nextInstruction_stage = 0;
					enableNextInstruction = 1;

				}


			}
			else if (opcode == RISCV_LUI){
				//Operation Load Upper Immediate is turned into a MOVI and a SLLi instructions.
				//If the instruction is not at the start point of a block, we can add the movi at the previous
				//cycle to prevent an insertion. Otherwise, we have to do the insertion...

				//TODO RISCV immediates are on 20 bits whereas we only handle 19-bit immediates...

				ac_int<1, false> keepHigher = 0;
				ac_int<1, false> keepBoth = 0;

				if (imm31_12[31] != imm31_12[30]){
					keepHigher = 1;
					if (imm31_12[12])
						keepBoth = 1;
				}


				ac_int<19, false> immediateValue = keepHigher ? imm31_12.slc<19>(13): imm31_12.slc<19>(12);
				ac_int<5, false> shiftValue = keepHigher ? 13 : 12;

				ac_int<32, false> instr1 = assembleIInstruction(VEX_MOVI, immediateValue, 33);
				ac_int<32, false> instr2 = assembleRiInstruction(VEX_SLLi, rd, 33, shiftValue);
				ac_int<32, false> instr3 = assembleRiInstruction(VEX_ADDi, rd, rd, 0x1000);


				if (blocksBoundaries[indexInSourceBinaries]){
					binaries = instr1;
					nextInstruction = instr2;

					//Mark the insertion
					nextInstruction_stage = 0;
					enableNextInstruction = 1;

					if (keepBoth){
						secondNextInstruction = instr3;
						enableSecondNextInstruction = 1;
						secondNextInstruction_stage = 0;
					}

				}
				else{
					previousBinaries.set_slc(0, instr1);
					binaries = instr2;

					if (keepBoth){
						nextInstruction = instr3;
						enableNextInstruction = 1;
						nextInstruction_stage = 0;
					}
				}

			}
			else if (opcode == RISCV_LD){
				//Memory access operations.
				binaries = assembleRiInstruction(functBindingLD[funct3], rd, rs1, imm12_I_signed);
				stage = 1;


			}
			else if (opcode == RISCV_ST){
				binaries = assembleRiInstruction(functBindingST[funct3], rs2, rs1, imm12_S_signed);
				stage = 1;
			}
			else if (opcode == RISCV_JAL){

				//If rsd is equal to zero, then we are in a simple J instruction
				ac_int<1, false> isSimpleJ = (rd==0);


				//We fill information on block boundaries
				setBoundaries1 = 1;
				boundary1 = indexInSourceBinaries + (imm21_1_signed>>2);
				setBoundaries2 = 1;
				boundary2 = indexInSourceBinaries + 1; //Only plus one because in riscv next instr is not executed

				unresolved_jump_src = indexInDestinationBinaries;
				unresolved_jump_type = UNRESOLVED_JUMP_ABSOLUTE;
				setUnresolvedJump = 1;

				//We also fill information on procedure boundaries
				if (rd==63){
					proceduresBoundaries[indexInSourceBinaries + (imm21_1_signed>>2)] = 1;
				}


				binaries= assembleIInstruction(isSimpleJ ? VEX_GOTO : VEX_CALL, 0, rd);

				//In order to deal with the fact that RISCV do not execute the isntruction following a branch,
				//we have to insert nop
				nextInstructionNop = 1;

			}
			else if (opcode == RISCV_JALR){

				//JALR instruction is a bit particular: it load a value from a register and add it an immediate value.
				//Once done, it jump and store return address in rd.

				setBoundaries1 = 1;
				boundary1 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed

				if (rs1 == 63){
					//We are in a simple return
					binaries = assembleIInstruction(VEX_GOTOR, imm12_I_signed, 63);

				}
				else{
					//FIXME should be able to add two instr at the same cycle... This would remove an insertion
					binaries = assembleRiInstruction(VEX_ADDi, 33, rs1, imm12_I_signed);

					nextInstruction = assembleIInstruction((rd == 63) ? VEX_CALL : VEX_GOTO, 4, rd);
					enableNextInstruction = 1;
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

					unresolved_jump_src = indexInDestinationBinaries;
					unresolved_jump_type = UNRESOLVED_JUMP_RELATIVE;
					setUnresolvedJump = 1;

					binaries = assembleIInstruction((funct3 == RISCV_BR_BEQ) ? VEX_BRF : VEX_BR, rs2, rs1);
				}
				else{



					setBoundaries1 = 1;
					boundary1 = ((imm13_signed>>2)+indexInSourceBinaries);
					setBoundaries2 = 1;
					boundary2 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed

					unresolved_jump_src = indexInDestinationBinaries+1;
					unresolved_jump_type = UNRESOLVED_JUMP_RELATIVE;
					setUnresolvedJump = 1;

					binaries = assembleRInstruction(functBindingBR[funct3], 32, rs1, rs2); //TODO check order

					nextInstruction = assembleIInstruction(VEX_BR, 0, 32);


					nextInstruction_stage = 0;
					enableNextInstruction = 1;


				}

				//In order to deal with the fact that RISCV do not execute the isntruction following a branch,
				//we have to insert nop
				nextInstructionNop = 1;


			}
			else if (opcode == RISCV_OPI){ //For all other instructions

				ac_int<13, true> extendedImm = imm12_I_signed;
				if (funct3 == RISCV_OPI_SLTIU)
					extendedImm = imm12_I;


				if (funct3 == RISCV_OPI_SLLI || funct3 == RISCV_OPI_SRI){

					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					ac_int<7, false> vexOpcode = (funct3==RISCV_OPI_SLLI) ? VEX_SLLi :
							(funct7==RISCV_OPI_SRI_SRAI) ? VEX_SRAi : VEX_SRLi;

					binaries = assembleRiInstruction(vexOpcode, rd, rs1, oneInstruction.slc<5>(20));
					//Note: we do not use rs2 var because this oen may have been modified to 63 if its value was 1
					//Of course in the context of imm value this has no sense

				}
				else {
					binaries = assembleRiInstruction(functBindingOPI[funct3], rd, rs1, extendedImm);
				}
			}
			else if (opcode == RISCV_SYSTEM){
				if (funct3 == RISCV_SYSTEM_ENV){
					binaries = assembleIInstruction(VEX_STOP, 0,0);
				}
				else {
					printf("CSR instr not handled yet...\n", oneInstruction);
					exit(-1);
				}
			}
			else{
				printf("Instr %x is not handled yet...\n", oneInstruction);
				exit(-1);
			}
		}




		if (indexInSourceBinaries != 0){
			destinationBinaries[previousIndex] = previousBinaries;
		}

		ac_int<128, false> result = 0;
		ac_int<32, false> const0_long = 0;
		previousBinaries = 0;
		previousBinaries.set_slc(96, (stage == 0) ? binaries : const0_long);
		previousBinaries.set_slc(64, (stage == 1) ? binaries : const0_long);
		previousBinaries.set_slc(32, (stage == 2) ? binaries : const0_long);

		previousIndex = indexInDestinationBinaries;
		previousStage = stage;


		if (isInsertion)
			insertions[1+localNumberInsertions++] = indexInDestinationBinaries;

		if (droppedInstruction)
			insertions[1+localNumberInsertions++] = -indexInDestinationBinaries;


		if (setBoundaries1 && boundary1 >= 0){
			blocksBoundaries[boundary1] = 1;
		}

		if (setUnresolvedJump){
			//We add the instruction to the list of unresolved destinations
			unresolvedJumps_src[numberUnresolvedJumps] = unresolved_jump_src;
			unresolvedJumps_type[numberUnresolvedJumps] = unresolved_jump_type;
			unresolvedJumps[numberUnresolvedJumps++] = boundary1;
		}

		if (setBoundaries2){
			blocksBoundaries[boundary2] = 1;

		}

		if (!wasExternalInstr)
			indexInSourceBinaries++;

		if (!droppedInstruction)
			indexInDestinationBinaries++;

		if (debugLevel >= 1)
			printf("at %d %x\n", (int) indexInDestinationBinaries, (int)binaries);

	}


	destinationBinaries[previousIndex] = previousBinaries;

	insertions[0] = localNumberInsertions;
	return (indexInDestinationBinaries-placeCode) + (numberUnresolvedJumps<<18);
}
