/*
 * buildTraces.cpp
 *
 *  Created on: 22 mai 2017
 *      Author: simon
 */

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <isa/vexISA.h>
#include <lib/endianness.h>
#include <lib/log.h>
#include <transformation/buildTraces.h>
#include <types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

bool VERBOSE = 1;


/****************************************************************************************************************
 * Function getReadWriteRegisters will analyze the block and build the list of registers that are read in the block and of the register
 * written in the block.
 *
 * Arguments:
 * 	block: [in] pointer to the block to analyze
 * 	readRegs: [out] pointer to an array of boolean representing register read
 *  writeRegs: [out] pointer to an array of boolean representing register written
 *
 ****************************************************************************************************************/

void getReadWriteRegisters(IRBlock *block, bool *readRegs, short *writeRegs){

	//First we initialize the two arrays with false
	for (int oneReg = 0; oneReg<128; oneReg++){
		readRegs[oneReg] = false;
		writeRegs[oneReg] = 0;
	}

	//For every instr, we analyze the register that are read and written
	for (int oneInstr = 0; oneInstr<block->nbInstr; oneInstr++){

		//We mark the written reg
		short writtenReg = getDestinationRegister(block->instructions, oneInstr);
		if (writtenReg >= 0)
			writeRegs[writtenReg-256] = 1;

		short operands[2];
		char nbOperand = getOperands(block->instructions, oneInstr, operands);

		for (int oneOperand = 0; oneOperand<nbOperand; oneOperand++)
			if (operands[oneOperand] >= 256 && writeRegs[operands[oneOperand]-256] == 0){
					readRegs[operands[oneOperand]-256] = true;
			}

	}
}

