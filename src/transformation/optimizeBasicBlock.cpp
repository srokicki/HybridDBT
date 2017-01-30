/*
 * optimizeBasicBlock.cpp
 *
 *  Created on: 16 nov. 2016
 *      Author: Simon Rokicki
 */

#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>
#include <isa/vexISA.h>
#include <isa/irISA.h>

#include <transformation/irScheduler.h>
#include <transformation/irGenerator.h>
#include <types.h>

void optimizeBasicBlock(unsigned int basicBlockStart, unsigned int basicBlockEnd, DBTPlateform *platform){

	/*********************************************************************************
	 * Function optimizeBasicBlock
	 * ********************************************************************************
	 *
	 * This function will perform basic optimization on the specified basic block.
	 * It will use the irBuilder to build the IR and then use the irScheduler to export it into
	 * vliw binaries.
	 *
	 *********************************************************************************/

	//We store old jump instruction. Its places is known from the basicBlockEnd value
	uint32 jumpInstruction = readInt(platform->vliwBinaries, (basicBlockEnd-2)*16 + 0);


	int globalVariableCounter = 288;
	unsigned long long registersUsage[256];

	int32 globalVariables[64] = {256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,
			279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,
			307,308,309,310,311,312,313,314,315,316,317,318,319
	};

	int blockSize = basicBlockEnd - basicBlockStart - 1;

	uint64 local_registersUsage[1];

	blockSize = irGenerator_hw(platform->vliwBinaries,basicBlockStart, blockSize, platform->bytecode, globalVariables, local_registersUsage, globalVariableCounter);

	fprintf(stderr, "*************************************************************************\n");
	fprintf(stderr, "Optimizing a block of size %d : \n", blockSize);
	fprintf(stderr, "\n*****************\n");
	for (int i=0; i<blockSize; i++){
		printBytecodeInstruction(i,platform->bytecode[i]);
	}



	ac_int<6, 0> placeOfRegisters[512] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
			40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
	ac_int<6, 0> freeRegisters[64] = {36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
	ac_int<32, false> placeOfInstr[256];

	int binaSize = scheduling(1,blockSize, platform->bytecode, platform->vliwBinaries,basicBlockStart, placeOfRegisters, 27, freeRegisters, 4, 0x001e, placeOfInstr);
	binaSize = binaSize & 0xffff;

	for (int i=basicBlockStart+binaSize+1;i<basicBlockEnd;i++){
		platform->vliwBinaries[i] = 0;
	}


	fprintf(stderr, "Block is scheduled in %d cycles\n", binaSize);


	//If the jump is relative we need to correct it because its place changed...
	if ((jumpInstruction & 0x7f) == VEX_BR || (jumpInstruction & 0x7f) == VEX_BRF){

		//We read the offset and correct its sign if needed
		int offset = (jumpInstruction >> 7) & 0x7ffff;
		if (offset & 0x40000 != 0)
			offset = offset - 0x80000;

		//We compute the original destination
		int destination = basicBlockEnd - 1 + offset;

		//We compute the new offset, considering the new destination
		int newOffset = destination - (basicBlockStart + binaSize + 1);

		fprintf(stderr, "Correction of jump at the end of the block. Original offset was %d\n From it derivated destination %d and new offset %d\n", offset, destination, newOffset);
		uint32 newInstruction = jumpInstruction & 0xfc00007f | ((newOffset & 0x7ffff) << 7);
		fprintf(stderr, "Old jump instr was %x. New is %x\n", jumpInstruction, newInstruction);
		writeInt(platform->vliwBinaries, (basicBlockStart+binaSize)*16 + 0, newInstruction);
	}

	if (basicBlockStart+binaSize < basicBlockEnd){
		//We need to add a jump to correct the shortening of the block.

		uint32 insertedJump = VEX_GOTO + (basicBlockEnd<<7);
		writeInt(platform->vliwBinaries, (basicBlockStart+binaSize+2)*16, insertedJump);


	}



	fprintf(stderr, "*************************************************************************\n");
	for (int i=basicBlockStart;i<basicBlockEnd;i++){
		fprintf(stderr, "0x%xl, 0x%xl, 0x%xl, 0x%xl,\n", (int) platform->vliwBinaries[i].slc<32>(0),
				(int) platform->vliwBinaries[i].slc<32>(32),
				(int) platform->vliwBinaries[i].slc<32>(64),
				(int) platform->vliwBinaries[i].slc<32>(96));
	}
	for (int i=basicBlockStart;i<basicBlockEnd;i++){
		fprintf(stderr, "schedule;%d;%d\n",i);
	}

	fprintf(stderr, "*************************************************************************\n");

	fprintf(stderr, "*************************************************************************\n");

}



