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

void optimizeBasicBlock(IRBlock *block, DBTPlateform *platform, IRApplication *application, uint32 placeCode){

	/*********************************************************************************
	 * Function optimizeBasicBlock
	 * ********************************************************************************
	 *
	 * This function will perform basic optimization on the specified basic block.
	 * It will use the irBuilder to build the IR and then use the irScheduler to export it into
	 * vliw binaries.
	 *
	 *********************************************************************************/
	char incrementInBinaries = (platform->vliwInitialIssueWidth>4) ? 2 : 1;
	int basicBlockStart = block->vliwStartAddress;
	int basicBlockEnd = block->vliwEndAddress;

	if (platform->debugLevel > 0)
		fprintf(stderr, "Block from %x to %x is eligible for scheduling\n", block->sourceStartAddress, block->sourceEndAddress);


#ifndef __NIOS

	//TODO make it work for nios too
	char isCurrentlyInBlock = (platform->vexSimulator->PC >= basicBlockStart*4) &&
			(platform->vexSimulator->PC < basicBlockEnd*4);

	if (isCurrentlyInBlock){
		if (platform->debugLevel > 1)
			fprintf(stderr, "Currently inside block, inserting stop...\n");
		writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0x2f);

		platform->vexSimulator->doStep(1000);
		writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0);


	}
#endif

	//We store old jump instruction. Its places is known from the basicBlockEnd value
	uint32 jumpInstruction = readInt(platform->vliwBinaries, (basicBlockEnd-2*incrementInBinaries)*16 + 0);


	int globalVariableCounter = 288;

	for (int oneGlobalVariable = 0; oneGlobalVariable < 64; oneGlobalVariable++)
		platform->globalVariables[oneGlobalVariable] = 256 + oneGlobalVariable;

	int originalScheduleSize = basicBlockEnd - basicBlockStart - 1;


	int blockSize = irGenerator(platform, basicBlockStart, originalScheduleSize, globalVariableCounter);

	//We store the result in an array cause it can be used later
	block->instructions = (uint32*) malloc(blockSize*4*sizeof(uint32));
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<blockSize; oneBytecodeInstr++){
		block->instructions[4*oneBytecodeInstr + 0] = readInt(platform->bytecode, 16*oneBytecodeInstr + 0);
		block->instructions[4*oneBytecodeInstr + 1] = readInt(platform->bytecode, 16*oneBytecodeInstr + 4);
		block->instructions[4*oneBytecodeInstr + 2] = readInt(platform->bytecode, 16*oneBytecodeInstr + 8);
		block->instructions[4*oneBytecodeInstr + 3] = readInt(platform->bytecode, 16*oneBytecodeInstr + 12);
	}

	block->nbInstr = blockSize;
	char opcodeOfLastInstr = getOpcode(block->instructions, blockSize-1);
	if ((opcodeOfLastInstr >> 4) == 2 && opcodeOfLastInstr != VEX_MOVI && opcodeOfLastInstr != VEX_SETCOND && opcodeOfLastInstr != VEX_SETCONDF){
		block->jumpID = blockSize-1;
	}
#ifndef __NIOS

	if (platform->debugLevel > 1){

		fprintf(stderr, "*************************************************************************\n");
		fprintf(stderr, "Previous version of sources:\n");
		fprintf(stderr, "*****************\n");


		for (int i=basicBlockStart-10;i<basicBlockEnd+10;i++){
			fprintf(stderr, "%d ", i);
			std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
			std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
			std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
			std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(96)); fprintf(stderr, " ");

			if (platform->vliwInitialIssueWidth>4){
				std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(0)); fprintf(stderr, " ");
				std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(32)); fprintf(stderr, " ");
				std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(64)); fprintf(stderr, " ");
				std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(96)); fprintf(stderr, " ");
				i++;
			}
			fprintf(stderr, "\n");

		}

		for (int i=basicBlockStart;i<basicBlockEnd;i++){
			fprintf(stderr, "schedule;%d\n",i);
		}

		fprintf(stderr, "*************************************************************************\n");
		fprintf(stderr, "Bytecode is: \n");
		fprintf(stderr, "\n*****************\n");
		for (int i=0; i<blockSize; i++){
			printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12));
		}

		for (int i=0; i<blockSize; i++){
			fprintf(stderr, "0x%x, 0x%x, 0x%x, 0x%x,\n",readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12));
		}
	}