IRBlock* unrollLoops(IRBlock *block, bool ignoreRegs, short *outputRegsToIgnore, char nbIgnore){

	/*****
	 * The goal of this function is to merge two block into one.
	 * There are two kinds of merging supported by this function
	 * -> The merging where the entry blocks has two successors. In this case, the execution may leave the block before its end.
	 * 		For this reason we have to write into new register which will be allocated by the scheduler. We then use the conditional
	 * 		instruction to ensure that we do modify the behavior of the program.
	 * -> The merge of two block when there is no escape. In such a case we do not have to take care of conditions,
	 *
	 *******************************************************
	 *******************************************************
	 *******************************************************
	 *
	 * Here are the steps done by the function:
	 * 0. We compute some parameters for the transformation:
	 * 		-> isEscapable is set to true if the first block is a conditional block (so if the second block may not be executed)
	 *
	 * 1. We go through the instructions of the first block. Doing so, we keep track of
	 * 		-> The last instruction that wrote in a given register
	 * 		-> The last 4 memory accesses (read/store)
	 * 		-> The jump and its condition
	 *   All these instructions are inserted in the block unmodified.
	 *
	 * 2. We go through the instructions from the second block. Doing this we still keep track of:
	 * 		-> Last writer on registers from this second block
	 * 	  We also do the following modifications:
	 * 	  	-> If isEscapable we transform memory stores into conditional memory stores, we say writing instruction to allocate
	 * 	  		a new register instead of using the normal one.
	 * 	  	-> For each operand of a given instruction, if the operand is global (>256) then we check if the said register has not
	 * 	  		been modified by the block 1. If so we change the reference by a non global one and we add a data dependency.
	 * 	  		Otherwise, we just change the local reference by adding an offset to the operand (the offset will be the size of
	 * 	  		block 1).
	 * 	  	-> For each dependencies of a given instruction, we add an offset in order to refer to the correct instruction index
	 * 	  		in the newly created block. Once again, this offset will be the size of block 1.
	 * 	  	-> The first memory operation found will depend on the last memory operation found on block 1 to ensure the memory coherence
	 *
	 * 3. If the block is escapable, then we have to add also some conditional instructions:
	 * 		-> One setCond instruction which will set the flag by looking at the condition (found during step 1).
	 * 			Note that this instruction will depend (control dependency) from the conditional instruction found during step one.
	 * 		-> For each register whose value was written on block 2, we insert a setCond which will commit the new value if and only if
	 * 			the condition was fulfilled. This instruction will have a data dependency from the instruction from block 2 that
	 * 			generated the value and a control dependency from the SETCOND instruction and from any previous write from the block 1
	 * 			(to ensure that there is no WAR dependency violated).
	 *
	 * 4. Finally, the new block is placed in the procedure, taking the place of the block 1.
	 */


	//We define which model will be used for the merging:
	// isEscape says if the second part may not be executed
	// useSetc says that we use conditional instruction for solving speculation. Other alternative is the use of correctly handled alloc

	char isEscape = 1;


	/*****************************************************
	 * We start by print debug messages
	 */
	Log::logScheduleProc << "***********Unrolling loop*******************\n";

	block->printBytecode(Log::logScheduleProc);

	Log::logScheduleProc << "\n";


	//********************************************************

	short sizeofEntryBlock = block->nbInstr;

	//We declare the result block:
	//We also compute the unrolling factor. The goal is to get a ~50 instr block
//	int unrollingFactor = (50/block->nbInstr) + 1;
//	if (unrollingFactor>8)
//		unrollingFactor = 8;
	int unrollingFactor = 4;
	if (block->nbInstr > 50)
		unrollingFactor = 180 / block->nbInstr;
	block->unrollingFactor = unrollingFactor;

	int sizeOfResult = block->nbInstr + unrollingFactor*(block->nbInstr + nbIgnore) + block->nbInstr;

	IRBlock *result = new IRBlock(0,0,0);
	result->instructions = (unsigned int*) malloc(sizeOfResult*4*sizeof(unsigned int));
	result->nbSucc = 0;
	result->unrollingFactor = block->unrollingFactor;

	//Arrays that keep track of last write places in each block
	short lastWriteReg[128];

	for (int oneReg = 0; oneReg < 128; oneReg++){
		lastWriteReg[oneReg] = -1;
	}

	//Arrays that keep track of last last memory accesses
	short lastMemInstr[4];
	char nbLastMemInstr = 0;
	unsigned char placeLastMemInstr = 0;
	short lastStore = -1;

	short lastMemInstrSecond[4];
	char nbLastMemInstrSecond = 0;
	unsigned char placeLastMemInstrSecond = 0;
	short lastStoreSecond = -1;

	//last jump/condition of entry block 	short indexOfJump = -1;
	short indexOfJump = -1;

	//****************************************************************************************************
	//We go through the first block to find all written register and memory accesses
	//Because of this we will be able to build dependencies correctly

	for (int oneInstr = 0; oneInstr<sizeofEntryBlock; oneInstr++){


		//We mark the written reg
		short writtenReg = getDestinationRegister(block->instructions, oneInstr);
		if (writtenReg >= 0){
			lastWriteReg[writtenReg-256] = oneInstr;
		}

		//We copy the result
		result->instructions[4*oneInstr+0] = block->instructions[4*oneInstr+0];
		result->instructions[4*oneInstr+1] = block->instructions[4*oneInstr+1];
		result->instructions[4*oneInstr+2] = block->instructions[4*oneInstr+2];
		result->instructions[4*oneInstr+3] = block->instructions[4*oneInstr+3];

		char opcode = getOpcode(block->instructions, oneInstr);



		//If it is a conditional branch, we mark the jump and the condition instr
		if (opcode == VEX_BR || opcode == VEX_BRF || opcode == VEX_BGE || opcode == VEX_BLT || opcode == VEX_BGEU || opcode == VEX_BLTU){
			indexOfJump = oneInstr;

			//If we are unrolling a loop we invert the jump condition
			if (opcode == VEX_BR)
				setOpcode(result->instructions, oneInstr, VEX_BRF);
			else if (opcode == VEX_BRF)
				setOpcode(result->instructions, oneInstr, VEX_BR);
			else if (opcode == VEX_BGE)
				setOpcode(result->instructions, oneInstr, VEX_BLT);
			else if (opcode == VEX_BLT)
				setOpcode(result->instructions, oneInstr, VEX_BGE);
			else if (opcode == VEX_BGEU)
				setOpcode(result->instructions, oneInstr, VEX_BLTU);
			else if (opcode == VEX_BLTU)
				setOpcode(result->instructions, oneInstr, VEX_BGEU);


		}



		//We keep track of last store/load instructions
		char shiftOpcode = opcode >> 3;
		if (shiftOpcode == (VEX_STD >> 3) || opcode == VEX_FSW){
			lastStore = oneInstr;
			nbLastMemInstr = 1;
			placeLastMemInstr = 1;
			lastMemInstr[0] = oneInstr;
		}

		if (shiftOpcode == (VEX_LDD >> 3)  || opcode == VEX_FLW){
			lastMemInstr[placeLastMemInstr] = oneInstr;
			placeLastMemInstr = (placeLastMemInstr+1) & 0x3;

			if (nbLastMemInstr<4)
				nbLastMemInstr++;
		}
	}
	result->nbInstr = block->nbInstr;

	//We insert the first jump
	for (int oneJump = 0; oneJump<block->nbJumps; oneJump++){
			result->addJump(block->jumpIds[oneJump], -1);
			result->successors[result->nbSucc] = block->successors[oneJump+1];
			result->nbSucc++;
	}

	//****************************************************************************************************
	//We insert instructions from second block

	for (int oneUnroll=1; oneUnroll<unrollingFactor; oneUnroll++){

		short indexOfSecondJump = -1;
		char hasStores = 0;

		for (int oneInstr = 0; oneInstr<block->nbInstr; oneInstr++){

			//We copy the instruction into the new block
			result->instructions[(result->nbInstr+oneInstr)*4+0] = block->instructions[oneInstr*4+0];
			result->instructions[(result->nbInstr+oneInstr)*4+1] = block->instructions[oneInstr*4+1];
			result->instructions[(result->nbInstr+oneInstr)*4+2] = block->instructions[oneInstr*4+2];
			result->instructions[(result->nbInstr+oneInstr)*4+3] = block->instructions[oneInstr*4+3];



			short operands[2];
			char nbOperand = getOperands(block->instructions, oneInstr, operands);


			//We update dependencies
			addOffsetToDep(result->instructions, result->nbInstr+oneInstr, result->nbInstr);

			//For each operand register, there are two possibilities:
			// -> The operand is lower than 256, then it is a reference to another instruction from the block: we correct it
			// -> The operand is greater than 256 then it is an access to global register and thus we need to check if there is a dep to add
			for (int oneOperand = 0; oneOperand<nbOperand; oneOperand++){

				if (operands[oneOperand] < 256){
						operands[oneOperand] += result->nbInstr;
				}
				else if (lastWriteReg[operands[oneOperand]-256] != -1){
					addDataDep(result->instructions, lastWriteReg[operands[oneOperand]-256], result->nbInstr+oneInstr);
					operands[oneOperand] = lastWriteReg[operands[oneOperand]-256];
				}
			}
			setOperands(result->instructions, result->nbInstr + oneInstr, operands);

			char opcode = getOpcode(block->instructions, oneInstr);
			char shiftedOpcode = opcode>>3;


			/***************************************************************
			 * We handle memory accesses from the second block:
			 *  -> Stores are turned into conditional store if needed
			 *  -> Dependencies to ensure memory coherence are added to the first memory accesses met
			 */

			if (shiftedOpcode == (VEX_STD>>3) || opcode == VEX_FSW){
				hasStores = 1;

				//If we are in a escapable block, we need to add a dependency from the first jump
				if (isEscape)
					addControlDep(result->instructions, indexOfJump, result->nbInstr+oneInstr);

				//In any case, we have to ensure a dependency from last mem instruction from previous block (this is only needed for first memory store)
				for (int oneLastMemInstr = 0; oneLastMemInstr<nbLastMemInstr; oneLastMemInstr++)
					addControlDep(result->instructions, lastMemInstr[oneLastMemInstr], result->nbInstr+oneInstr);

				nbLastMemInstr = 0;
			}

			if (lastStore != -1 && nbLastMemInstr > 0 && (shiftedOpcode == (VEX_LDD>>3)  || opcode == VEX_FLW)){
				//For the first 4 load instr we add a dependency to ensure the correctness
				if (nbLastMemInstr == 4){
					addControlDep(result->instructions, lastMemInstr[placeLastMemInstr], result->nbInstr+oneInstr);
					lastMemInstr[placeLastMemInstr] = result->nbInstr+oneInstr;
					placeLastMemInstr = (placeLastMemInstr+1) & 0x3;

				}
				else{
					addControlDep(result->instructions, lastStore, result->nbInstr+oneInstr);
					lastMemInstr[placeLastMemInstr] = result->nbInstr+oneInstr;
					placeLastMemInstr = (placeLastMemInstr+1) & 0x3;
				}
			}

			//We keep track of last store/load instructions of second block
			char shiftOpcode = opcode >> 3;
			if (shiftOpcode == (VEX_STD >> 3) || opcode == VEX_FSW){
				lastStoreSecond = result->nbInstr+oneInstr;
				nbLastMemInstrSecond = 1;
				placeLastMemInstrSecond = 1;
				lastMemInstrSecond[0] = result->nbInstr+oneInstr;
			}

			if (shiftOpcode == (VEX_LDD >> 3)  || opcode == VEX_FLW){
				lastMemInstrSecond[placeLastMemInstrSecond] = result->nbInstr+oneInstr;
				placeLastMemInstrSecond = (placeLastMemInstrSecond+1) & 0x3;

				if (nbLastMemInstrSecond<4)
					nbLastMemInstrSecond++;
			}



			short writtenReg = getDestinationRegister(block->instructions, oneInstr);
			if (writtenReg >= 0){
				lastWriteReg[writtenReg-256] = oneInstr+result->nbInstr;

			}
			//We check if there is a jump in the second block
			if (opcode == VEX_GOTO || opcode == VEX_GOTOR || opcode == VEX_BRF || opcode == VEX_BR || opcode == VEX_BGE || opcode == VEX_BLT || opcode == VEX_BGEU || opcode == VEX_BLTU || opcode == VEX_CALL || opcode == VEX_CALLR){
				if (indexOfSecondJump == -1 && indexOfJump != -1){
					addControlDep(result->instructions, indexOfJump, oneInstr+result->nbInstr);
				}

				if (oneUnroll != unrollingFactor-1){
				//If we are unrolling a loop we invert the jump condition
					if (opcode == VEX_BR)
						setOpcode(result->instructions, oneInstr+result->nbInstr, VEX_BRF);
					else if (opcode == VEX_BRF)
						setOpcode(result->instructions, oneInstr+result->nbInstr, VEX_BR);
					else if (opcode == VEX_BGE)
						setOpcode(result->instructions, oneInstr+result->nbInstr, VEX_BLT);
					else if (opcode == VEX_BLT)
						setOpcode(result->instructions, oneInstr+result->nbInstr, VEX_BGE);
					else if (opcode == VEX_BGEU)
						setOpcode(result->instructions, oneInstr+result->nbInstr, VEX_BLTU);
					else if (opcode == VEX_BLTU)
						setOpcode(result->instructions, oneInstr+result->nbInstr, VEX_BGEU);
				}
				indexOfSecondJump = oneInstr+result->nbInstr;

			}

			//The new instruction will write into an allocated register (if we are in a case with escape)
			if (isEscape && writtenReg >= 0)
				setAlloc(result->instructions, result->nbInstr + oneInstr, 1);



		}

		//We register the jump
		for (int oneJump = 0; oneJump<block->nbJumps; oneJump++){
				result->addJump(block->jumpIds[oneJump]+result->nbInstr, -1);
				if (oneUnroll != unrollingFactor-1)
					result->successors[result->nbSucc] = block->successors[oneJump+1];
				else
					result->successors[result->nbSucc] = block->successors[oneJump];

				result->nbSucc++;
		}



		result->nbInstr += block->nbInstr;


		if (isEscape){

			int firstAvailable = 256+34;
			for (int oneReg = 1; oneReg <64; oneReg++){
				if (lastWriteReg[oneReg]>=0){
					if (outputRegsToIgnore[oneReg]){
						//We are authorized to ignore this register which is only alive if we are in the last iteration
						setAlloc(result->instructions, lastWriteReg[oneReg], 1);
						setDestinationRegister(result->instructions, lastWriteReg[oneReg], firstAvailable);
						firstAvailable++;
					}
					else {
						setAlloc(result->instructions, lastWriteReg[oneReg], 0);
						//We add a control dependency from one of the last four cond instruction (note: this array is initialized with four times the setcond instruction)

						addControlDep(result->instructions, indexOfJump, lastWriteReg[oneReg]);
					}

				}
			}

			unsigned int availableReg = 256+34;
			for (int oneReg=0; oneReg<128; oneReg++){
				if (outputRegsToIgnore[oneReg]){

					write128(result->instructions, result->nbInstr*16, assembleRBytecodeInstruction(2, 0, VEX_ADD, lastWriteReg[oneReg], 256, 256+oneReg, 0));
					addDataDep(result->instructions, lastWriteReg[oneReg], result->nbInstr);
					addControlDep(result->instructions, block->jumpIds[0], indexOfSecondJump);

					availableReg++;
					result->nbInstr++;
				}
			}

			indexOfJump = indexOfSecondJump;

			lastMemInstr[0] = lastMemInstrSecond[0];
			lastMemInstr[1] = lastMemInstrSecond[0];
			lastMemInstr[2] = lastMemInstrSecond[0];
			lastMemInstr[3] = lastMemInstrSecond[0];

			nbLastMemInstr = nbLastMemInstrSecond;
			placeLastMemInstr = placeLastMemInstrSecond;
			lastStore = lastStoreSecond;



		}


	}
	/*
	 * Last step of the merging: we merge all jumps and destinators.
	 * Merging is done with the following rules:
	 * 	-> Jumps from entry block are inserted similarly
	 * 	-> Jumps from second block are inserted
	 */




	result->successors[result->nbSucc] = block->successors[block->nbSucc - 1];
	result->nbSucc++;




	result->vliwStartAddress = block->vliwStartAddress;
	result->vliwEndAddress = block->vliwEndAddress;
	result->blockState = IRBLOCK_UNROLLED;

	return result;


}



