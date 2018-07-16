#include <isa/riscvISA.h>
#include <isa/riscvToVexISA.h>
#include <isa/vexISA.h>
#include <transformation/firstPassTranslation.h>
#include <transformation/reconfigureVLIW.h>
#include <lib/endianness.h>

int firstPassTranslator_riscv_sw(unsigned int code[1024],
		unsigned int size,
		unsigned char conf,
		unsigned int addressStart,
		unsigned int codeSectionStart,
		unsigned int destinationBinaries[4*1024],
		unsigned int placeCode,
		unsigned int insertions[256],
		unsigned char blocksBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned int unresolvedJumps_type[512],
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
	char issueWidth = getIssueWidth(conf);

	char stageMem = ((conf & 0xf) == 1) ? 2 : ((conf & 0xf) == 3) ? 6 : 1;
	char stageMult = ((conf & 0xf) == 1) ? 3 : ((conf & 0xf) == 3) ? 2 : 1;


	int indexInSourceBinaries = 0;
	int indexInDestinationBinaries = placeCode;


	unsigned int nextInstruction, nextInstruction_stage;
	char nextInstruction_rs1, nextInstruction_rs2, nextInstruction_rd;
	char secondNextInstruction_rs1, secondNextInstuction_rs2, secondNextInstruction_rd;

	unsigned int secondNextInstruction, secondNextInstruction_stage;

	unsigned char enableNextInstruction = 0;
	unsigned char enableSecondNextInstruction = 0;

	unsigned char nextInstructionNop = 0;


	char reg1_mul = 0, reg2_mul = 0;
	short imm_mul = 0;
	bool is_imm_mul = 0;


	blocksBoundaries[(codeSectionStart-addressStart)>>2] = 1;

	unsigned int previousBinaries[8];
	unsigned int previousIndex = 0;
	char previousStage = 0;
	unsigned int localNumberInsertions = 0;

	bool currentBoundaryJustSet = 0;

	bool setNextBoundaries = 0;
	unsigned int nextBoundaries;

	char previousWrittenRegister = 0;
	char lastWrittenRegister = 0;
	char lastLatency = 0;
	char previousLatency = 0;

	char incrementInDest = (issueWidth>4) ? 2:1;


	while (indexInSourceBinaries < size || nextInstructionNop || enableNextInstruction){

		bool setBoundaries1 = false, setBoundaries2 = false, setUnresolvedJump = false;
		int boundary1, boundary2, unresolved_jump_src, unresolved_jump_type;

		bool isInsertion = false;

		char stage = 0;
		unsigned int binaries = 0;

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
				enableNextInstruction = true;
				wasExternalInstr = 1;
				isInsertion = true;
			}
			else{
				binaries = nextInstruction;
				stage = nextInstruction_stage;
				wasExternalInstr = 1;
				enableNextInstruction = false;
				isInsertion = true;

				lastWrittenRegister = nextInstruction_rd;
				lastLatency = SIMPLE_LATENCY;

				nextInstruction_rd = 0;
				nextInstruction_rs1 = 0;
				nextInstruction_rs2 = 0;

				if (enableSecondNextInstruction){
					enableSecondNextInstruction = false;
					enableNextInstruction = true;
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
			isInsertion = true; //A nop instruction is an insertion...
			setBoundaries1 = setNextBoundaries;
			boundary1 = nextBoundaries;
			setNextBoundaries = 0;
		}
		else{

			unsigned int oneInstruction = readInt(code, 4*indexInSourceBinaries);
			/**************************************************************
			*  Instruction decoding
			*
			*  First step of the process is the instruction decoding where we build
			*  each part of the instruction. These part will be used later on in
			*  order to build the new instruction.
			***************************************************************/

			char opcode = oneInstruction & 0x7f;
			char rs1 = ((oneInstruction >> 15) & 0x1f);
			char rs2 = ((oneInstruction >> 20) & 0x1f);
			char rs3 = ((oneInstruction >> 27) & 0x1f);
			char rd = ((oneInstruction >> 7) & 0x1f);
			char funct7 = ((oneInstruction >> 25) & 0x7f);
			char funct7_smaller = funct7 & 0x3e;

			char funct3 = ((oneInstruction >> 12) & 0x7);
			unsigned short imm12_I = ((oneInstruction >> 20) & 0xfff);
			unsigned short imm12_S = ((oneInstruction >> 20) & 0xfe0) + ((oneInstruction >> 7) & 0x1f);


			short imm12_I_signed = (imm12_I >= 2048) ? imm12_I - 4096 : imm12_I;
			short imm12_S_signed = (imm12_S >= 2048) ? imm12_S - 4096 : imm12_S;


			short imm13 = ((oneInstruction >> 19) & 0x1000) + ((oneInstruction >> 20) & 0x7e0) + ((oneInstruction >> 7) & 0x1e) + ((oneInstruction << 4) & 0x800);
			short imm13_signed = (imm13 >= 4096) ? imm13 - 8192 : imm13;

			unsigned int imm31_12 = oneInstruction & 0xfffff000;
			int imm31_12_signed = imm31_12;

			unsigned int imm21_1 = (oneInstruction & 0xff000) + ((oneInstruction >> 9) & 0x800) + ((oneInstruction >> 20) & 0x7fe) + ((oneInstruction >> 11) & 0x100000);
			int imm21_1_signed = (imm21_1 >= 1048576) ? imm21_1 - 2097152 : imm21_1;

			char shamt = ((oneInstruction >> 20) & 0x3f);
			unsigned int correctedTgtadr = imm21_1 - (addressStart>>2);

			if (rs1 == 1)
				rs1 = 63;

			if (rs2==1)
				rs2=63;

			if (rd==1)
				rd=63;

			if (opcode == RISCV_FMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMADD || opcode == RISCV_FNMSUB ||
					(opcode == RISCV_FP && (funct7 != RISCV_FP_FCVTS && funct7 != RISCV_FP_FMVW))){
				rs1 += 64;
			}

			if (opcode == RISCV_FSW || opcode == RISCV_FMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMADD || opcode == RISCV_FNMSUB ||
					(opcode == RISCV_FP && funct7 != RISCV_FP_SQRT && funct7 != RISCV_FP_FCVTW && funct7 != RISCV_FP_FMVXFCLASS
							&& funct7 != RISCV_FP_FCVTS && funct7 != RISCV_FP_FMVW)){
				rs2 += 64;
			}

			bool isRdFloat = (funct7 != RISCV_FP_FCVTW) && (funct7 != RISCV_FP_FCMP) && (funct7 != RISCV_FP_FMVXFCLASS);

			/***************************************************************/
			/*  We assemble the instruction  							   */
			/***************************************************************/


			//We compute a bit saying if previous instruction is at BB boundary
			bool previousIsBoundary = 0;
			unsigned int currentAddress = (indexInSourceBinaries + ((codeSectionStart - addressStart)>>2));
			int offset = currentAddress & 0xffff;
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
				isInsertion = true;
			}
			else if (opcode == RISCV_OP){
				//Instrucion io OP type: it needs two registers operand (except for rol/sll/sr)

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;

				if (funct7 == RISCV_OP_M){
					//We are in the part dedicated to RV32M extension
					binaries =  assembleRInstruction_sw(functBindingMULT_sw[funct3], rd, rs1, rs2);
					stage = stageMult;

					//nextInstructionNop = 1;
					//TODO: should certainly insert a nop
					lastLatency = MULT_LATENCY;
				}
				else if (funct3 == RISCV_OP_SLL || funct3 == RISCV_OP_SR){
					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					char vexOpcode = (funct3==RISCV_OP_SLL) ? VEX_SLL :
							(funct7==RISCV_OP_SR_SRA) ? VEX_SRA : VEX_SRL;

					binaries = assembleRInstruction_sw(vexOpcode, rd, rs1, rs2);
					lastLatency = SIMPLE_LATENCY;
				}
				else {
					char vexOpcode = (funct7==RISCV_OP_ADD_SUB) ? VEX_SUB : (char) functBindingOP_sw[funct3];
					binaries =  assembleRInstruction_sw(vexOpcode, rd, rs1, rs2);
					lastLatency = SIMPLE_LATENCY;
				}


			}
			else if (opcode == RISCV_AUIPC){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				unsigned int value = codeSectionStart + (indexInSourceBinaries<<2) + imm31_12_signed;

				if (previousIsBoundary){

					binaries = assembleIInstruction_sw(VEX_MOVI, (value>>12) & 0x7ffff, rd);
					nextInstruction = assembleRiInstruction_sw(VEX_SLLi, rd, rd, 12);
					secondNextInstruction = assembleRiInstruction_sw(VEX_ADDi, rd, rd, value & 0xfff);

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					secondNextInstruction_rd = rd;
					secondNextInstruction_rs1 = rd;
					secondNextInstuction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = true;
					enableSecondNextInstruction = 1;
					secondNextInstruction_stage = 0;

				}
				else{
					binaries = assembleIInstruction_sw(VEX_MOVI, (value>>12) & 0x7ffff, rd);
					nextInstruction = assembleRiInstruction_sw(VEX_SLLi, rd, rd, 12);
					secondNextInstruction = assembleRiInstruction_sw(VEX_ADDi, rd, rd, value & 0xfff);

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					secondNextInstruction_rd = rd;
					secondNextInstruction_rs1 = rd;
					secondNextInstuction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = true;
					enableSecondNextInstruction = true;
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


				char bit31 = imm31_12>>31;
				char bit30 = ((imm31_12>>30)&0x1);

				if (bit31 != bit30){
					binaries = assembleIInstruction_sw(VEX_MOVUI, (imm31_12>>14) & 0x7ffff, rd);
					nextInstruction = assembleRiInstruction_sw(VEX_SH2ADDi, rd, rd, (imm31_12>>12)&0x3);

					//Mark the insertion
					nextInstruction_rd = rd;
					nextInstruction_rs1 = rd;
					nextInstruction_rs2 = 0;

					nextInstruction_stage = 0;
					enableNextInstruction = true;
				}
				else {
					binaries = assembleIInstruction_sw(VEX_MOVUI, (imm31_12>>12) & 0x7ffff, rd);
				}



//				bool keepHigher = 0;
//				bool keepBoth = 0;
//
//				if ((imm31_12>>31) != ((imm31_12>>30)&0x1)){
//					keepHigher = 1;
//					if ((imm31_12>>12) & 0x1)
//						keepBoth = 1;
//				}
//
//
//				int immediateValue = keepHigher ? (imm31_12 >> 13) & 0x7ffff : (imm31_12 >> 12) & 0x7ffff;
//				char shiftValue = keepHigher ? 13 : 12;
//
//				unsigned int instr1 = assembleIInstruction_sw(VEX_MOVI, immediateValue, rd);
//				unsigned int instr2 = assembleRiInstruction_sw(VEX_SLLi, rd, rd, shiftValue);
//				unsigned int instr3 = assembleRiInstruction_sw(VEX_ADDi, rd, rd, 0x1000);
//
//
//				if (previousIsBoundary){
//					binaries = instr1;
//					nextInstruction = instr2;
//
//					//Mark the insertion
//					nextInstruction_rd = rd;
//					nextInstruction_rs1 = rd;
//					nextInstruction_rs2 = 0;
//
//					nextInstruction_stage = 0;
//					enableNextInstruction = true;
//
//					if (keepBoth){
//						secondNextInstruction_rd = rd;
//						secondNextInstruction_rs1 = rd;
//						secondNextInstuction_rs2 = 0;
//
//						secondNextInstruction = instr3;
//						enableSecondNextInstruction = 1;
//						secondNextInstruction_stage = 0;
//					}
//
//				}
//				else{
//					binaries = instr1;
//					nextInstruction = instr2;
//
//					//Mark the insertion
//					nextInstruction_rd = rd;
//					nextInstruction_rs1 = rd;
//					nextInstruction_rs2 = 0;
//
//					nextInstruction_stage = 0;
//					enableNextInstruction = true;
//
//					if (keepBoth){
//						secondNextInstruction_rd = rd;
//						secondNextInstruction_rs1 = rd;
//						secondNextInstuction_rs2 = 0;
//
//						secondNextInstruction = instr3;
//						enableSecondNextInstruction = true;
//						secondNextInstruction_stage = 0;
//					}
//				}

			}
			else if (opcode == RISCV_LD){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = MEMORY_LATENCY;

				//Memory access operations.
				binaries = assembleRiInstruction_sw(functBindingLD_sw[funct3], rd, rs1, imm12_I_signed);
				stage = stageMem;


			}
			else if (opcode == RISCV_ST){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = 0;

				binaries = assembleRiInstruction_sw(functBindingST_sw[funct3], rs2, rs1, imm12_S_signed);
				stage = stageMem;
			}
			else if (opcode == RISCV_JAL){

				//If rsd is equal to zero, then we are in a simple J instruction
				bool isSimpleJ = (rd==0);
				binaries= assembleIInstruction_sw(isSimpleJ ? VEX_GOTO : VEX_CALL, 0, rd);


				//We fill information on block boundaries
				setBoundaries1 = true;
				boundary1 = indexInSourceBinaries + (imm21_1_signed>>2);
				setBoundaries2 = true;
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

				setBoundaries1 = true;
				boundary1 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed

				if (rs1 == 63 && rd == 0){
					//We are in a simple return
					binaries = assembleIInstruction_sw(VEX_GOTOR, imm12_I_signed, 63);

				}
				else{
					//FIXME should be able to add two instr at the same cycle... This would remove an insertion
					binaries = assembleRiInstruction_sw(VEX_ADDi, 33, rs1, imm12_I_signed);

					nextInstruction = assembleIInstruction_sw((rd == 63) ? VEX_CALL : VEX_GOTO, 4*incrementInDest, rd);
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


//				if (rs2 == 0 && (funct3 == RISCV_BR_BEQ || funct3 == RISCV_BR_BNE)){
					//This is a comparison to zero

					setBoundaries1 = true;
					boundary1 = ((imm13_signed>>2)+indexInSourceBinaries);
					setBoundaries2 = true;
					boundary2 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed

					binaries = assembleRiInstruction_sw(functBindingBR_sw[funct3], rs2, rs1,0);

					unresolved_jump_src = indexInDestinationBinaries;
					unresolved_jump_type = binaries;
					setUnresolvedJump = 1;

//				}
//				else{
//
//					previousLatency = lastLatency;
//					previousWrittenRegister = lastWrittenRegister;
//					lastWrittenRegister = 32;
//					lastLatency = SIMPLE_LATENCY;
//
//					setBoundaries1 = true;
//					boundary1 = ((imm13_signed>>2)+indexInSourceBinaries);
//					setBoundaries2 = true;
//					boundary2 = indexInSourceBinaries + 1;//Only plus one because in riscv next instr is not executed
//
//
//					binaries = assembleRInstruction_sw(functBindingBR_sw[funct3], 32, rs1, rs2); //TODO check order
//
//					nextInstruction = assembleRiInstruction_sw(VEX_BRF, 32, 0,0);
//
//
//					nextInstruction_stage = 0;
//					enableNextInstruction = 1;
//					nextInstruction_rd = 0;
//					nextInstruction_rs1 = 32;
//					nextInstruction_rs2 = 0;
//
//					unresolved_jump_src = indexInDestinationBinaries+2*incrementInDest;
//					unresolved_jump_type = nextInstruction;
//					setUnresolvedJump = 1;
//
//
//				}

				//In order to deal with the fact that RISCV do not execute the isntruction following a branch,
				//we have to insert nop
				nextInstructionNop = 1;


			}
			else if (opcode == RISCV_OPI){ //For all other instructions

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				int extendedImm = imm12_I_signed;
				if (funct3 == RISCV_OPI_SLTIU)
					extendedImm = imm12_I;


				if (funct3 == RISCV_OPI_SLLI || funct3 == RISCV_OPI_SRI){

					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					char vexOpcode = (funct3==RISCV_OPI_SLLI) ? VEX_SLLi :
							(funct7_smaller==RISCV_OPI_SRI_SRAI) ? VEX_SRAi : VEX_SRLi;

					binaries = assembleRiInstruction_sw(vexOpcode, rd, rs1, shamt);


				}
				else {
					binaries = assembleRiInstruction_sw(functBindingOPI_sw[funct3], rd, rs1, extendedImm);
				}
			}
			else if (opcode == RISCV_OPW){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				if (funct7 == RISCV_OP_M){
					//We are in the part dedicated to RV64M extension
					binaries =  assembleRInstruction_sw(functBindingMULTW_sw[funct3], rd, rs1, rs2);
					stage = stageMult;

					nextInstructionNop = 1;

				}
				else if (funct3 == RISCV_OPW_SLLW || funct3 == RISCV_OPW_SRW){
					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					char vexOpcode = (funct3==RISCV_OPW_SLLW) ? VEX_SLLW :
							(funct7==RISCV_OPW_SRW_SRAW) ? VEX_SRAW : VEX_SRLW;

					binaries = assembleRInstruction_sw(vexOpcode, rd, rs1, rs2);

				}
				else {


					char vexOpcode = (funct7==RISCV_OPW_ADDSUBW_SUBW) ? VEX_SUBW : VEX_ADDW;
					binaries =  assembleRInstruction_sw(vexOpcode, rd, rs1, rs2);
				}
			}
			else if (opcode == RISCV_OPIW){ //For 32-bits instructions (labelled with a W)

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = SIMPLE_LATENCY;

				int extendedImm = imm12_I_signed;
				if (funct3 == RISCV_OPI_SLTIU)
					extendedImm = imm12_I;


				if (funct3 == RISCV_OPIW_SLLIW || funct3 == RISCV_OPIW_SRW){

					//Shift instruction. Careful if we are on SR, we need to use funct7 to determine which instruction we
					//are working with...

					char vexOpcode = (funct3==RISCV_OPIW_SLLIW) ? VEX_SLLWi :
							(funct7==RISCV_OPIW_SRW_SRAIW) ? VEX_SRAWi : VEX_SRLWi;

					binaries = assembleRiInstruction_sw(vexOpcode, rd, rs1, (shamt & 0x1f));


				}
				else {
					binaries = assembleRiInstruction_sw(VEX_ADDWi, rd, rs1, extendedImm);
				}
			}
			else if (opcode == RISCV_SYSTEM){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = 10;
				lastLatency = SIMPLE_LATENCY;

				if (funct3 == RISCV_SYSTEM_ENV){
					setBoundaries1 = 1;
					boundary1 = indexInSourceBinaries;//Only plus one because in riscv next instr is not executed

					binaries = assembleIInstruction_sw(VEX_ECALL, 0,0);
				}
				else {

					#ifndef __CATAPULT
					binaries = assembleIInstruction_sw(VEX_NOP, 0,0);

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
					binaries = assembleRiInstruction_sw(VEX_FLW, rd, rs1, imm12_I_signed);
				else if (funct3 == 4)
					binaries = assembleRiInstruction_sw(VEX_FLH, rd, rs1, imm12_I_signed);
				else
					binaries = assembleRiInstruction_sw(VEX_FLB, rd, rs1, imm12_I_signed);

				stage = stageMem;
			}
			else if (opcode == RISCV_FSW){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				lastLatency = 0;
//TODO
				if (funct3 == 4)
					binaries = assembleRiInstruction_sw(VEX_FSW, rs2, rs1, imm12_S_signed);
				else if (funct3 == 2)
					binaries = assembleRiInstruction_sw(VEX_FSH, rs2, rs1, imm12_S_signed);
				else
					binaries = assembleRiInstruction_sw(VEX_FSB, rs2, rs1, imm12_S_signed);

				stage = stageMem;

			}
			else if (opcode == RISCV_FMADD || opcode == RISCV_FNMADD || opcode == RISCV_FMSUB || opcode == RISCV_FNMSUB){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = 64+rd;
				lastLatency = MULT_LATENCY;

				if (opcode == RISCV_FMADD)
					binaries = assembleRRInstruction_sw(VEX_FMADD, rd, rs1, rs2, rs3);
				else if (opcode == RISCV_FNMADD)
					binaries = assembleRRInstruction_sw(VEX_FNMADD, rd, rs1, rs2, rs3);
				else if (opcode == RISCV_FMSUB)
					binaries = assembleRRInstruction_sw(VEX_FMSUB, rd, rs1, rs2, rs3);
				else
					binaries = assembleRRInstruction_sw(VEX_FNMSUB, rd, rs1, rs2, rs3);

				stage = stageMult;
			}
			else if (opcode == RISCV_FP){

				previousLatency = lastLatency;
				previousWrittenRegister = lastWrittenRegister;
				lastWrittenRegister = rd;
				if (isRdFloat)
					lastWrittenRegister += 64;
				lastLatency = MULT_LATENCY;

				char funct = 0;
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
						rs2 = rs1;
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

						rs2 = rs1;
						break;
					case  RISCV_FP_FMVXFCLASS:
						if (funct3 == RISCV_FP_FMVXFCLASS_FMVX){
							funct = VEX_FP_FMVXW;
						}
						else
							funct = VEX_FP_FCLASS;

						rs2 = rs1;
						break;
					case  RISCV_FP_FCMP:
						if (funct3 == RISCV_FP_FCMP_FEQ)
							funct = VEX_FP_FEQ;
						else if (funct3 == RISCV_FP_FCMP_FLT)
							funct = VEX_FP_FLT;
						else
							funct = VEX_FP_FLE;
						break;
					case  RISCV_FP_FCVTS:
						if (rs2 == RISCV_FP_FCVTS_W)
							funct = VEX_FP_FCVTSW;
						else
							funct = VEX_FP_FCVTSWU;
						rs2 = rs1;
						break;
					case  RISCV_FP_FMVW:
						funct = VEX_FP_FMVWX;
						rs2 = rs1;
						break;
				}


				binaries = assembleFPInstruction_sw(VEX_FP, funct, rd, rs2, rs1);
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
				printf("In first pass translator_riscv_sw, instr %x is not handled yet...\n", oneInstruction);
				exit(-1);
				#endif
			}
		}




		if (indexInSourceBinaries != 0){
			destinationBinaries[(previousIndex * 4) + 0] = previousBinaries[0];
			destinationBinaries[(previousIndex * 4) + 1] = previousBinaries[1];
			destinationBinaries[(previousIndex * 4) + 2] = previousBinaries[2];
			destinationBinaries[(previousIndex * 4) + 3] = previousBinaries[3];

			if (incrementInDest == 2){
				destinationBinaries[(previousIndex * 4) + 4] = previousBinaries[4];
				destinationBinaries[(previousIndex * 4) + 5] = previousBinaries[5];
				destinationBinaries[(previousIndex * 4) + 6] = previousBinaries[6];
				destinationBinaries[(previousIndex * 4) + 7] = previousBinaries[7];
			}
		}

		previousBinaries[0] = (stage == 0) ? binaries : 0;
		previousBinaries[1] = (stage == 1) ? binaries : 0;
		previousBinaries[2] = (stage == 2) ? binaries : 0;
		previousBinaries[3] = (stage == 3) ? binaries : 0;
		previousBinaries[4] = 0;
		previousBinaries[5] = 0;
		previousBinaries[6] = (stage == 6) ? binaries : 0;
		previousBinaries[7] = 0;

		previousIndex = indexInDestinationBinaries;
		previousStage = stage;

		if (isInsertion)
			insertions[1+localNumberInsertions++] = indexInDestinationBinaries;

		if (droppedInstruction)
			insertions[1+localNumberInsertions++] = -indexInDestinationBinaries;


		if (setBoundaries1){
			int boundaryAddress = (boundary1 + ((codeSectionStart - addressStart)>>2));
			int offset = boundaryAddress & 0xffff;
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


	if (indexInSourceBinaries != 0){
		destinationBinaries[(previousIndex * 4) + 0] = previousBinaries[0];
		destinationBinaries[(previousIndex * 4) + 1] = previousBinaries[1];
		destinationBinaries[(previousIndex * 4) + 2] = previousBinaries[2];
		destinationBinaries[(previousIndex * 4) + 3] = previousBinaries[3];

		if (incrementInDest == 2){
			destinationBinaries[(previousIndex * 4) + 4] = previousBinaries[4];
			destinationBinaries[(previousIndex * 4) + 5] = previousBinaries[5];
			destinationBinaries[(previousIndex * 4) + 6] = previousBinaries[6];
			destinationBinaries[(previousIndex * 4) + 7] = previousBinaries[7];
		}
	}

	insertions[0] = localNumberInsertions;
	return (indexInDestinationBinaries-placeCode) + (numberUnresolvedJumps<<18);
}
