/*
 * rescheduleProcedure.cpp
 *
 *  Created on: 30 mai 2017
 *      Author: simon
 */

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <transformation/irScheduler.h>
#include <lib/endianness.h>

int rescheduleProcedure(DBTPlateform *platform, IRProcedure *procedure,int writePlace){

	int originalWritePlace = writePlace;
	int originalEntry = procedure->entryBlock->vliwStartAddress;

	//We do all the schedules
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];

		//We move instructions into bytecode memory
		for (int oneBytecodeInstr = 0; oneBytecodeInstr<block->nbInstr; oneBytecodeInstr++){
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, block->instructions[4*oneBytecodeInstr + 0]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, block->instructions[4*oneBytecodeInstr + 1]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, block->instructions[4*oneBytecodeInstr + 2]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, block->instructions[4*oneBytecodeInstr + 3]);
		}

		//We initialize other memories
		for (int oneFreeRegister = 34; oneFreeRegister<63; oneFreeRegister++)
			platform->freeRegisters[oneFreeRegister-35] = oneFreeRegister;

		for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
			platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;

		for (int i=0; i<block->nbInstr; i++){
			printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12));
		}

		//We call the register
		int binaSize = irScheduler(platform, 1,block->nbInstr, writePlace, 29, 4, 0x001e);
		block->vliwStartAddress = writePlace + binaSize + 1;
		block->vliwStartAddress = writePlace;

		if (block->jumpID != -1){
			int addressOfScheduledJump = platform->placeOfInstr[block->jumpID];
			fprintf(stderr, "Place of jump is %d (%d)\n", addressOfScheduledJump, block->jumpID);
			block->jumpPlace = addressOfScheduledJump;
		}

		writePlace+=binaSize;
	}

	//We correct jumps

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];

		if (block->nbSucc>1){
			//Conditional block (br)
			int offset = 4*(block->successor1->vliwStartAddress - block->jumpPlace);
			fprintf(stderr, "Correcting a br jump to %d. Offset is %d\n", block->successor1->vliwStartAddress, offset);
			unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlace);
			writeInt(platform->vliwBinaries, 16*block->jumpPlace, (oldJump & 0xfc00007f) | ((offset & 0x7ffff) << 7));
		}
		else if (block->jumpID != -1 && block->nbSucc == 1){
			int dest = 4*block->successor1->vliwStartAddress;
			unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlace);
			writeInt(platform->vliwBinaries, 16*block->jumpPlace, (oldJump & 0xfc00007f) | ((dest & 0x7ffff) << 7));
		}
	}

	fprintf(stderr, "previous entry was %d, inserting a jump to %d\n", originalEntry,procedure->entryBlock->vliwStartAddress);
	writeInt(platform->vliwBinaries, 16*originalEntry+0, assembleIInstruction(VEX_GOTO, procedure->entryBlock->vliwStartAddress*4, 0));
	writeInt(platform->vliwBinaries, 16*originalEntry+4, 0);
	writeInt(platform->vliwBinaries, 16*originalEntry+8, 0);
	writeInt(platform->vliwBinaries, 16*originalEntry+12, 0);
	writeInt(platform->vliwBinaries, 16*originalEntry+16, 0);
	writeInt(platform->vliwBinaries, 16*originalEntry+20, 0);
	writeInt(platform->vliwBinaries, 16*originalEntry+24, 0);
	writeInt(platform->vliwBinaries, 16*originalEntry+28, 0);

	for (int i=originalWritePlace;i<writePlace;i++){
		fprintf(stderr, "%d ", i);
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(96)); fprintf(stderr, "\n");

	}

}

