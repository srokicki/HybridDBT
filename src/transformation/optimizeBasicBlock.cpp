/*
 * optimizeBasicBlock.cpp
 *
 *  Created on: 16 nov. 2016
 *      Author: Simon Rokicki
 */

#include <stdlib.h>
#include <stdio.h>

#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>
#include <isa/irISA.h>
#include <simulator/vexSimulator.h>
#include <transformation/irScheduler.h>
#include <transformation/irGenerator.h>
#include <types.h>

void optimizeBasicBlock(IRBlock *block, DBTPlateform *platform, IRApplication *application){

	/*********************************************************************************
	 * Function optimizeBasicBlock
	 * ********************************************************************************
	 *
	 * This function will perform basic optimization on the specified basic block.
	 * It will use the irBuilder to build the IR and then use the irScheduler to export it into
	 * vliw binaries.
	 *
	 *********************************************************************************/

	int basicBlockStart = block->vliwStartAddress;
	int basicBlockEnd = block->vliwEndAddress;

#ifndef __NIOS

	//TODO make it work for nios too
	char isCurrentlyInBlock = (platform->vexSimulator->PC >= basicBlockStart*4) &&
			(platform->vexSimulator->PC < basicBlockEnd*4);

	if (isCurrentlyInBlock){
		fprintf(stderr, "Currently inside block, inserting stop...\n");
		writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0x2f);

		platform->vexSimulator->doStep(1000);
		writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0);


	}
#endif

	//We store old jump instruction. Its places is known from the basicBlockEnd value
	uint32 jumpInstruction = readInt(platform->vliwBinaries, (basicBlockEnd-2)*16 + 0);


	int globalVariableCounter = 288;

	for (int oneGlobalVariable = 0; oneGlobalVariable < 64; oneGlobalVariable++)
		platform->globalVariables[oneGlobalVariable] = 256 + oneGlobalVariable;

	int blockSize = basicBlockEnd - basicBlockStart - 1;

	blockSize = irGenerator(platform, basicBlockStart, blockSize, globalVariableCounter);

	//We store the result in an array cause it can be used later
	block->instructions = (uint32*) malloc(blockSize*4*sizeof(uint32));
	memcpy(block->instructions, platform->bytecode, blockSize*sizeof(uint32)); //TODO this is not correct...
	block->nbInstr = blockSize;

//	fprintf(stderr, "*************************************************************************\n");
//	fprintf(stderr, "Optimizing a block of size %d : \n", blockSize);
//	fprintf(stderr, "\n*****************\n");
//	for (int i=0; i<blockSize; i++){
//		printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12));
//	}

	//Preparation of required memories
	for (int oneFreeRegister = 36; oneFreeRegister<63; oneFreeRegister++)
		platform->freeRegisters[oneFreeRegister-36] = oneFreeRegister;

	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;


	int binaSize = irScheduler(platform, 1,blockSize, basicBlockStart, 27, 4, 0x001e);
	binaSize = binaSize & 0xffff;

	for (int i=basicBlockStart+binaSize+1;i<basicBlockEnd;i++){
		platform->vliwBinaries[i] = 0;
	}


	fprintf(stderr, "Block is scheduled in %d cycles\n", binaSize);


	//If the jump is relative we need to correct it because its place changed...
	if ((jumpInstruction & 0x7f) == VEX_BR || (jumpInstruction & 0x7f) == VEX_BRF){

		//We read the offset and correct its sign if needed
		int offset = (jumpInstruction >> 7) & 0x7ffff;
		if ((offset & 0x40000) != 0)
			offset = offset - 0x80000;

		//We compute the original destination
		int destination = basicBlockEnd - 1 + (offset>>2);

		//We compute the new offset, considering the new destination
		int newOffset = destination - (basicBlockStart + binaSize + 1);
		newOffset = newOffset << 2;

		fprintf(stderr, "Correction of jump at the end of the block. Original offset was %d\n From it derivated destination %d and new offset %d\n", offset, destination, newOffset);
		uint32 newInstruction = (jumpInstruction & 0xfc00007f) | ((newOffset & 0x7ffff) << 7);
		fprintf(stderr, "Old jump instr was %x. New is %x\n", jumpInstruction, newInstruction);
		writeInt(platform->vliwBinaries, (basicBlockStart+binaSize)*16 + 0, newInstruction);
	}
	printf("%d + %d <> %d\n", basicBlockStart, binaSize, basicBlockEnd);
	if (basicBlockStart+binaSize+2 < basicBlockEnd){
		//We need to add a jump to correct the shortening of the block.

		uint32 insertedJump = VEX_GOTO + (basicBlockEnd<<9); // Note added the *4 to handle the new PC encoding
		writeInt(platform->vliwBinaries, (basicBlockStart+binaSize+2)*16, insertedJump);

		//In this case, we also added a block in the design
		//We need to insert it in the set of blocks
		IRBlock* newBlock = new IRBlock(basicBlockStart + binaSize + 2, basicBlockStart + binaSize + 4, block->section);
		application->addBlock(newBlock, block->section);
		fprintf(stderr, "adding a block from %d tp %d\n", basicBlockStart + binaSize + 2, basicBlockStart + binaSize + 4);
	}



	#ifndef __NIOS
	fprintf(stderr, "*************************************************************************\n");
	for (int i=basicBlockStart-10;i<basicBlockEnd+10;i++){
		fprintf(stderr, "%d ", i);
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
		std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(96)); fprintf(stderr, "\n");

	}
	#endif

	for (int i=basicBlockStart;i<basicBlockEnd;i++){
		fprintf(stderr, "schedule;%d;%d\n",i);
	}

	fprintf(stderr, "*************************************************************************\n");

	fprintf(stderr, "*************************************************************************\n");

	//We modify the stored information concerning the block
	block->vliwEndAddress = basicBlockStart + binaSize + 2;
	block->blockState = IRBLOCK_STATE_SCHEDULED;

}



