/*
 * rescheduleProcedure.cpp
 *
 *  Created on: 30 mai 2017
 *      Author: Simon Rokicki
 */

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <transformation/irScheduler.h>
#include <lib/endianness.h>

/******************************************************************************************
 ******************************  Reschedule Procedure
 ******************************************************************************************
 * This transformation will take as argument a procedure and will schedule and place all
 * blocks. It will also modify the branch parameters so that they target the new block locations
 * and will insert some jump in the old code to link toward the new location of binaries.
 *
 * Note that this transformation MODIFY the vliw code memory but DO NOT NEED THE VLIW TO BE STOPPED.
 * Indeed, the code modification is done at a new location and the linkage is only done at the end
 * of the transformation. At any time, if the vliw is executing the code modified, it should not affect
 * the normal execution.
 *
 * Arguments are :
 * 	-> platform is the DBTPlatform containing all global memories
 * 	-> procedure is the procedure that we want to schedule
 * 	-> writePlace is the destination when we will generate the new binaries
 *
 * 	Transformation returns the sum of writePlace and the size of the generated binaries.
 ******************************************************************************************/

int rescheduleProcedure(DBTPlateform *platform, IRProcedure *procedure,int writePlace){

	int originalWritePlace = writePlace;
	int originalEntry = procedure->entryBlock->vliwStartAddress;
	int *oldBlockStarts = (int*) malloc(procedure->nbBlock * sizeof(int));


	/******************************************************************************************
	 ******************************  Scheduling of all blocks
	 ******************************************************************************************
	 * For each block of the procedure, we will do the instruction scheduling and keep track of their
	 * previous start address as well as the place where the jump instruction (if any) as been scheduled.
	 ******************************************************************************************/

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];

		//We save the old block start address
		oldBlockStarts[oneBlock] = block->vliwStartAddress;

		//We move instructions into bytecode memory
		for (int oneBytecodeInstr = 0; oneBytecodeInstr<block->nbInstr; oneBytecodeInstr++){
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, block->instructions[4*oneBytecodeInstr + 0]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, block->instructions[4*oneBytecodeInstr + 1]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, block->instructions[4*oneBytecodeInstr + 2]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, block->instructions[4*oneBytecodeInstr + 3]);
		}

		//We initialize other memories
		for (int oneFreeRegister = 33; oneFreeRegister<63; oneFreeRegister++)
			platform->freeRegisters[oneFreeRegister-33] = oneFreeRegister;

		for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
			platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;

		//This is only for debug
		for (int i=0; i<block->nbInstr; i++){
			printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12));
		}

		//We call the register
		int binaSize = irScheduler(platform, 1,block->nbInstr, writePlace, 29, 4, 0x001e);
		if (block->jumpID == -1)
			binaSize++;

		block->vliwEndAddress = writePlace + binaSize;
		block->vliwStartAddress = writePlace;

		if (block->jumpID != -1){
			int addressOfScheduledJump = platform->placeOfInstr[block->jumpID];
			fprintf(stderr, "Place of jump is %d (%d)\n", addressOfScheduledJump, block->jumpID);
			block->jumpPlace = addressOfScheduledJump;
		}

		writePlace+=binaSize;
	}

	/******************************************************************************************
	 ******************************  Jump correction
	 ******************************************************************************************
	 * Now that all blocks have been scheduled correctly, we can go through all jump instruction and
	 * modify their destination to fit with the new block position.
	 ******************************************************************************************/

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];

		if (block->nbSucc>1){
			//Conditional block (br)
			int offset = 4*(block->successor1->vliwStartAddress - block->jumpPlace);
			fprintf(stderr, "Correcting a br jump to %d at %d. Offset is %d\n", block->successor1->vliwStartAddress,block->jumpPlace, offset);
			unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlace);
			writeInt(platform->vliwBinaries, 16*block->jumpPlace, (oldJump & 0xfc00007f) | ((offset & 0x7ffff) << 7));

			fprintf(stderr, "Changind %x to %x\n", oldJump, readInt(platform->vliwBinaries, 16*(block->vliwEndAddress-2)));

		}
		else if (block->jumpID != -1 && block->nbSucc == 1){
			int dest = 4*block->successor1->vliwStartAddress;
			unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlace);
			writeInt(platform->vliwBinaries, 16*block->jumpPlace, (oldJump & 0xfc00007f) | ((dest & 0x7ffff) << 7));
		}
	}

	/******************************************************************************************
	 ******************************  Link to the newly generated code
	 ******************************************************************************************
	 * For each basic block in the procedure, we will insert jump instructions at the previous start
	 * address targeting to the new start address. This way, the execution will simply switch toward the newly
	 * generated binaries.
	 ******************************************************************************************/

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];
		int originalEntry = oldBlockStarts[oneBlock];
		fprintf(stderr, "previous entry was %d, inserting a jump to %d\n", originalEntry,block->vliwStartAddress);
		writeInt(platform->vliwBinaries, 16*originalEntry+0, assembleIInstruction(VEX_GOTO, block->vliwStartAddress*4, 0));
		writeInt(platform->vliwBinaries, 16*originalEntry+4, 0);
		writeInt(platform->vliwBinaries, 16*originalEntry+8, 0);
		writeInt(platform->vliwBinaries, 16*originalEntry+12, 0);
		writeInt(platform->vliwBinaries, 16*originalEntry+16, 0);
		writeInt(platform->vliwBinaries, 16*originalEntry+20, 0);
		writeInt(platform->vliwBinaries, 16*originalEntry+24, 0);
		writeInt(platform->vliwBinaries, 16*originalEntry+28, 0);

	}

	//This is only for debug
	for (int i=originalWritePlace;i<writePlace;i++){
		fprintf(stderr, "%d ", i);
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(96)); fprintf(stderr, "\n");

	}

	return writePlace;

}

