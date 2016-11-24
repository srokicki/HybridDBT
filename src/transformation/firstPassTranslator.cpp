/*
 * File firstPassTranslator.cpp
 * Describe transformation that translate the MIPS binaries into the vliw binaries.
 * This translation is made to be naive and not to exploit instruction level parallelism.
 * However, using the hardware accelerator, this transformation will be very cheap and with low
 * overhead on the startup time (ie. cold code execution).
 *
 * author: Simon Rokicki
 */


#include <cstdio>
#include <cstdlib>

#include <frontend.h>
#include <lib/elfFile.h>
#include <lib/endianness.h>
#include <lib/tools.h>
#include <isa/mipsISA.h>
#include <isa/vexISA.h>
#include <isa/mipsToVexISA.h>
#include <types.h>

//Values for unresolved jumps
#define UNRESOLVED_JUMP_RELATIVE 1
#define UNRESOLVED_JUMP_ABSOLUTE 0

const unsigned int debugLevel = 0;

using namespace std;

#ifndef __USE_HW

//If we are not running on a platform with the accelerator, we declare the function that perform the translation
int generateInterpretationBinaries_loop(uint32 code[1024],
		uint32 size,
		uint32 addressStart,
		uint128 destinationBinaries[1024],
		uint32 placeCode,
		uint32 insertions[256],
		int16 blocksBoundaries[65536],
		int16 proceduresBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned char unresolvedJumps_type[512],
		unsigned int unresolvedJumps[512]);
#endif



#ifdef __USE_AC

/**************************************************************
 *  Function firstPassTranslator will translate a piece of MIPS binaries from the memory 'code' and
 *  write the result in VLIW binaries. At the same time, it collects and solve unresolved jumps (indeed jumps
 *  destination may change because of insertions during the translation); it also keep trace of the basic block
 *  and procedures boundaries.
 *
 **************************************************************/