#endif







	//Preparation of required memories
	for (int oneFreeRegister = 33; oneFreeRegister<63; oneFreeRegister++)
		platform->freeRegisters[oneFreeRegister-33] = oneFreeRegister;


	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;


	//Calling scheduler
	int binaSize = irScheduler(platform, 1,blockSize, placeCode, 29, platform->vliwInitialIssueWidth, platform->vliwInitialConfiguration);
	binaSize = binaSize & 0xffff;

	if (binaSize < originalScheduleSize){

		memcpy(&platform->vliwBinaries[basicBlockStart], &platform->vliwBinaries[placeCode], (binaSize+1)*sizeof(uint128));

		for (int i=basicBlockStart+binaSize;i<basicBlockEnd;i++){
			platform->vliwBinaries[i] = 0;
		}




		/*****************************************************************
		 *	Control Flow Correction
		 ************
		 * In this part we make the necessary work to make jumps correct.
		 * There are three different tasks:
		 * 	-> If the jump was relative jump then we have to correct the offset
		 * 	-> We have to write the (corrected) jump instruction because current scheduler just compute the place but do not place the instruction
		 * 	-> If the jump is a passthrough (eg. if the execution can go in the part after the schedule) we need to add a goto instruction
		 * 		to prevent the execution of a large area of nop instruction (which would remove the interest of the schedule
		 *
		 *****************************************************************/

		char isRelativeJump = (jumpInstruction & 0x7f) == VEX_BR || (jumpInstruction & 0x7f) == VEX_BRF;
		char isNoJump = (jumpInstruction & 0x70) != (VEX_CALL>>4);
		char isPassthroughJump = isRelativeJump || (jumpInstruction & 0x7f) == VEX_CALL || (jumpInstruction & 0x7f) == VEX_CALLR ;

		//Ofset correction
		if (isRelativeJump){

			//We read the offset and correct its sign if needed
			int offset = (jumpInstruction >> 7) & 0x7ffff;
			if ((offset & 0x40000) != 0)
				offset = offset - 0x80000;

			//We compute the original destination
			int destination = basicBlockEnd - 1*incrementInBinaries + (offset>>2);

			//We compute the new offset, considering the new destination
			int newOffset = destination - (basicBlockStart + binaSize-1*incrementInBinaries);
			newOffset = newOffset << 2;

			if (platform->debugLevel > 1)
				fprintf(stderr, "Correction of jump at the end of the block. Original offset was %d\n From it derivated destination %d and new offset %d\n", offset, destination, newOffset);
			jumpInstruction = (jumpInstruction & 0xfc00007f) | ((newOffset & 0x7ffff) << 7);
		}
//		else if (isNoJump){
//			jumpInstruction = assembleIInstruction(VEX_GOTO, basicBlockEnd<<2, 0);
//		}

		//Insertion of jump instruction
		writeInt(platform->vliwBinaries, (basicBlockStart+binaSize-2*incrementInBinaries)*16 + 0, jumpInstruction);

		//Insertion of the new block with the goto instruction
		if (isPassthroughJump && basicBlockStart+binaSize+1 < basicBlockEnd){
			//We need to add a jump to correct the shortening of the block.

			uint32 insertedJump = VEX_GOTO + (basicBlockEnd<<9); // Note added the *4 to handle the new PC encoding
			writeInt(platform->vliwBinaries, (basicBlockStart+binaSize)*16, insertedJump);

			//In this case, we also added a block in the design
			//We need to insert it in the set of blocks
			IRBlock* newBlock = new IRBlock(basicBlockStart + binaSize, basicBlockStart + binaSize + 2, block->section);
			newBlock->sourceStartAddress = -1;
			newBlock->sourceEndAddress = -1;
			application->addBlock(newBlock, block->section);

			if (platform->debugLevel > 1)
				fprintf(stderr, "adding a block from %d tp %d\n", basicBlockStart + binaSize, basicBlockStart + binaSize + 2);
		}

		/*****************************************************************/


		#ifndef __NIOS

		if (platform->debugLevel > 1){

			fprintf(stderr, "*************************************************************************\n");
			for (int i=basicBlockStart-10;i<basicBlockEnd+10;i++){
				fprintf(stderr, "%d ", i);
				std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(0)); fprintf(stderr, " ");
				std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(32)); fprintf(stderr, " ");
				std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(64)); fprintf(stderr, " ");
				std::cerr << printDecodedInstr(platform->vliwBinaries[i].slc<32>(96)); fprintf(stderr, " ");

				if (platform->vliwInitialIssueWidth>4){
					std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(0)); fprintf(stderr, " ");
					std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(32)); fprintf(stderr, " ");
					std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(64)); fprintf(stderr, " ");
					std::cerr << printDecodedInstr(platform->vliwBinaries[i+1].slc<32>(96)); fprintf(stderr, " ");
					i++;
				}
				fprintf(stderr, "\n");
			}

			for (int i=basicBlockStart;i<basicBlockEnd;i++){
				fprintf(stderr, "schedule;%d\n",i);
			}

			fprintf(stderr, "*************************************************************************\n");

			fprintf(stderr, "*************************************************************************\n");
		}
		#endif

		//We modify the stored information concerning the block
		block->vliwEndAddress = basicBlockStart + binaSize;
	}
	else{
		if (platform->debugLevel > 0)
			fprintf(stderr, "Schedule is dropped (%d cycles)\n", binaSize);

	}

	block->blockState = IRBLOCK_STATE_SCHEDULED;

}



