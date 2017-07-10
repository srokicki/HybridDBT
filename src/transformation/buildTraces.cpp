/*
 * buildTraces.cpp
 *
 *  Created on: 22 mai 2017
 *      Author: simon
 */
#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>

IRBlock* ifConversion(IRBlock *entryBlock, IRBlock *thenBlock, IRBlock *elseBlock){

}

IRBlock* superBlock(IRBlock *entryBlock, IRBlock *secondBlock){

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
	 * 		-> The last 4 conditional instruction
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

	char isEscape = (entryBlock->nbSucc == 2);

	fprintf(stderr, "***********Merging blocks*******************\n");
	for (int i=0; i<entryBlock->nbInstr; i++){
		printBytecodeInstruction(i, readInt(entryBlock->instructions, i*16+0), readInt(entryBlock->instructions, i*16+4), readInt(entryBlock->instructions, i*16+8), readInt(entryBlock->instructions, i*16+12));
	}

	fprintf(stderr, "\n\n\n\n\n");
	for (int i=0; i<secondBlock->nbInstr; i++){
		printBytecodeInstruction(i, readInt(secondBlock->instructions, i*16+0), readInt(secondBlock->instructions, i*16+4), readInt(secondBlock->instructions, i*16+8), readInt(secondBlock->instructions, i*16+12));
	}

	short sizeofEntryBlock = entryBlock->nbInstr;
	char hasJump = (entryBlock->nbSucc >= 1) && entryBlock->successor1 != secondBlock; //True if there is a way to exit the block in the middle
	char jumpId = entryBlock->nbInstr-1; //We suppose the jump is always the last instruction

	//We declare the result block:
	IRBlock *result = new IRBlock(0,0,0);
	result->instructions = (uint32*) malloc(256*4*sizeof(uint32));

	short lastWriteReg[64];
	short lastWriteRegForSecond[64];

	for (int oneReg = 0; oneReg < 64; oneReg++){
		lastWriteReg[oneReg] = -1;
		lastWriteRegForSecond[oneReg] = -1;
	}

	//Last conditional instr
	char lastCondInstr[4];
	char nbLastCondInstr = 0;
	char placeLastCondInstr = 0;

	//Last memory accesses
	char lastMemInstr[4];
	char nbLastMemInstr = 0;
	char placeLastMemInstr = 0;

	//****************************************************************************************************
	//We go through the first block to find all written register
	//Because of this we will be able to build dependencies correctly
	unsigned char indexOfJump = -1;
	unsigned char indexOfCondition = -1;
	for (int oneInstr = 0; oneInstr<sizeofEntryBlock; oneInstr++){
		short writtenReg = getDestinationRegister(entryBlock->instructions, oneInstr);
		if (writtenReg >= 0)
			lastWriteReg[writtenReg-256] = oneInstr;

		result->instructions[4*oneInstr+0] = entryBlock->instructions[4*oneInstr+0];
		result->instructions[4*oneInstr+1] = entryBlock->instructions[4*oneInstr+1];
		result->instructions[4*oneInstr+2] = entryBlock->instructions[4*oneInstr+2];
		result->instructions[4*oneInstr+3] = entryBlock->instructions[4*oneInstr+3];

		char opcode = getOpcode(entryBlock->instructions, oneInstr);
		if (opcode == VEX_BR || opcode == VEX_BRF){
			indexOfJump = oneInstr;
			short operands[2];
			char nbOperands = getOperands(entryBlock->instructions, oneInstr, operands);
			if (operands[0] >= 256)
				indexOfCondition = lastWriteReg[operands[0]];
			else
				indexOfCondition = operands[0];
		}

		//We keep track of last cond instruction
		if (opcode == VEX_STDc || opcode == VEX_STWc || opcode == VEX_STHc || opcode == VEX_STBc || opcode == VEX_SETFc || opcode == VEX_SETc){
			lastCondInstr[placeLastCondInstr] = oneInstr;
			placeLastCondInstr = (placeLastCondInstr+1) & 0x3;

			if (nbLastCondInstr<4)
				nbLastCondInstr++;
		}

		if (opcode == VEX_SETCOND || opcode == VEX_SETCONDF){
			nbLastCondInstr = 0;
		}

		//We keep track of last store/load instructions
		char shiftOpcode = opcode >> 3;
		if (shiftOpcode == (VEX_STD >> 3)){
			lastMemInstr[0] = oneInstr;
			lastMemInstr[1] = oneInstr;
			lastMemInstr[2] = oneInstr;
			lastMemInstr[3] = oneInstr;
			nbLastMemInstr = 4;
		}

		if (shiftOpcode == (VEX_LDD >> 3)){
			lastMemInstr[placeLastMemInstr] = oneInstr;
			placeLastMemInstr = (placeLastMemInstr+1) & 0x3;

			if (nbLastMemInstr<4)
				nbLastMemInstr++;
		}
	}
	result->nbInstr = entryBlock->nbInstr;

	//Prints for debug
	fprintf(stderr, "\n\n\n\n\n");
	for (int i=0; i<result->nbInstr; i++){
		printBytecodeInstruction(i, readInt(result->instructions, i*16+0), readInt(result->instructions, i*16+4), readInt(result->instructions, i*16+8), readInt(result->instructions, i*16+12));
	}

	fprintf(stderr, "Analyzed that jump is %d and condition is %d\n", indexOfJump, indexOfCondition);

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

		//The new instruction will write into an allocated register (if we are in a case with escape)
		if (isEscape)
			setAlloc(result->instructions, sizeofEntryBlock + oneInstr, 1);

		//We update dependencies
		addOffsetToDep(result->instructions, sizeofEntryBlock+oneInstr, sizeofEntryBlock);

		//For each operand register, there are two possibilities:
		// -> The operand is lower than 256, then it is a reference to another instruction from the block: we correct it
		// -> The operand is greater than 256 then it is an access to global register and thus we need to check if there is a dep to add
		for (int oneOperand = 0; oneOperand<nbOperand; oneOperand++){
			if (lastWriteReg[operands[oneOperand]] < sizeofEntryBlock){
				if (operands[oneOperand] < 256){
					fprintf(stderr, "Changed operand %d to %d\n", operands[oneOperand], operands[oneOperand] + sizeofEntryBlock);
					operands[oneOperand] += sizeofEntryBlock;
				}
				else if (lastWriteReg[operands[oneOperand]-256] != -1){
					fprintf(stderr, "Changed global operand %d to %d\n", operands[oneOperand], lastWriteReg[operands[oneOperand]-256]);
					addDataDep(result->instructions, lastWriteReg[operands[oneOperand]-256], sizeofEntryBlock+oneInstr);
					operands[oneOperand] = lastWriteReg[operands[oneOperand]-256];
				}
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

		if (shiftedOpcode == (VEX_STD>>3)){
			hasStores = 1;

			//If we are in a escapable block, we need to change the store into a conditional store
			if (isEscape)
				setOpcode(result->instructions, sizeofEntryBlock+oneInstr, opcode + 4);

			//In any case, we have to ensure a dependency from last mem instruction from previous block (this is only needed for first memory store)
			for (int oneLastMemInstr = 0; oneLastMemInstr<nbLastMemInstr; oneLastMemInstr++)
				addControlDep(result->instructions, lastMemInstr[oneLastMemInstr], sizeofEntryBlock+oneInstr);

			nbLastMemInstr = 0;
		}

		if (nbLastMemInstr > 0 && shiftedOpcode == (VEX_LDD>>3)){
			//For the first 4 load instr we add a dependency to ensure the correctness
			addControlDep(result->instructions, lastMemInstr[placeLastMemInstr], sizeofEntryBlock+oneInstr);
			nbLastMemInstr--;
			placeLastMemInstr = (placeLastMemInstr-1) & 0x3;
		}

		/***************************************************************
		 * We handle cond instructions from the second block:
		 *  -> Stores are turned into conditional store if needed
		 *  -> Dependencies to ensure memory coherence are added to the first memory accesses met
		 */

		if (nbLastCondInstr > 0 && (opcode == VEX_SETCOND || opcode == VEX_SETCONDF)){
			fprintf(stderr, "Found a setcond. There is %d last cond : %d %d %d %d \n", nbLastCondInstr, lastCondInstr[0], lastCondInstr[1], lastCondInstr[2], lastCondInstr[3]);
			for (int onePreviousCond = 0; onePreviousCond<nbLastCondInstr; onePreviousCond++){
				fprintf(stderr, "Adding %d \n", lastCondInstr[placeLastCondInstr]);
				placeLastCondInstr = (placeLastCondInstr-1) & 0x3;
				addControlDep(result->instructions, lastCondInstr[placeLastCondInstr], sizeofEntryBlock+oneInstr);
			}
			nbLastCondInstr = 0;
		}

		short writtenReg = getDestinationRegister(secondBlock->instructions, oneInstr);
		if (writtenReg >= 0)
			lastWriteRegForSecond[writtenReg-256] = oneInstr+sizeofEntryBlock;

		//We check if there is a jump in the second block
		if (opcode == VEX_GOTO || opcode == VEX_GOTOR || opcode == VEX_BRF || opcode == VEX_BR || opcode == VEX_CALL || opcode == VEX_CALLR){
			indexOfSecondJump = oneInstr+sizeofEntryBlock;

		}
	}
	result->nbInstr = entryBlock->nbInstr + secondBlock->nbInstr;


	//We insert a SETCOND depending on the value of the condition
	//This instruction will depend from all previous cond instrucion (if any)
	if (isEscape){

		if (hasStores){
			char indexOfSETCOND = result->nbInstr;
			uint128 bytecodeInstr = assembleIBytecodeInstruction(0, 0, VEX_SETCONDF, indexOfCondition, 0, 0);
			result->instructions[(result->nbInstr)*4+0] = bytecodeInstr.slc<32>(96);
			result->instructions[(result->nbInstr)*4+1] = bytecodeInstr.slc<32>(64);
			result->instructions[(result->nbInstr)*4+2] = bytecodeInstr.slc<32>(32);
			result->instructions[(result->nbInstr)*4+3] = bytecodeInstr.slc<32>(0);

			addDataDep(result->instructions, indexOfCondition, result->nbInstr);
			fprintf(stderr, "While adding dep: there are %d pred %d %d %d %d", nbLastCondInstr, lastCondInstr[0], lastCondInstr[1], lastCondInstr[2], lastCondInstr[3]);
			for (int onePreviousCond = 0; onePreviousCond<nbLastCondInstr; onePreviousCond++)
				addControlDep(result->instructions, lastCondInstr[onePreviousCond], result->nbInstr);

			//In order to simplify future insertions, we write the current isntr as last cond
			nbLastCondInstr = 0;
			lastCondInstr[0] = result->nbInstr;
			lastCondInstr[1] = result->nbInstr;
			lastCondInstr[2] = result->nbInstr;
			lastCondInstr[3] = result->nbInstr;

			result->nbInstr++;
		}

		nbLastCondInstr = 0;


		for (int oneReg = 1; oneReg < 64; oneReg++){
			if (lastWriteRegForSecond[oneReg]>=0){

				fprintf(stderr, "Adding cond for %d\n", oneReg);

				uint128 bytecodeInstr = assembleRBytecodeInstruction(2, 0, VEX_SETc, indexOfCondition, lastWriteRegForSecond[oneReg], oneReg+256, 0);
				result->instructions[(result->nbInstr)*4+0] = bytecodeInstr.slc<32>(96);
				result->instructions[(result->nbInstr)*4+1] = bytecodeInstr.slc<32>(64);
				result->instructions[(result->nbInstr)*4+2] = bytecodeInstr.slc<32>(32);
				result->instructions[(result->nbInstr)*4+3] = bytecodeInstr.slc<32>(0);
				addDataDep(result->instructions, lastWriteRegForSecond[oneReg], result->nbInstr);
				if (lastWriteReg[oneReg]>= 0)
					addControlDep(result->instructions, lastWriteReg[oneReg], result->nbInstr);

				//We add a control dependency from one of the last four cond instruction (note: this array is initialized with four times the setcond instruction)
				if (nbLastCondInstr<4){
					//We add a control dependencies to the jump in the second block if any
					if (indexOfSecondJump >= 0)
						addControlDep(result->instructions, result->nbInstr, indexOfSecondJump);

					addDataDep(result->instructions, indexOfCondition, result->nbInstr);
					nbLastCondInstr++;
					lastCondInstr[placeLastCondInstr] = result->nbInstr;
					placeLastCondInstr = (placeLastCondInstr+1) & 0x3;
				}
				else{
					addControlDep(result->instructions, lastCondInstr[placeLastCondInstr], result->nbInstr);
					lastCondInstr[placeLastCondInstr] = result->nbInstr;
					placeLastCondInstr = (placeLastCondInstr + 1) & 0x3;
				}

				lastWriteRegForSecond[oneReg] = result->nbInstr;

				result->nbInstr++;
				fprintf(stderr, "test %d\n", result->nbInstr);
			}
		}
		fprintf(stderr, "testa %d\n", result->nbInstr);

		//If there is a jump in the second block, we add dependencies with the setcond
		if (indexOfSecondJump != -1){
			for (int oneLastSet = 0; oneLastSet < nbLastCondInstr; oneLastSet++){
				placeLastCondInstr = (placeLastCondInstr - 1) & 0x3;

				fprintf(stderr, "Addign control dep between %d and %d\n", lastCondInstr[placeLastCondInstr], indexOfSecondJump);


				addControlDep(result->instructions, lastCondInstr[placeLastCondInstr], indexOfSecondJump);
			}
			nbLastCondInstr = 0;
		}
		fprintf(stderr, "testb %d\n", result->nbInstr);

		//We correct the jump register if any
		char jumpopcode = getOpcode(result->instructions, indexOfSecondJump);
		if (jumpopcode == VEX_BR || jumpopcode == VEX_BRF){
			short operands[2];
			short nbOperand = getOperands(result->instructions, indexOfSecondJump, operands);
			char physicalDest = getDestinationRegister(result->instructions, operands[0]);
			operands[0] = lastWriteRegForSecond[physicalDest];
			setOperands(result->instructions, indexOfSecondJump, operands);
			addDataDep(result->instructions, operands[0], indexOfSecondJump);
		}

	}

	if (indexOfSecondJump != -1){
		result->instructions[result->nbInstr*4+0] = result->instructions[indexOfSecondJump*4+0];
		result->instructions[result->nbInstr*4+1] = result->instructions[indexOfSecondJump*4+1];
		result->instructions[result->nbInstr*4+2] = result->instructions[indexOfSecondJump*4+2];
		result->instructions[result->nbInstr*4+3] = result->instructions[indexOfSecondJump*4+3];

		result->instructions[indexOfSecondJump*4+0] = 0;
		result->instructions[indexOfSecondJump*4+1] = 0;
		result->instructions[indexOfSecondJump*4+2] = 0;
		result->instructions[indexOfSecondJump*4+3] = 0;

		result->nbInstr++;

	}

	//If the successor of the two blocks are the same, we can remove the br
	if (entryBlock == secondBlock){
			result->jumpID = indexOfSecondJump;
			result->instructions[indexOfJump*4+0] = 0;
	}
	else if (isEscape && entryBlock->successor1 == secondBlock->successor1){
		result->instructions[indexOfJump*4+0] = 0;
	}
	else if (!isEscape && indexOfJump != -1){
		result->instructions[indexOfJump*4+0] = 0;
	}

	else {
		result->jumpID = indexOfJump;
	}
	fprintf(stderr, "test %d %d\n", result->nbInstr, indexOfJump);

	fprintf(stderr, "\n\n\n\n\n");
	for (int i=0; i<result->nbInstr; i++){
		printBytecodeInstruction(i, readInt(result->instructions, i*16+0), readInt(result->instructions, i*16+4), readInt(result->instructions, i*16+8), readInt(result->instructions, i*16+12));
	}

	result->vliwStartAddress = entryBlock->vliwStartAddress;
	result->vliwEndAddress = secondBlock->vliwEndAddress;
	result->blockState = entryBlock->blockState;

	return result;


}

void buildTraces(DBTPlateform *platform, IRProcedure *procedure){

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

	int nbBlock = procedure->nbBlock;

	char changeMade = 1;
	while (changeMade){
		changeMade = 0;
		for (int oneBlock=0; oneBlock<procedure->nbBlock; oneBlock++){
			IRBlock *block = procedure->blocks[oneBlock];
			if (block->nbSucc >= 1){

				IRBlock *secondBlock = (block->nbSucc == 1) ? block->successor1 : block->successor2;


				char isElligible = 1;
				if (procedure->entryBlock == secondBlock)
					isElligible = 0;
				else
					for (int oneOtherBlock = 0; oneOtherBlock<procedure->nbBlock; oneOtherBlock++){
						IRBlock *otherBlock = procedure->blocks[oneOtherBlock];
						if (otherBlock != block && ((otherBlock->nbSucc >= 1 && otherBlock->successor1 == secondBlock) || (otherBlock->nbSucc >= 2 && otherBlock->successor2 == secondBlock))){
							isElligible = 0;
							break;
						}
					}

				if (block->blockState < IRBLOCK_UNROLLED && block->nbSucc == 2 && block->successor1 == block){
					block->blockState = IRBLOCK_UNROLLED;
					fprintf(stderr, "UNROLLING !!!\n");
					IRBlock *oneSuperBlock = superBlock(block, block->successor1);

					block->nbInstr = oneSuperBlock->nbInstr;

					uint32 *oldInstruction = block->instructions;
					block->instructions = oneSuperBlock->instructions;
					oneSuperBlock->instructions = oldInstruction;

					block->jumpID = oneSuperBlock->jumpID;

					delete oneSuperBlock;


				}
				else if (isElligible && secondBlock->nbSucc == 1 && secondBlock->nbInstr<8){
					IRBlock *oneSuperBlock = superBlock(block, secondBlock);

					if (block->successor1 == secondBlock->successor1 || block->nbSucc == 1){
						oneSuperBlock->nbSucc = 1;
						oneSuperBlock->successor1 = secondBlock->successor1;
					}
					else{
						oneSuperBlock->nbSucc = 2;
						oneSuperBlock->successor1 = block->successor1;
						oneSuperBlock->successor2 = secondBlock->successor1;
					}
					secondBlock->nbSucc = 0;
					secondBlock->nbInstr = 0;
					nbBlock--;


					for (int oneOtherBlock = 0; oneOtherBlock<procedure->nbBlock; oneOtherBlock++){
						IRBlock *otherBlock = procedure->blocks[oneOtherBlock];
						if (otherBlock->successor1 == block)
							otherBlock->successor1 = oneSuperBlock;

						if (otherBlock->successor2 == block)
							otherBlock->successor2 = oneSuperBlock;

						procedure->blocks[oneBlock] = oneSuperBlock;

					}

					if (procedure->entryBlock == block)
						procedure->entryBlock = oneSuperBlock;

//					delete block;
//					continue;
					changeMade=1;
					break;
				}
			}
		}

	}
	IRBlock **newBlocks = (IRBlock**)  malloc(nbBlock*sizeof(IRBlock*));
	int index = 0;
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		if (procedure->blocks[oneBlock]->nbInstr != 0){
			newBlocks[index] = procedure->blocks[oneBlock];
			if (procedure->entryBlock == procedure->blocks[oneBlock])
				procedure->entryBlock = newBlocks[index];
			index++;
		}
	}
	procedure->blocks = newBlocks;
	procedure->nbBlock = nbBlock;
	procedure->print();
}