int firstPassTranslator(uint32 *mipsBinaries,
		uint32 *size,
		uint32 addressStart,
		uint128 *vliwBinaries,
		uint32 *placeCode,
		uint32 *insertions,
		int16 *blocksBoundaries,
		int16 *proceduresBoundaries){



	unsigned int localUnresolvedJumps_src[65536];
	unsigned char localUnresolvedJumps_type[65536];
	unsigned int localUnresolvedJumps[65536];

	insertions[0] = 0;

	//We call the accelerator (or the software counterpart if no accelerator)
	int returnedValue = generateInterpretationBinaries_loop(mipsBinaries,
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

		if (type == UNRESOLVED_JUMP_ABSOLUTE){
			//In here we solve an absolute jump

			initialDestination += *placeCode;
			for (int oneInsertion = 1; oneInsertion <= insertions[0]; oneInsertion++){
				if (insertions[oneInsertion] < 0 && -insertions[oneInsertion] <= initialDestination){
					initialDestination--;
				}
				else if (insertions[oneInsertion] <= initialDestination){
					initialDestination++;
				}
			}

			//We modify the jump instruction to make it jump at the correct place
			indexOfDestination = initialDestination + 1;
			initialDestination = initialDestination + 1;

			writeInt(vliwBinaries, 16*(source), oldJump + ((initialDestination & 0x7ffff)<<7));

		}
		else{
			//In here we solve a relative jump

			initialDestination += *placeCode;
			for (int oneInsertion = 1; oneInsertion <= insertions[0]; oneInsertion++){

				int savedInitialDestination = initialDestination;
				if (insertions[oneInsertion] < 0 && -insertions[oneInsertion] <= initialDestination){
					initialDestination--;
				}
				else if (insertions[oneInsertion] <= initialDestination ){
					initialDestination++;
				}
			}
			indexOfDestination = initialDestination + 1;

			initialDestination = initialDestination - (source) + 1;


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
#endif

#ifdef __USE_AC

#include <lib/ac_int.h>
typedef ac_int<7,false> acu7;

typedef ac_int<5,false> acu5;
typedef ac_int<32,false> acu32;
typedef ac_int<6,false> acu6;
typedef ac_int<16,true> acs16;
typedef ac_int<26,true> acs26;
typedef ac_int<1,false> acu1;

int generateInterpretationBinaries_loop(uint32 code[1024],
		uint32 size,
		uint32 addressStart,
		uint128 destinationBinaries[1024],
		uint32 placeCode,
		uint32 insertions[256],
		int16 blocksBoundaries[65536],
		int16 proceduresBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned char unresolvedJumps_type[512],
		unsigned int unresolvedJumps[512]){


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


		if (enableNextInstruction){
			binaries = nextInstruction;
			stage = nextInstruction_stage;
			wasExternalInstr = 1;
			enableNextInstruction = 0;

			if (enableSecondNextInstruction){
				enableSecondNextInstruction = 0;
				enableNextInstruction = 1;
				nextInstruction = secondNextInstruction;
				nextInstruction_stage = secondNextInstruction_stage;
				isInsertion = 1;
			}
		}
		else if (nextInstructionNop){
			binaries = 0;
			stage = 0;
			wasExternalInstr = 1;
			nextInstructionNop = 0;
		}
		else{

			ac_int<32, false> oneInstruction = code[indexInSourceBinaries];

			if (debugLevel >= 1)
				printf("Source instr %x\n", (int) oneInstruction);

			/***************************************************************/
			/*  We first decode the source instruction  				   */
			/***************************************************************/

			acu6 op = oneInstruction.slc<6>(26);

			acu6 funct = oneInstruction.slc<6>(0);
			acu5 shamt = oneInstruction.slc<5>(6);
			acu5 rd = oneInstruction.slc<5>(11);
			acu5 rt = oneInstruction.slc<5>(16);
			acu5 rs = oneInstruction.slc<5>(21);
			acu5 regimm = rt;
			acs16 address = oneInstruction.slc<16>(0);

			acs26 tgtadr = oneInstruction.slc<26>(0);;

			acs26 correctedTgtadr = tgtadr - (addressStart>>2);

			acu1 pred1_ena = 0, pred2_ena = 0, dest_ena = 0;
			acu6 pred1_reg = rs, pred2_reg = rt, dest_reg;


			//We determine whether to use pred1
			if (!(op == J || op == JAL ) & !(op == 0 && funct == 0 && rd == 0)){
				pred1_ena = 1;
			}
			if (op == R && (funct == SLL || funct == SRL || funct == SRA || funct == 0 || funct == SLLV || funct == SRLV || funct == SRAV)){
				pred1_reg = rt;
			}

			//We determine whether to use pred2
			if (op == BEQ || op == BNE || op == BLEZ || op == BGTZ || op == SB || op == SH || op == SW ||
					(op == R && !(funct == SLL || funct == SRL || funct == SRA || funct == 0)))
				pred2_ena = 1;

			if (op == R && (funct == SLLV || funct == SRLV || funct == SRAV)){
				pred2_reg = rs;
			}

			//We determine whether to use dest
			if (op == R && funct != JALR){
				dest_ena = 1;
				dest_reg = rd;
			}
			else if (! (op == SB || op == SH || op == SW || op == J || op == JAL || op == BEQ || op == BNE || op == BLEZ || op == BGTZ )){
				dest_ena = 1;
				dest_reg = rt;
			}

			if (pred1_reg == 31)
				pred1_reg = 63;

			if (pred2_reg==31)
				pred2_reg=63;

			if (dest_reg==31)
				dest_reg=63;

			/***************************************************************/
			/*  We assemble the instruction  							   */
			/***************************************************************/

			if (op == R){
				//Instrucion is R-Type
				if (funct == SLL || funct == SRL || funct == SRA){

					binaries = assembleRiInstruction(functBinding[funct], dest_reg, pred1_reg, shamt);

				}
				else if (funct == MFHI || funct == MFLO){

					ac_int<6, false> pred1_mul = (funct==MFHI) ? 35:34;
					binaries = assembleRInstruction(0x41, dest_reg, 0, pred1_mul);


				}
				else if(funct == JALR || funct == JR){

					setBoundaries1 = 1;
					boundary1 = indexInSourceBinaries + 2;

					if (pred1_reg == 63){
						binaries = assembleIInstruction(VEX_IGOTO, 0, pred1_reg);

					}
					else{
						//FIXME should be able to add two instr at the same cycle... This would remove an insertion
						binaries = assembleRInstruction(VEX_ADD, 33, pred1_reg, 0);

						nextInstruction = assembleIInstruction(functBinding[funct], 4, 0);
						enableNextInstruction = 1;
						isInsertion = 1;
					}



				}
				else if (funct == MULT || funct == MULTU){

					binaries = assembleRInstruction(VEX_MPYLO, 34, pred2_reg, pred1_reg);
					stage = 2;

					nextInstruction = assembleRInstruction(VEX_MPYHI, 35, pred2_reg, pred1_reg);
					nextInstruction_stage = 2;
					enableNextInstruction = 1;
					isInsertion = 1;

				}
				else if (funct == DIV || funct == DIVU){

					if (blocksBoundaries[indexInSourceBinaries+1]){

						previousBinaries.set_slc(32, assembleRInstruction(VEX_DIVLO, 34, pred1_reg, pred2_reg));
						stage = 2;

						binaries = assembleRInstruction(VEX_DIVHI, 35, pred1_reg, pred2_reg);

					}
					else{

						binaries = assembleRInstruction(VEX_DIVLO, 34, pred1_reg, pred2_reg);
						stage = 2;

						nextInstruction = assembleRInstruction(VEX_DIVHI, 35, pred1_reg, pred2_reg);
						nextInstruction_stage = 2;
						enableNextInstruction = 1;
						isInsertion = 1;
					}


				}
				else if (funct == BREAK){
					binaries = assembleIInstruction(functBinding[funct], 0, 0);
				}
				else {

//					if (functBinding[funct] == -1 || functBinding[funct] == -2){
//						printf("funct not handled %x in %x\n", funct, oneInstruction);
//						exit(-1);
//					}

					binaries =  assembleRInstruction(functBinding[funct], dest_reg, pred1_reg, pred2_reg);


				}
			}
			else if (op == LUI){
				//Operation Load Upper Immediate is turned into a MOVI and a SLLi instructions.
				//If the instruction is not at the start point of a block, we can add the movi at the previous
				//cycle to prevent an insertion. Otherwise, we have to do the insertion...

				if (blocksBoundaries[indexInSourceBinaries]){
					binaries = assembleIInstruction(VEX_MOVI, address, dest_reg);
					nextInstruction = assembleRiInstruction(VEX_SLLi, dest_reg, dest_reg, 16);

					//Mark the insertion
					nextInstruction_stage = 0;
					enableNextInstruction = 1;
					isInsertion = 1;
				}
				else{
					previousBinaries.set_slc(0, assembleIInstruction(VEX_MOVI, address, 33));
					binaries = assembleRiInstruction(VEX_SLLi, dest_reg, 33, 16);
				}


			}
			else if (op == SB || op == SH || op == SW || op == LB || op == LH || op == LW || op == LBU || op == LHU){
				//Memory access operations.
				//These operation have special encoding as the use of the immediate value is mandatory.
				//Consequently, if immediate value is too big to fit in the 13 bits dedicated, we cannot simply perform a
				//MOVI at the previous cycle. We will have to MOV the immediate in a register, add it to the second
				//second register value and use this result as an address, resulting in three operations.
				//
				//Similarly to LUI instructions, we may use the previous cycle to hide one insertion ONLY if the
				//instruction is not the first of a block/

				if (address >= 4096 || address < -4096){
					//Immediate value is too high
					if (blocksBoundaries[indexInSourceBinaries] || !blocksBoundaries[indexInSourceBinaries+1]){

						binaries = assembleRiInstruction(VEX_SH3bADDi, 32, pred1_reg, address.slc<16>(3));

						ac_int<13, true> lowerThreeBits = address - (address & 0xfff8);


						nextInstruction = assembleRiInstruction(opcodeBinding[op], pred2_reg, 32, lowerThreeBits);
						nextInstruction_stage = 1;
						enableNextInstruction = 1;

						isInsertion = 1;
					}
					else{
						//We can hide one insertion using previous cycle
						previousBinaries.set_slc(0, assembleRiInstruction(VEX_SH3bADDi, 32, pred1_reg, address.slc<16>(3)));

						ac_int<13, true> lowerThreeBits = address - (address & 0xfff8);


						binaries = assembleRiInstruction(opcodeBinding[op], pred2_reg, 32, lowerThreeBits);
						stage = 1;

					}
				}
				else{
					//Immediate value fit in instruction encoding

					binaries = assembleRiInstruction(opcodeBinding[op], pred2_reg, pred1_reg, address.slc<13>(0));
					stage = 1;
				}


			}
			else if (op == J){

				//We fill information on block boundaries

				setBoundaries1 = 1;
				boundary1 = correctedTgtadr;
				unresolved_jump_src = indexInDestinationBinaries;
				unresolved_jump_type = UNRESOLVED_JUMP_ABSOLUTE;
				setUnresolvedJump = 1;

				setBoundaries2 = 1;
				boundary2 = indexInSourceBinaries + 2;


				binaries = assembleIInstruction(opcodeBinding[op], 0, 0);


			}
			else if (op == BEQ || op == BNE){

				//We fill information on block boundaries

				if (address >= 32768)
					address = address - 65536;

				if (rt == 0){
					setBoundaries1 = 1;
					boundary1 = (address+indexInSourceBinaries) + 1;
					setBoundaries2 = 1;
					boundary2 = indexInSourceBinaries + 2;

					unresolved_jump_src = indexInDestinationBinaries;
					unresolved_jump_type = UNRESOLVED_JUMP_RELATIVE;
					setUnresolvedJump = 1;

					binaries = assembleIInstruction((op == BEQ) ? VEX_BRF : VEX_BR, 0, pred1_reg);
				}
				else{

					setBoundaries1 = 1;
					boundary1 = (address+indexInSourceBinaries) + 1;
					setBoundaries2 = 1;
					boundary2 = indexInSourceBinaries + 2;

					unresolved_jump_src = indexInDestinationBinaries+1;
					unresolved_jump_type = UNRESOLVED_JUMP_RELATIVE;
					setUnresolvedJump = 1;

					binaries = assembleRInstruction(opcodeBinding[op], 32, pred2_reg, pred1_reg);

					nextInstruction = assembleIInstruction(0x25, 0, 32);


					nextInstruction_stage = 0;
					enableNextInstruction = 1;

					isInsertion = 1;
				}
			}
			else if (op == BLEZ || op == BGTZ || (op == REGIMM && (rt == 0 || rt == 1))){

				//We fill information on block boundaries

				if (address >= 32768)
					address = address - 65536;

				setBoundaries1 = 1;
				boundary1 = (address+indexInSourceBinaries) + 1;
				setBoundaries2 = 1;
				boundary2 = indexInSourceBinaries + 2;

				unresolved_jump_src = indexInDestinationBinaries+1;
				unresolved_jump_type = UNRESOLVED_JUMP_RELATIVE;
				setUnresolvedJump = 1;


				ac_int<7, false> new_opcode = (op==REGIMM) ? regimmBindings[rt] : opcodeBinding[op];
				binaries = assembleRInstruction(new_opcode, 32, pred1_reg, 0);

				nextInstruction = assembleIInstruction(0x25, 0, 32);
				nextInstruction_stage = 0;
				enableNextInstruction = 1;

				isInsertion = 1;


			}
			else if (op == JAL){

				//We fill information on block boundaries

				setBoundaries1 = 1;
				boundary1 = correctedTgtadr;
				setBoundaries2 = 1;
				boundary2 = indexInSourceBinaries + 2;

				unresolved_jump_src = indexInDestinationBinaries;
				unresolved_jump_type = UNRESOLVED_JUMP_ABSOLUTE;
				setUnresolvedJump = 1;

				//We also fill information on procedure boundaries
				if (!correctedTgtadr[25])
					proceduresBoundaries[correctedTgtadr] = 1;


				binaries= assembleIInstruction(opcodeBinding[op], 0, 0);


			}
			else if ((op == ADDIU || op == ADDI) && rs == 0){
				//We are facing a li instruction (sort of movi)
				binaries = assembleIInstruction(0x28, address, dest_reg);

			}
			else if (op == SLTIU || op == ORI || op == ANDI || op == XORI){
				ac_int<19, true> imm19 = address;
				imm19 = imm19 & 0xffff;
				if (imm19>=8192){
					if (blocksBoundaries[indexInSourceBinaries]){
						binaries = assembleIInstruction(VEX_MOVI, imm19, 33);

						nextInstruction = assembleRInstruction(opcodeBinding[op]-0x20, dest_reg, pred1_reg, 33);
						nextInstruction_stage = 0;
						enableNextInstruction = 1;

						isInsertion = 1;
					}
					else {
						previousBinaries.set_slc(0, assembleIInstruction(VEX_MOVI, imm19, 33));
						binaries = assembleRInstruction(opcodeBinding[op]-0x20, dest_reg, pred1_reg, 33);
					}
				}
				else {
					binaries = assembleRiInstruction(opcodeBinding[op], dest_reg, pred1_reg, address.slc<13>(0));

				}
			}
			else{ //For all other instructions

				if (address >= 4096 || address < -4096){

					ac_int<19, true> imm19 = address;

					if (blocksBoundaries[indexInSourceBinaries]){
						binaries = assembleIInstruction(VEX_MOVI, imm19, 33);

						nextInstruction = assembleRInstruction(opcodeBinding[op]-0x20, dest_reg, pred1_reg, 33);
						nextInstruction_stage = 0;
						enableNextInstruction = 1;

						isInsertion = 1;
					}
					else {
						previousBinaries.set_slc(0, assembleIInstruction(VEX_MOVI, imm19, 33));
						binaries = assembleRInstruction(opcodeBinding[op]-0x20, dest_reg, pred1_reg, 33);
					}
				}
				else {
					binaries = assembleRiInstruction(opcodeBinding[op], dest_reg, pred1_reg, address.slc<13>(0));

				}
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

		if (setUnresolvedJump && boundary1 >= 0){
			//We add the instruction to the list of unresolved destinations
			unresolvedJumps_src[numberUnresolvedJumps] = unresolved_jump_src;
			unresolvedJumps_type[numberUnresolvedJumps] = unresolved_jump_type;
			unresolvedJumps[numberUnresolvedJumps++] = boundary1;
		}

		if (setBoundaries2)
			blocksBoundaries[boundary2] = 1;

		if (!wasExternalInstr)
			indexInSourceBinaries++;

		if (!droppedInstruction)
			indexInDestinationBinaries++;

		if (debugLevel >= 1)
			printf("Destination instr %x\n", (int)binaries);

	}


	destinationBinaries[previousIndex] = previousBinaries;

	insertions[0] = localNumberInsertions;
	return (indexInDestinationBinaries-placeCode) + (numberUnresolvedJumps<<18);
}
#endif





#ifndef __USE_AC
int firstPassTranslator(unsigned char* code,
		unsigned int *size,
		unsigned int addressStart,
		unsigned char* destinationBinaries,
		unsigned int *placeCode,
		unsigned int &numberInsertions,
		int *insertions,
		short* blocksBoundaries,
		short* proceduresBoundaries){

	unsigned int unresolvedJumps_src[512];
	unsigned char unresolvedJumps_type[512];
	unsigned int unresolvedJumps[512];
	unsigned int numberUnresolvedJumps = 0;


	int oneInstructionIndex;
	int destinationIndex;


	//We fetch values for the current procedure
	int start = 0;



	oneInstructionIndex = start;
	destinationIndex = *placeCode;

	unsigned int nextInstruction, nextInstruction_stage;
	unsigned char enableNextInstruction;
	unsigned char nextInstructionNop = 0;


	unsigned int reg1_mul = 0, reg2_mul = 0, imm_mul = 0, is_imm_mul = 0;

	unsigned int binaries=0;

	blocksBoundaries[0] |= 1;
	proceduresBoundaries[0] = 1;


	while (oneInstructionIndex < *size){

		char stage = 0;
		unsigned int binaries = 0;

		char wasExternalInstr = 0;
		char droppedInstruction = 0;

		if (enableNextInstruction){
			binaries = nextInstruction;
			stage = nextInstruction_stage;
			wasExternalInstr = 1;
			enableNextInstruction = 0;
		}
		else if (nextInstructionNop){
			binaries = 0;
			stage = 0;
			wasExternalInstr = 1;
			nextInstructionNop = 0;
			insertions[numberInsertions++] = oneInstructionIndex;

		}
		else{

			unsigned int oneInstruction = (code[4*oneInstructionIndex] << 24)
						+ (code[4*oneInstructionIndex+1] << 16)
						+ (code[4*oneInstructionIndex+2] << 8)
						+ (code[4*oneInstructionIndex+3]);

			if (debugLevel >= 1)
				printf("Source instr %x\n",oneInstruction);

			char op = (oneInstruction >> 26);

			char funct = (oneInstruction & 0x3f);
			int shamt = ((oneInstruction >> 6) & 0x1f);
			char rd = ((oneInstruction >> 11) & 0x1f);
			char rt = ((oneInstruction >> 16) & 0x1f);
			char rs = ((oneInstruction >> 21) & 0x1f);
			int regimm = rt;
			int address = (oneInstruction & 0xffff);

			int tgtadr = (oneInstruction & 0x3ffffff);

			int correctedTgtadr = tgtadr - (addressStart>>2);

			//TODO gérer les mult en créant des variables LI et hO. En cas de movli on fait un mpyli...


			char pred1_ena = 0, pred2_ena = 0, dest_ena = 0;
			char pred1_reg = rs, pred2_reg = rt, dest_reg;



			//We determine whether to use pred1
			if (!(op == J || op == JAL ) & !(op == 0 && funct == 0 && rd == 0)){
				pred1_ena = 1;
			}
			if (op == R && (funct == SLL || funct == SRL || funct == SRA || funct == 0)){
				pred1_reg = rt;
			}

			//We determine whether to use pred2
			if (op == BEQ || op == BNE || op == BLEZ || op == BGTZ || op == SB || op == SH || op == SW ||
					(op == R && !(funct == SLL || funct == SRL || funct == SRA || funct == 0)))
				pred2_ena = 1;

			//We determine whether to use dest
			if (op == R && funct != JALR){
				dest_ena = 1;
				dest_reg = rd;
			}
			else if (! (op == SB || op == SH || op == SW || op == J || op == JAL || op == BEQ || op == BNE || op == BLEZ || op == BGTZ )){
				dest_ena = 1;
				dest_reg = rt;
			}

			if (pred1_reg == 31)
				pred1_reg = 63;

			if (pred2_reg==31)
				pred2_reg=63;

			if (dest_reg==31)
				dest_reg=63;
			/***************************************************************/
			/*  We generate the instruction  */
			 binaries=0;


			if (op == R){
				//Instrucion is R-Type
				if (funct == SLL || funct == SRL || funct == SRA){

					binaries += functBinding[funct]; //opcode
					binaries += 1 << 7; //instruction is immediate
					binaries += 0 << 8; //instruction do not use br register
					binaries += shamt << 9; // Immediate value
					binaries += dest_reg << 20; //Destination
					binaries += pred1_reg << 26; //Source reg

				}
				else if (funct == MFHI || funct == MFLO){

					binaries += functBinding[funct]; //opcode
					binaries += (is_imm_mul & 0x1) << 7; //immediate
					binaries += 0 << 8; //instruction do not use br register

					if (is_imm_mul){
						binaries += imm_mul << 9; // Destination
						binaries += dest_reg << 20; //source 1
						binaries += reg1_mul << 26; //Source 2
					}
					else {
						binaries += dest_reg << 14; // Destination
						binaries += reg1_mul << 20; //source 1
						binaries += reg2_mul << 26; //Source 2
					}


					nextInstructionNop = 1;
				}
				else if(funct == JALR){
					blocksBoundaries[oneInstructionIndex] |= 2;

					binaries += functBinding[funct]; //opcode
					binaries += pred1_reg << 26; // Immediate value

					//FIXME here we should jump to a exception handling part
				}
				else if (funct == JR){
					blocksBoundaries[oneInstructionIndex] |= 2;

					binaries += functBinding[funct]; //opcode
					binaries += pred1_reg << 26; // Immediate value

					//FIXME here we should jump to a exception handling part
				}
				else if (funct == MULT || funct == MULTU){
					reg1_mul = pred1_reg;
					reg2_mul = pred2_reg;
					is_imm_mul = 0;
					stage = 3;
					droppedInstruction = 1;
					insertions[numberInsertions++] = -oneInstructionIndex;

				}
				else {

					if (functBinding[funct] == -1 || functBinding[funct] == -2){
						printf("funct not handled %x in %x\n", funct, oneInstruction);
						exit(-1);
					}

					binaries += functBinding[funct]; //opcode
					binaries += 0 << 7; //instruction is not immediate
					binaries += 0 << 8; //instruction do not use br register
					binaries += dest_reg << 14; // Destination
					binaries += pred1_reg << 20; //source 1
					binaries += pred2_reg << 26; //Source 2 FIXME: check pred order
				}
			}
			else if (op == LUI){
				//First instruction is movi
				binaries +=  0x28; //opcode
				binaries += address << 7; // Immediate value
				binaries += dest_reg << 26; //Destination

				//Second instruction is shl
				nextInstruction = 0x4f; //opcode
				nextInstruction += 1 << 7; //instruction is immediate
				nextInstruction += 0 <<8; // instruction is not br
				nextInstruction += 16 << 9; // immediate value is 16
				nextInstruction += dest_reg << 20; //destination
				nextInstruction += dest_reg << 26; //source
				nextInstruction_stage = 0;

				enableNextInstruction = 1;

				insertions[numberInsertions++] = oneInstructionIndex;
			}
			else if (op == SB || op == SH || op == SW){
				/****************************/
				/* We update lastWriterOnMemory and add required dependencies to keep memory coherence */

				binaries += opcodeBinding[op]; //opcode
				binaries += 1 << 7; //instruction is immediate
				binaries += 0 << 8; //instruction do not use br register
				binaries += (address & 0x7ff) << 9; // Immediate value
				binaries += pred2_reg << 20; //Destination
				binaries += pred1_reg << 26; //Source reg FIXME: check pred order

				//Instruction is a memory access and needs to be mapped on the correct place
				stage = 1;

			}
			else if (op == LB || op == LH || op == LW || op == LBU || op == LHU){

				binaries += opcodeBinding[op]; //opcode
				binaries += 1 << 7; //instruction is immediate
				binaries += 0 << 8; //instruction do not use br register
				binaries += (address & 0x7ff) << 9; // Immediate value
				binaries += dest_reg << 20; //Destination
				binaries += pred1_reg << 26; //Source reg

				//Instruction is a memory access and needs to be mapped on the correct place
				stage = 1;

			}
			else if (op == J){

				//We fill information on block boundaries
				blocksBoundaries[oneInstructionIndex] |= 2;
				blocksBoundaries[correctedTgtadr] |= 1;

				binaries += opcodeBinding[op]; //opcode

				//We add the instruction to the list of unresolved destinations
				unresolvedJumps_src[numberUnresolvedJumps] = destinationIndex;
				unresolvedJumps_type[numberUnresolvedJumps] = UNRESOLVED_JUMP_ABSOLUTE;
				unresolvedJumps[numberUnresolvedJumps++] = correctedTgtadr;


			}
			else if (op == BEQ || op == BNE){

				//We fill information on block boundaries
				blocksBoundaries[oneInstructionIndex] |= 2;
				if (address >= 32768)
					address = address - 65536;
				blocksBoundaries[(address+oneInstructionIndex) + 1] |= 1;
				blocksBoundaries[oneInstructionIndex + 2] |= 1;

				//First instruction is cmp r32 reg1 reg2
				binaries +=  opcodeBinding[op]; //opcode (here the binding will be done with cmpeq/cmpne
				binaries += 0 << 7; //instruction is immediate
				binaries += 0 << 8; //instruction do not use br register
				binaries += 32 << 14; //Destination
				binaries += pred1_reg << 20; //Destination
				binaries += pred2_reg << 26; //Source reg

				//Second instruction is br
				nextInstruction = 0x25; //opcode
				nextInstruction += 32 << 26; //source register
				nextInstruction_stage = 0;
				enableNextInstruction = 1;


				insertions[numberInsertions++] = oneInstructionIndex;

				//We add the instruction to the list of unresolved destinations (the instruction will be added at next cycle)
				unresolvedJumps_src[numberUnresolvedJumps] = destinationIndex+1;
				unresolvedJumps_type[numberUnresolvedJumps] = UNRESOLVED_JUMP_RELATIVE;
				unresolvedJumps[numberUnresolvedJumps++] = (address+oneInstructionIndex) + 1;

			}
			else if (op == BLEZ || op == BGTZ || op == REGIMM){


				//We fill information on block boundaries
				blocksBoundaries[oneInstructionIndex] |= 2;
				if (address >= 32768)
					address = address - 65536;
				blocksBoundaries[(address+oneInstructionIndex) + 1] |= 1;
				blocksBoundaries[oneInstructionIndex + 2] |= 1;


				//First instruction is cmp r32 reg1 reg2
				binaries +=  (op==REGIMM) ? regimmBindings[rt] : opcodeBinding[op]; //opcode (here the binding will be done with cmpeq/cmpne
				binaries += 0 << 7; //instruction is immediate
				binaries += 0 << 8; //instruction do not use br register
				binaries += 32 << 14; //Destination
				binaries += pred1_reg << 20; //Destination
				binaries += 0 << 26; //Source reg

				//Second instruction is br
				nextInstruction = 0x25; //opcode
				nextInstruction += 32 << 26; //source register
				nextInstruction_stage = 0;

				enableNextInstruction = 1;


				insertions[numberInsertions++] = oneInstructionIndex;

				//We add the instruction to the list of unresolved destinations (the instruction will be added at next cycle)
				unresolvedJumps_src[numberUnresolvedJumps] = destinationIndex+1;
				unresolvedJumps_type[numberUnresolvedJumps] = UNRESOLVED_JUMP_RELATIVE;
				unresolvedJumps[numberUnresolvedJumps++] = (address+oneInstructionIndex) + 1;

			}
			else if (op == JAL){

				//We fill information on block boundaries
				blocksBoundaries[oneInstructionIndex] |= 2;
				blocksBoundaries[oneInstructionIndex+2] |= 1;
				blocksBoundaries[correctedTgtadr] |= 1;

				//We also fill information on procedure boundaries
				proceduresBoundaries[correctedTgtadr] = 1;

				binaries += opcodeBinding[op]; //opcode

				//We add the instruction to the list of unresolved destinations
				unresolvedJumps_src[numberUnresolvedJumps] = destinationIndex;
				unresolvedJumps_type[numberUnresolvedJumps] = UNRESOLVED_JUMP_ABSOLUTE;
				unresolvedJumps[numberUnresolvedJumps++] = correctedTgtadr;

			}
			else{ //For all other instructions

				//These instruction should not be immediate
				if (opcodeBinding[op] == -1 || opcodeBinding[op] == -2){
					printf("Opcode not handled %x in %x\n", op, oneInstruction);
					exit(-1);
				}

				binaries += opcodeBinding[op]; //opcode
				binaries += 1 << 7; //instruction is immediate
				binaries += 0 << 8; //instruction do not use br register
				binaries += (address & 0x7ff) << 9; // Immediate value
				binaries += dest_reg << 20; //Destination
				binaries += pred1_reg << 26; //Source reg
			}
		}


		writeInt(destinationBinaries, 16*destinationIndex, (stage == 0) ? binaries : 0);
		writeInt(destinationBinaries, 16*destinationIndex+4, (stage == 1) ? binaries : 0);
		writeInt(destinationBinaries, 16*destinationIndex+8, 0);
		writeInt(destinationBinaries, 16*destinationIndex+12, (stage == 3) ? binaries : 0);


		if (debugLevel >= 1)
			printf("Destination instr %x\n",binaries);

		if (!wasExternalInstr)
			oneInstructionIndex++;

		if (!droppedInstruction)
			destinationIndex++;


	}



	/************************************************************/
	/* Resolution of unresolved jumps 							*/
	/************************************************************/

	for (int oneUnresolvedJump = 0; oneUnresolvedJump<numberUnresolvedJumps; oneUnresolvedJump++){
		unsigned int source = unresolvedJumps_src[oneUnresolvedJump];
		unsigned int initialDestination = unresolvedJumps[oneUnresolvedJump];
		unsigned char type = unresolvedJumps_type[oneUnresolvedJump];


		if (type == UNRESOLVED_JUMP_ABSOLUTE){
			//We count the insertion from start

			initialDestination += (*placeCode);
			for (int oneInsertion = 0; oneInsertion < numberInsertions; oneInsertion++){
				if (insertions[oneInsertion] < 0 && (-insertions[oneInsertion]) < initialDestination){
					initialDestination--;
				}
				else if (insertions[oneInsertion] < initialDestination){
					initialDestination++;
				}
			}

			//We modify the jump instruction to make it jump at the correct place
			unsigned int oldJump = readInt(destinationBinaries, 16*source);
			writeInt(destinationBinaries, 16*source, oldJump + ((initialDestination & 0x7ffff)<<7));

		}
		else{
			//We count the insertion between source and destination
			for (int oneInsertion = 0; oneInsertion < numberInsertions; oneInsertion++){
				if (insertions[oneInsertion] < 0 && -insertions[oneInsertion] <= initialDestination){
					initialDestination--;
				}
				else if (insertions[oneInsertion] <= initialDestination){
					initialDestination++;
				}
			}

			//We modify the jump instruction to make it jump at the correct place
			unsigned int oldJump = readInt(destinationBinaries, 16*source);
			writeInt(destinationBinaries, 16*source, oldJump + ((initialDestination & 0x7ffff)<<9));

		}

	}

	numberUnresolvedJumps = 0;
	numberInsertions = 0;
	*placeCode += destinationIndex * 16;

	/************************************************************/
	/************************************************************/

}
#endif