IRBlock* superBlock(IRBlock *entryBlock, IRBlock *secondBlock, bool ignoreRegs, short *outputRegsToIgnore, char nbIgnore){

	/*****
	 * The goal of this function is to merge two block into one.
	 * There are two kinds of merging supported by this function
	 * -> The merging where the entry blocks has two successors. In this case, the execution may leave the block before its end.
	 * 		For this reason we have to write into new register which will be allocated by the scheduler. We then use the conditional
	 * 		instruction to ensure that we do modify the behavior of the program.
	 * -> The merge of two block when there is no escape. In such a case we do not have to take care of conditions,
	 *
	 *******************************************************
	 *******************************************************
	 *******************************************************
	 *
	 * Here are the steps done by the function:
	 * 0. We compute some parameters for the transformation:
	 * 		-> isEscapable is set to true if the first block is a conditional block (so if the second block may not be executed)
	 *
	 * 1. We go through the instructions of the first block. Doing so, we keep track of
	 * 		-> The last instruction that wrote in a given register
	 * 		-> The last 4 memory accesses (read/store)
	 * 		-> The jump and its condition
	 *   All these instructions are inserted in the block unmodified.
	 *
	 * 2. We go through the instructions from the second block. Doing this we still keep track of:
	 * 		-> Last writer on registers from this second block
	 * 	  We also do the following modifications:
	 * 	  	-> If isEscapable we transform memory stores into conditional memory stores, we say writing instruction to allocate
	 * 	  		a new register instead of using the normal one.
	 * 	  	-> For each operand of a given instruction, if the operand is global (>256) then we check if the said register has not
	 * 	  		been modified by the block 1. If so we change the reference by a non global one and we add a data dependency.
	 * 	  		Otherwise, we just change the local reference by adding an offset to the operand (the offset will be the size of
	 * 	  		block 1).
	 * 	  	-> For each dependencies of a given instruction, we add an offset in order to refer to the correct instruction index
	 * 	  		in the newly created block. Once again, this offset will be the size of block 1.
	 * 	  	-> The first memory operation found will depend on the last memory operation found on block 1 to ensure the memory coherence
	 *
	 * 3. If the block is escapable, then we have to add also some conditional instructions:
	 * 		-> One setCond instruction which will set the flag by looking at the condition (found during step 1).
	 * 			Note that this instruction will depend (control dependency) from the conditional instruction found during step one.
	 * 		-> For each register whose value was written on block 2, we insert a setCond which will commit the new value if and only if
	 * 			the condition was fulfilled. This instruction will have a data dependency from the instruction from block 2 that
	 * 			generated the value and a control dependency from the SETCOND instruction and from any previous write from the block 1
	 * 			(to ensure that there is no WAR dependency violated).
	 *
	 * 4. Finally, the new block is placed in the procedure, taking the place of the block 1.
	 */


	//We define which model will be used for the merging:
	// isEscape says if the second part may not be executed
	// useSetc says that we use conditional instruction for solving speculation. Other alternative is the use of correctly handled alloc

	char isEscape = (entryBlock->nbSucc > 1);
	char useSetc = 0 && (entryBlock->nbInstr<8 ||  secondBlock->nbInstr < 4);

	/*****************************************************
	 * We start by print debug messages
	 */
	Log::logScheduleProc << "***********Merging blocks*******************\n";
	entryBlock->printBytecode(Log::logScheduleProc);
	Log::logScheduleProc <<  "\n";
	secondBlock->printBytecode(Log::logScheduleProc);

	//********************************************************

	short sizeofEntryBlock = entryBlock->nbInstr;

	//We declare the result block:
	int sizeOfResult = entryBlock->nbInstr + secondBlock->nbInstr + (useSetc ? 32 : 0) + nbIgnore;
	IRBlock *result = new IRBlock(0,0,0);
	result->instructions = (unsigned int*) malloc(sizeOfResult*4*sizeof(unsigned int));

	//Arrays that keep track of last write places in each block
	short lastWriteReg[128];
	short lastWriteRegForSecond[128];
	for (int oneReg = 0; oneReg < 128; oneReg++){
		lastWriteReg[oneReg] = -1;
		lastWriteRegForSecond[oneReg] = -1;
	}

	//Arrays that keep track of last last memory accesses
	short lastMemInstr[4];
	char nbLastMemInstr = 0;
	unsigned char placeLastMemInstr = 0;
	short lastStore = -1;

	//last jump/condition of entry block 	short indexOfJump = -1;
	short indexOfJump = -1;
	unsigned short indexOfCondition = -1;

	//****************************************************************************************************
	//We go through the first block to find all written register and memory accesses
	//Because of this we will be able to build dependencies correctly

	for (int oneInstr = 0; oneInstr<sizeofEntryBlock; oneInstr++){


		//We mark the written reg
		short writtenReg = getDestinationRegister(entryBlock->instructions, oneInstr);
		if (writtenReg >= 0){
			lastWriteReg[writtenReg-256] = oneInstr;
		}

		//We copy the result
		result->instructions[4*oneInstr+0] = entryBlock->instructions[4*oneInstr+0];
		result->instructions[4*oneInstr+1] = entryBlock->instructions[4*oneInstr+1];
		result->instructions[4*oneInstr+2] = entryBlock->instructions[4*oneInstr+2];
		result->instructions[4*oneInstr+3] = entryBlock->instructions[4*oneInstr+3];

		char opcode = getOpcode(entryBlock->instructions, oneInstr);



		//If it is a conditional branch, we mark the jump and the condition instr
		if (opcode == VEX_BR || opcode == VEX_BRF || opcode == VEX_BGE || opcode == VEX_BLT || opcode == VEX_BGEU || opcode == VEX_BLTU){
			indexOfJump = oneInstr;

			//If we are unrolling a loop we invert the jump condition
			if (entryBlock == secondBlock){
				if (opcode == VEX_BR)
					setOpcode(result->instructions, oneInstr, VEX_BRF);
				else if (opcode == VEX_BRF)
					setOpcode(result->instructions, oneInstr, VEX_BR);
				else if (opcode == VEX_BGE)
					setOpcode(result->instructions, oneInstr, VEX_BLT);
				else if (opcode == VEX_BLT)
					setOpcode(result->instructions, oneInstr, VEX_BGE);
				else if (opcode == VEX_BGEU)
					setOpcode(result->instructions, oneInstr, VEX_BLTU);
				else if (opcode == VEX_BLTU)
					setOpcode(result->instructions, oneInstr, VEX_BGEU);
			}

			if (useSetc){
				short operands[2];

				fprintf(stderr, "Error: truying to build a trace using setc while no code has been written to build the condition for the setc...\n");
				exit(-1);
				if (operands[0] >= 256){
					if (lastWriteReg[operands[0]-256] != -1)
						indexOfCondition = lastWriteReg[operands[0]-256];
					else
						indexOfCondition = operands[0];
				}
				else
					indexOfCondition = operands[0];
			}
		}



		//We keep track of last store/load instructions
		char shiftOpcode = opcode >> 3;
		if (shiftOpcode == (VEX_STD >> 3) || opcode == VEX_FSW){
			lastStore = oneInstr;
			nbLastMemInstr = 1;
			placeLastMemInstr = 1;
			lastMemInstr[0] = oneInstr;
		}

		if (shiftOpcode == (VEX_LDD >> 3) || opcode == VEX_FLW){
			lastMemInstr[placeLastMemInstr] = oneInstr;
			placeLastMemInstr = (placeLastMemInstr+1) & 0x3;

			if (nbLastMemInstr<4)
				nbLastMemInstr++;
		}
	}
	result->nbInstr = entryBlock->nbInstr;


	//****************************************************************************************************
	//We insert instructions from second block
	short indexOfSecondJump = -1;
	char hasStores = 0;

	for (int oneInstr = 0; oneInstr<secondBlock->nbInstr; oneInstr++){

		//We copy the instruction into the new block
		result->instructions[(sizeofEntryBlock+oneInstr)*4+0] = secondBlock->instructions[oneInstr*4+0];
		result->instructions[(sizeofEntryBlock+oneInstr)*4+1] = secondBlock->instructions[oneInstr*4+1];
		result->instructions[(sizeofEntryBlock+oneInstr)*4+2] = secondBlock->instructions[oneInstr*4+2];
		result->instructions[(sizeofEntryBlock+oneInstr)*4+3] = secondBlock->instructions[oneInstr*4+3];



		short operands[2];
		char nbOperand = getOperands(secondBlock->instructions, oneInstr, operands);


		//We update dependencies
		addOffsetToDep(result->instructions, sizeofEntryBlock+oneInstr, sizeofEntryBlock);

		//For each operand register, there are two possibilities:
		// -> The operand is lower than 256, then it is a reference to another instruction from the block: we correct it
		// -> The operand is greater than 256 then it is an access to global register and thus we need to check if there is a dep to add
		for (int oneOperand = 0; oneOperand<nbOperand; oneOperand++){

			if (operands[oneOperand] < 256){
					operands[oneOperand] += sizeofEntryBlock;
			}
			else if (lastWriteReg[operands[oneOperand]-256] != -1){
				addDataDep(result->instructions, lastWriteReg[operands[oneOperand]-256], sizeofEntryBlock+oneInstr);
				operands[oneOperand] = lastWriteReg[operands[oneOperand]-256];
			}
		}
		setOperands(result->instructions, sizeofEntryBlock + oneInstr, operands);

		char opcode = getOpcode(secondBlock->instructions, oneInstr);
		char shiftedOpcode = opcode>>3;


		/***************************************************************
		 * We handle memory accesses from the second block:
		 *  -> Stores are turned into conditional store if needed
		 *  -> Dependencies to ensure memory coherence are added to the first memory accesses met
		 */

		if (shiftedOpcode == (VEX_STD>>3) || opcode == VEX_FSW){
			hasStores = 1;

			//If we are in a escapable block, we need to add a dependency from the first jump
			if (isEscape)
				addControlDep(result->instructions, indexOfJump, sizeofEntryBlock+oneInstr);

			//In any case, we have to ensure a dependency from last mem instruction from previous block (this is only needed for first memory store)
			for (int oneLastMemInstr = 0; oneLastMemInstr<nbLastMemInstr; oneLastMemInstr++)
				addControlDep(result->instructions, lastMemInstr[oneLastMemInstr], sizeofEntryBlock+oneInstr);

			nbLastMemInstr = 0;
		}

		if (lastStore != -1 && nbLastMemInstr > 0 && (shiftedOpcode == (VEX_LDD>>3) || opcode == VEX_FLW)){
			//For the first 4 load instr we add a dependency to ensure the correctness
			if (nbLastMemInstr == 4){
				addControlDep(result->instructions, lastMemInstr[placeLastMemInstr], sizeofEntryBlock+oneInstr);
				lastMemInstr[placeLastMemInstr] = sizeofEntryBlock+oneInstr;
				placeLastMemInstr = (placeLastMemInstr+1) & 0x3;

			}
			else{
				addControlDep(result->instructions, lastStore, sizeofEntryBlock+oneInstr);
				lastMemInstr[placeLastMemInstr] = sizeofEntryBlock+oneInstr;
				placeLastMemInstr = (placeLastMemInstr+1) & 0x3;
			}
		}


		short writtenReg = getDestinationRegister(secondBlock->instructions, oneInstr);
		if (writtenReg >= 0){
			lastWriteRegForSecond[writtenReg-256] = oneInstr+sizeofEntryBlock;

		}
		//We check if there is a jump in the second block
		if (opcode == VEX_GOTO || opcode == VEX_GOTOR || opcode == VEX_BRF || opcode == VEX_BR || opcode == VEX_BGE || opcode == VEX_BLT || opcode == VEX_BGEU || opcode == VEX_BLTU || opcode == VEX_CALL || opcode == VEX_CALLR){
			if (indexOfSecondJump == -1 && indexOfJump != -1){
				addControlDep(result->instructions, indexOfJump, oneInstr+sizeofEntryBlock);
			}
			indexOfSecondJump = oneInstr+sizeofEntryBlock;


		}

		//The new instruction will write into an allocated register (if we are in a case with escape)
		if (isEscape && writtenReg >= 0)
			setAlloc(result->instructions, sizeofEntryBlock + oneInstr, 1);



	}
	result->nbInstr = entryBlock->nbInstr + secondBlock->nbInstr;


	//We insert a SETCOND depending on the value of the condition
	//This instruction will depend from all previous cond instrucion (if any)
	if (isEscape && useSetc){


		for (int oneReg = 1; oneReg < 64; oneReg++){
			if (lastWriteRegForSecond[oneReg]>=0){

				char opcodeJump = getOpcode(entryBlock->instructions, indexOfJump);
				fprintf(stderr, "Error: truying to build a trace using setc while no code has been written to choose the correct setc...\n");
				exit(-1);
				char opcodeCond;
				if (opcodeJump == VEX_BR)
					opcodeCond = VEX_SETc;
				else
					opcodeCond = VEX_SETFc;

				struct uint128_struct bytecodeInstr = assembleRBytecodeInstruction(2, 0, VEX_SETc, indexOfCondition, lastWriteRegForSecond[oneReg], oneReg+256, 0);
				result->instructions[(result->nbInstr)*4+0] = bytecodeInstr.word96;
				result->instructions[(result->nbInstr)*4+1] = bytecodeInstr.word64;
				result->instructions[(result->nbInstr)*4+2] = bytecodeInstr.word32;
				result->instructions[(result->nbInstr)*4+3] = bytecodeInstr.word0;
				addDataDep(result->instructions, lastWriteRegForSecond[oneReg], result->nbInstr);
				if (lastWriteReg[oneReg]>= 0)
					addControlDep(result->instructions, lastWriteReg[oneReg], result->nbInstr);

				//We add a data dependency from condition instruction
				if (indexOfCondition<256)
					addDataDep(result->instructions, indexOfCondition, result->nbInstr);

				lastWriteRegForSecond[oneReg] = result->nbInstr;


				//If there are more than 1 jump in first block we also need a dep from the previous jump and set instr
				if (entryBlock->nbJumps > 1)
					addControlDep(result->instructions, entryBlock->jumpIds[entryBlock->nbJumps-2], result->nbInstr);

				result->nbInstr++;
			}
		}



		//We correct the jump register if any
		if (indexOfSecondJump != -1){

			char jumpopcode = getOpcode(result->instructions, indexOfSecondJump);
			if (jumpopcode == VEX_BR || jumpopcode == VEX_BRF || jumpopcode == VEX_BGE || jumpopcode == VEX_BLT || jumpopcode == VEX_BGEU || jumpopcode == VEX_BLTU){
				short operands[2];
				if (operands[0] < 256){
					char physicalDest = getDestinationRegister(result->instructions, operands[0]);
					operands[0] = lastWriteRegForSecond[physicalDest];
					addDataDep(result->instructions, operands[0], indexOfSecondJump);
				}
				if (operands[1] < 256){
					char physicalDest = getDestinationRegister(result->instructions, operands[1]);
					operands[1] = lastWriteRegForSecond[physicalDest];
					addDataDep(result->instructions, operands[1], indexOfSecondJump);
				}

				setOperands(result->instructions, indexOfSecondJump, operands);

		}

	#ifndef IR_SUCC
			result->instructions[result->nbInstr*4+0] = result->instructions[indexOfSecondJump*4+0];
			result->instructions[result->nbInstr*4+1] = result->instructions[indexOfSecondJump*4+1];
			result->instructions[result->nbInstr*4+2] = result->instructions[indexOfSecondJump*4+2];
			result->instructions[result->nbInstr*4+3] = result->instructions[indexOfSecondJump*4+3];


			result->instructions[indexOfSecondJump*4+0] = 0;
			result->instructions[indexOfSecondJump*4+1] = 0;
			result->instructions[indexOfSecondJump*4+2] = 0;
			result->instructions[indexOfSecondJump*4+3] = 0;

			indexOfSecondJump = result->nbInstr;
			result->nbInstr++;

	#endif

		}



	}
	else if (isEscape){

		int firstAvailable = 256+34;
		for (int oneReg = 1; oneReg <64; oneReg++){
					if (lastWriteRegForSecond[oneReg]>=0){

						if (ignoreRegs && outputRegsToIgnore[oneReg]){
							//We are authorized to ignore this register which is only alive if we are in the last iteration
							setAlloc(result->instructions, lastWriteRegForSecond[oneReg], 1);
							setDestinationRegister(result->instructions, lastWriteRegForSecond[oneReg], firstAvailable);
							firstAvailable++;
						}
						else {
							setAlloc(result->instructions, lastWriteRegForSecond[oneReg], 0);
							//We add a control dependency from one of the last four cond instruction (note: this array is initialized with four times the setcond instruction)

							addControlDep(result->instructions, indexOfJump, lastWriteRegForSecond[oneReg]);
						}

					}
				}
	}
	result->nbSucc = 0;

	/*
	 * Last step of the merging: we merge all jumps and destinators.
	 * Merging is done with the following rules:
	 * 	-> Jumps from entry block are inserted similarly
	 * 	-> Jumps from second block are inserted
	 */

	for (int oneJump = 0; oneJump<entryBlock->nbJumps; oneJump++){
		char jumpOpcode = getOpcode(entryBlock->instructions, entryBlock->jumpIds[oneJump]);
		/*if (isDropEscape && oneJump == entryBlock->nbJumps-1){
			//We remove the concerned jump
			fprintf(stderr, "%d\n",entryBlock->jumpIds[oneJump]);
			setOpcode(result->instructions, entryBlock->jumpIds[oneJump], VEX_NOP);
//			break;
		}
		else */if (entryBlock == secondBlock){
			result->addJump(entryBlock->jumpIds[oneJump], -1);
			result->successors[result->nbSucc] = entryBlock->successors[oneJump+1];
			result->nbSucc++;
		}
		else if (jumpOpcode != VEX_GOTO && jumpOpcode != VEX_GOTOR && (entryBlock->successors[oneJump] != secondBlock->sourceStartAddress || (entryBlock->successors[oneJump] == secondBlock->sourceStartAddress && entryBlock == secondBlock))){
			result->addJump(entryBlock->jumpIds[oneJump], -1);
			result->successors[result->nbSucc] = entryBlock->successors[oneJump];
			result->nbSucc++;
		}
	}

	if (useSetc){
		for (int oneJump = 0; oneJump<secondBlock->nbJumps; oneJump++){

			//The last jump is moved while using setc
			if (oneJump == secondBlock->nbJumps-1)
				result->addJump(indexOfSecondJump, -1);
			else
				result->addJump(secondBlock->jumpIds[oneJump]+entryBlock->nbInstr, -1);

			if (oneJump<secondBlock->nbSucc){
				result->successors[result->nbSucc] = secondBlock->successors[oneJump];
				result->nbSucc++;
			}
		}
	}
	else{
		for (int oneJump = 0; oneJump<secondBlock->nbJumps; oneJump++){
			result->addJump(secondBlock->jumpIds[oneJump]+entryBlock->nbInstr, -1);
			if (oneJump<secondBlock->nbSucc){
				result->successors[result->nbSucc] = secondBlock->successors[oneJump];
				result->nbSucc++;
			}
		}
	}

	if (secondBlock->nbJumps < secondBlock->nbSucc){
		result->successors[result->nbSucc] = secondBlock->successors[secondBlock->nbSucc - 1];
		result->nbSucc++;
	}


	Log::logScheduleProc << "\n Resulting block is: \n";
	result->printBytecode(Log::logScheduleProc);


	result->vliwStartAddress = entryBlock->vliwStartAddress;
	result->vliwEndAddress = secondBlock->vliwEndAddress;
	result->blockState = entryBlock->blockState;

	return result;


}

