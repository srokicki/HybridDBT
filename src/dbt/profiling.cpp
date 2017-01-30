/*
 * profiling.cpp
 *
 *  Created on: 26 janv. 2017
 *      Author: simon
 */

#include <lib/endianness.h>
#include <isa/vexISA.h>
#include <types.h>
#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>

int destinationCallProfiling;
int numberProfiledBlocks = 0;
IRBlock* profiledBlocks[1024];

unsigned int insertCodeForProfiling(ac_int<128, false> *binaries, int start, unsigned int startAddress){

	//This procedure will add code necessary for profiling through function call.
	//Using this way of profiling, we only need to insert a movi and a add in the BB in
	//to make profiling effective.
	//This method has a cost: it will take 6 cycles which can be expensive if in critical loop


	int cycle = start;
	destinationCallProfiling = start;
	//		| r35 = 0x800	 | 					| 				|
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_MOVI, 0x800, 35));
	writeInt(binaries, cycle*16+4, 0);
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	//		| r35 = r35 << 16	 | r34 = r34 << 2		| 				|
	cycle++;
	writeInt(binaries, cycle*16+0, assembleRiInstruction(VEX_SLLi, 35, 35, 16));
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_SLLi, 34, 34, 2));
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	//		| 				 | r35 = ldw 0(r34) 	| 					|
	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 35, 34, 0));
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	//		| return	 | r35++		|					|
	cycle++;
	writeInt(binaries, cycle*16+0, assembleIInstruction(VEX_GOTOR, 0, 8));
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_ADDi, 35, 35, 1));
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);

	//		|				 | r35 = ldw 0(r34) 	|					| offset = offset<<24
	cycle++;
	writeInt(binaries, cycle*16+0, 0);
	writeInt(binaries, cycle*16+4, assembleRiInstruction(VEX_STW, 35, 34, 0));
	writeInt(binaries, cycle*16+8, 0);
	writeInt(binaries, cycle*16+12, 0);


	cycle++;
	return cycle;
}

void profileBlock(IRBlock *oneBlock, DBTPlateform &platform){
	int start = oneBlock->vliwStartAddress;


	int placeMOVI=-1, placeSLLI=-1, placeLD=-1, placeINCR=-1, placeSTW=-1, offMOVI, offSLLI, offINCR;

	for (int oneInstruction = start; oneInstruction<oneBlock->vliwEndAddress; oneInstruction++){
		ac_int<128, false> oneVLIWInstruction = platform.vliwBinaries[oneInstruction];
		if (placeINCR != -1){
			//We now place STW
			if (oneVLIWInstruction.slc<32>(64) == 0){
				//We found a place
				ac_int<32, false> instr = assembleRiInstruction(VEX_STW, 35, 34, numberProfiledBlocks<<2);
				oneVLIWInstruction.set_slc(64, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeSTW = oneInstruction;
				break;
			}

		}
		else if (placeLD != -1){
			//We place INCR
			if (oneVLIWInstruction.slc<32>(96) == 0){
				//We found a place
				ac_int<32, false> instr = assembleRiInstruction(VEX_ADDi, 35, 35, 1);
				oneVLIWInstruction.set_slc(96, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeINCR = oneInstruction;
			}
			else {
				//We found a place
				ac_int<32, false> instr = assembleRiInstruction(VEX_ADDi, 35, 35, 1);
				oneVLIWInstruction.set_slc(64, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeINCR = oneInstruction;
			}
		}
		else if (placeSLLI != -1){
			//We place LDW
			if (oneVLIWInstruction.slc<32>(64) == 0){
				//We found a place
				ac_int<32, false> instr = assembleRiInstruction(VEX_LDW, 35, 34, numberProfiledBlocks<<2);
				oneVLIWInstruction.set_slc(64, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeLD = oneInstruction;
			}
		}
		else if (placeMOVI != -1){
			//We now place SLLi
			if (oneVLIWInstruction.slc<32>(96) == 0){
				//We found a place
				ac_int<32, false> instr = assembleRiInstruction(VEX_SLLi, 34, 34, 16);
				oneVLIWInstruction.set_slc(96, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeSLLI = oneInstruction;
			}
			else {
				//We found a place
				ac_int<32, false> instr = assembleRiInstruction(VEX_SLLi, 34, 34, 16);
				oneVLIWInstruction.set_slc(64, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeSLLI = oneInstruction;
			}
		}
		else{
			//We now place MOVi
			if (oneVLIWInstruction.slc<32>(96) == 0){
				//We found a place
				ac_int<32, false> instr = assembleIInstruction(VEX_MOVI, 0x800, 34);
				oneVLIWInstruction.set_slc(96, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeMOVI = oneInstruction;
			}
			else {
				//We found a place
				ac_int<32, false> instr = assembleIInstruction(VEX_MOVI, 0x800, 34);
				oneVLIWInstruction.set_slc(64, instr);
				platform.vliwBinaries[oneInstruction] = oneVLIWInstruction;
				placeMOVI = oneInstruction;
			}
		}
	}

	if (placeSTW == -1){
		printf("Failed at inserting profiling, need alternative method\n");
	}
	else{
		profiledBlocks[numberProfiledBlocks] = oneBlock;
		numberProfiledBlocks++;


	}

}

void returnProfilingInformation(DBTPlateform &platform){
	for (int oneBlock = 0; oneBlock<numberProfiledBlocks; oneBlock++){
		IRBlock *block = profiledBlocks[oneBlock];
		printf("Block profiled %d from %d to %d has been called %d times\n",oneBlock, block->vliwStartAddress, block->vliwEndAddress, (int) platform.vexSimulator->ldw(0x8000000 + oneBlock*4));
	}
}