void buildTraces(DBTPlateform *platform, IRProcedure *procedure, int optLevel){

	/******************************************************************
	 * Optimization that merges blocks:
	 *
	 * The idea of this optimization is to merge several blocks in order to build bigger one which may lead to more
	 * parallelism. We define several kinds of merging:
	 *
	 * -> Merging three blocks corresponding to a if: in this case, you will work the following way:
	 * 		-> insert all instructions of the two conditionnal blocks working as allocs and with dependencies from
	 * 		last writes in predecessor block.
	 * 		-> If there are memory stores on those blocks, insert a dependency from the br instruction to them and
	 *
	 * -> Merging two blocks to obtain something with more than one output but only one input
	 *
	 */

	IRBlock *blocksToAdd[10];
	int nbBlocksToAdd = 0;

	int nbBlock = 0;
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		if (procedure->blocks[oneBlock]->nbInstr != 0){
			nbBlock++;

			//We do an online computation of average block size and squarred distance before trace construction
			platform->nbBlockProcedureBeforeTrace++;
			double delta = procedure->blocks[oneBlock]->nbInstr - platform->blockProcAverageSizeBeforeTrace;
			platform->blockProcAverageSizeBeforeTrace += (delta/platform->nbBlockProcedureBeforeTrace);
			double delta2 = procedure->blocks[oneBlock]->nbInstr - platform->blockProcAverageSizeBeforeTrace;
			platform->blockProcDistanceBeforeTrace += delta*delta2;

		}
	}

	//We malloc two arrays needed for the analysis
	bool *readRegs = (bool*)  malloc(128*sizeof(bool));
	short *writeRegs = (short*) malloc(128*sizeof(short));

	//We identify all perfect loops to preserve them
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		if (procedure->blocks[oneBlock]->nbInstr != 0){
			IRBlock *block = procedure->blocks[oneBlock];

			//We first check if it is a perfect block
			if (block->nbJumps + 1 == block->nbSucc && block->nbSucc == 2 && block->successors[0] == block->sourceStartAddress){
				block->blockState = IRBLOCK_PERFECT_LOOP;

				if (block->nbInstr<100 && nbBlocksToAdd < 10){
					/**************************************************************************************************
					 * In this situation, we are unrolling a loop.
					 **************************************************************************************************
					 * The block CFG information should remain unchanged : only the number of instruction changes.
					 * The place location will also be modified...
					 **************************************************************************************************/


					if (block->nbJumps == block->nbSucc)
						continue;

					block->blockState = IRBLOCK_UNROLLED;

					Log::logScheduleProc << "******************************************************************\n";
					Log::logScheduleProc << "******************** Perfect loop identified *********************\n";
					Log::logScheduleProc << "******************** " << (unsigned int) block->sourceStartAddress << "  ---  " << (unsigned int) block->sourceEndAddress << " *********************\n";
					Log::logScheduleProc << "******************************************************************\n";

					block->printBytecode(Log::logScheduleProc);

					getReadWriteRegisters(block, readRegs, writeRegs);
					int nbIgnoredRegs = 0;

					for (int oneReg = 0; oneReg<128; oneReg++){
						if (writeRegs[oneReg] && readRegs[oneReg])
							writeRegs[oneReg] = 0;
						else if (writeRegs[oneReg] && !readRegs[oneReg]){
							writeRegs[oneReg] += block->nbInstr;
							nbIgnoredRegs++;
						}
					}

					IRBlock *oneSuperBlock = unrollLoops(block, true, writeRegs, nbIgnoredRegs);



					platform->unrollingCounter++;

					block->nbInstr = oneSuperBlock->nbInstr;

					unsigned int *oldInstruction = block->instructions;
					block->instructions = oneSuperBlock->instructions;
					oneSuperBlock->instructions = oldInstruction;

					block->jumpIds = oneSuperBlock->jumpIds;
					block->nbJumps = oneSuperBlock->nbJumps;
					block->jumpPlaces = oneSuperBlock->jumpPlaces;
					block->nbSucc = oneSuperBlock->nbSucc;
					block->unrollingFactor = oneSuperBlock->unrollingFactor;

					for (int oneSuccessor = 0; oneSuccessor<10; oneSuccessor++)
						block->successors[oneSuccessor] = oneSuperBlock->successors[oneSuccessor];

					oneSuperBlock->nbJumps = 0;
					oneSuperBlock->instructions = NULL;

					Log::logScheduleProc << "******************************************************************\n";
					Log::logScheduleProc << "********************     Modified loop       *********************\n";
					Log::logScheduleProc << "******************************************************************\n";
					block->printBytecode(Log::logScheduleProc);


					//We generate a block for the loop termination
					if (nbIgnoredRegs>0){

//						IRBlock *newBlock = new IRBlock(0,0,block->section);
//						newBlock->sourceStartAddress = block->successors[block->nbSucc-1]->sourceStartAddress - 1;
//						newBlock->sourceEndAddress = newBlock->sourceStartAddress + 1;
//
//						newBlock->instructions = (unsigned int*) malloc(nbIgnoredRegs * 4 * sizeof(unsigned int));
//						newBlock->nbInstr = nbIgnoredRegs;

						//New, extra instruction for reallocation are now inserted at the end of the block.
						//TODO do that in the buildSuperBlock
						//WARNING: only work when unrolling of 1

//						unsigned int availableReg = 256+34;
//						for (int oneReg=0; oneReg<128; oneReg++){
//							if (writeRegs[oneReg]){
//								write128(block->instructions, block->nbInstr*16, assembleRBytecodeInstruction(2, 0, VEX_ADD, writeRegs[oneReg], 256, 256+oneReg, 0));
//								addDataDep(block->instructions, writeRegs[oneReg], block->nbInstr);
//								addControlDep(block->instructions, block->jumpIds[0], block->nbInstr);
//
//								availableReg++;
//								block->nbInstr++;
//							}
//						}


//						newBlock->successors[0] = block->successors[block->nbSucc-1];
//						block->successors[block->nbSucc-1] = newBlock;
//						blocksToAdd[nbBlocksToAdd] = newBlock;
//						nbBlocksToAdd++;

						Log::logScheduleProc << "********************   Successor identified  *********************\n";
						Log::logScheduleProc << "******************************************************************\n";


					}




					int nbPred = 0;
					IRBlock *predecessors[1];

					for (int oneOtherBlock = 0; oneOtherBlock<procedure->nbBlock; oneOtherBlock++){
						IRBlock *otherBlock = procedure->blocks[oneOtherBlock];
						if (otherBlock != block){
							for (int oneSuccessor = 0; oneSuccessor < otherBlock->nbSucc; oneSuccessor++){
								unsigned int successorAddr = otherBlock->successors[oneSuccessor];

								if (successorAddr == block->sourceStartAddress){
									nbPred++;
									predecessors[0] = otherBlock;
								}
							}
						}
					}

					Log::logScheduleProc << "Block has " << nbPred << " predecessor\n";
					if (optLevel>=3){
						memoryDisambiguation(platform, block, predecessors, 1);
						spec_loop_counter++;
					}

//					delete oneSuperBlock;
				}

			}
		}
	}

	//We free the allocated things
	free(readRegs);
	free(writeRegs);

	char changeMade = 1;
	while (changeMade){


		changeMade = 0;
		for (int oneBlock=0; oneBlock<procedure->nbBlock; oneBlock++){
			IRBlock *block = procedure->blocks[oneBlock];

			//For the trace to be eligible, we need that only one otherBlock has the block as a successor (and only once) and that
			// there is no jump for this successor (normal control flow).
			// We also need that these block do not terminates with a call.


			bool predecessorFound = false;
			bool eligible = false;
			IRBlock *firstPredecessor;

			if (block->nbInstr>0){

				//We first check if it is a perfect block
				if (block->blockState == IRBLOCK_PERFECT_LOOP){
					continue;
				}


				for (int oneOtherBlock = 0; oneOtherBlock<procedure->nbBlock; oneOtherBlock++){
					IRBlock *otherBlock = procedure->blocks[oneOtherBlock];

					if (otherBlock->blockState == IRBLOCK_PERFECT_LOOP)
						continue;

					for (int oneSuccessor = 0; oneSuccessor < otherBlock->nbSucc; oneSuccessor++){
						unsigned int successorAddr = otherBlock->successors[oneSuccessor];

						if (successorAddr == block->sourceStartAddress){

							//We found a predecessor of block, it needs to be the only one
							if (predecessorFound){
								eligible = false;
								break;
							}

							predecessorFound = true;
							firstPredecessor = otherBlock;

							//We now check if other requirements are satisfied. By default we assume it is.
							eligible = true;

							if (otherBlock->nbSucc != otherBlock->nbJumps + 1 || oneSuccessor != otherBlock->nbSucc-1){
								//We do not have natural connection (pc+4)
								eligible = false;
								break;
							}
						}

					}

					//If we found a predecessor and if we are not eligible, we can exit the loop
					if (predecessorFound && !eligible)
						break;


				}
			}

			if (eligible && block->nbInstr + firstPredecessor->nbInstr < 220){


				IRBlock *oneSuperBlock = superBlock(firstPredecessor, block, false, NULL, 0);

				if (oneSuperBlock == NULL){
					block->blockState = IRBLOCK_TRACE;
					break;
				}
				else{
					platform->traceConstructionCounter++;
				}

				firstPredecessor->nbInstr = oneSuperBlock->nbInstr;
				free(firstPredecessor->instructions);
				firstPredecessor->instructions = NULL;
				firstPredecessor->instructions = (unsigned int*) malloc(sizeof(unsigned int) * 4 * oneSuperBlock->nbInstr);
				memcpy(firstPredecessor->instructions, oneSuperBlock->instructions, 4*oneSuperBlock->nbInstr*sizeof(unsigned int));

				if (firstPredecessor->nbJumps>0){
					firstPredecessor->nbJumps = 0;
					free(firstPredecessor->jumpIds);
					free(firstPredecessor->jumpPlaces);
				}


				if (oneSuperBlock->nbJumps > 0){
					firstPredecessor->jumpIds = (unsigned char*) malloc(sizeof(unsigned char) * oneSuperBlock->nbJumps);
					firstPredecessor->jumpPlaces = (unsigned int *) malloc(sizeof(unsigned int) * oneSuperBlock->nbJumps);
					for (int oneJump=0; oneJump<oneSuperBlock->nbJumps; oneJump++){
						firstPredecessor->jumpIds[oneJump] = oneSuperBlock->jumpIds[oneJump];
					}
				}
				firstPredecessor->nbJumps = oneSuperBlock->nbJumps;


				firstPredecessor->nbSucc = oneSuperBlock->nbSucc;
				for (int oneSuccessor = 0; oneSuccessor<oneSuperBlock->nbSucc; oneSuccessor++)
					firstPredecessor->successors[oneSuccessor] = oneSuperBlock->successors[oneSuccessor];


				block->nbSucc = 0;
				block->nbInstr = 0;
//				block->vliwStartAddress = 0;
				nbBlock--;

				delete (oneSuperBlock);

				block->blockState = IRBLOCK_TRACE;
				firstPredecessor->blockState = IRBLOCK_TRACE;

				//We add the merged block in the predecessor's list of merged blockState
				if (firstPredecessor->nbMergedBlocks < 10){
					firstPredecessor->mergedBlocks[firstPredecessor->nbMergedBlocks] = block->sourceStartAddress;
					firstPredecessor->nbMergedBlocks++;
				}
				else{
					Log::logError << "Error while building a trace: trying to add a merged block while there's no place left\n";
				}


				firstPredecessor->sourceEndAddress = block->sourceEndAddress;

//					delete block;
//					continue;

				assert(!(firstPredecessor->nbJumps == 2 && firstPredecessor->jumpIds[0] == firstPredecessor->jumpIds[1]));

				changeMade=1;
				break;


			}


		}

	}


	IRBlock **newBlocks = (IRBlock**)  malloc((nbBlock + nbBlocksToAdd)*sizeof(IRBlock*));
	int index = 0;
	int blockToAddId = 0;
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){

		//We may insert here one of the new blocks
		if (blockToAddId < nbBlocksToAdd && blocksToAdd[blockToAddId]->sourceStartAddress < procedure->blocks[oneBlock]->sourceStartAddress){
			newBlocks[index] = blocksToAdd[blockToAddId];
			blocksToAdd[blockToAddId]->reference = &(newBlocks[index]);

			index++;
			blockToAddId++;

		}

		//If it is not empty, we insert the block from the procedure
		if (procedure->blocks[oneBlock]->nbInstr != 0){
			newBlocks[index] = procedure->blocks[oneBlock];
			procedure->blocks[oneBlock]->reference = &(newBlocks[index]);
			if (procedure->entryBlock == procedure->blocks[oneBlock])
				procedure->entryBlock = newBlocks[index];
			index++;
//			if (procedure->blocks[oneBlock]->nbInstr > 20){
//				memoryDisambiguation(platform, procedure->blocks[oneBlock]);
//			}

			//We do an online computation of average block size and squarred distance AFTER trace construction
			platform->nbBlockProcedure++;
			double delta = procedure->blocks[oneBlock]->nbInstr - platform->blockProcAverageSize;
			platform->blockProcAverageSize += (delta/platform->nbBlockProcedure);
			double delta2 = procedure->blocks[oneBlock]->nbInstr - platform->blockProcAverageSize;
			platform->blockProcDistance += delta*delta2;
		}
		else if (!procedure->blocks[oneBlock]->isUndestroyable)
			delete procedure->blocks[oneBlock];
	}


	procedure->blocks = newBlocks;
	procedure->nbBlock = nbBlock+nbBlocksToAdd;



	// We go through the blocks and we build speculation groups
	if (optLevel >= 4){

		int numberPred = 0;
		IRBlock **preds = (IRBlock **) malloc(procedure->nbBlock * sizeof(IRBlock*));
		bool eligibleForSpec = true;

		for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
			IRBlock *block = procedure->blocks[oneBlock];


			if (block->blockState == IRBLOCK_PERFECT_LOOP || block->blockState == IRBLOCK_UNROLLED || block->blockState == IRBLOCK_TRACE || block->nbInstr < 8)
				continue;

			//We check if the block is elligible
			eligibleForSpec = true;
			numberPred = 0;


			if (oneBlock == 0)
				eligibleForSpec = false;

			for (int oneOtherBlock=0; oneOtherBlock<procedure->nbBlock; oneOtherBlock++){
				IRBlock *otherBlock = procedure->blocks[oneOtherBlock];

				if (otherBlock == block)
					continue;

				for (int oneSuccessor = 0; oneSuccessor < otherBlock->nbSucc; oneSuccessor++){
					if (otherBlock->successors[oneSuccessor] == block->sourceStartAddress){
						//The block 'otherBlock' is a predecessor of current block.
						//To be eligible, we need it not to be a loop body...
						if (otherBlock->blockState == IRBLOCK_PERFECT_LOOP){
							eligibleForSpec = false;
							break;
						}


						preds[numberPred] = otherBlock;
						numberPred++;
						break;
					}
				}

			}

			if (eligibleForSpec){
					memoryDisambiguation(platform, block, preds, 0);
					spec_trace_counter++;
			}
		}
	}



}
