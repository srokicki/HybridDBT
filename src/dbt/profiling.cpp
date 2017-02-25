/*
 * profiling.cpp
 *
 *  Created on: 26 janv. 2017
 *      Author: simon
 */

#include <stdio.h>

#include <lib/endianness.h>
#include <isa/vexISA.h>
#include <types.h>
#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>

#include <dbt/profiling.h>



unsigned int Profiler::insertProfilingProcedure(int start, unsigned int startAddress){

	//This procedure will add code necessary for profiling through function call.
	//Using this way of profiling, we only need to insert a movi and a add in the BB in
	//to make profiling effective.
	//This method has a cost: it will take 6 cycles which can be expensive if in critical loop

	int cycle = start;
	this->destinationCallProfiling = start;
	//		| r35 = 0x800	 | 					| 				|
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_MOVI, 0x800, 35));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| r35 = r35 << 16	 | r34 = r34 << 2		| 				|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction(VEX_SLLi, 35, 35, 16));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_SLLi, 34, 34, 2));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| 				 | r35 = ldw 0(r34) 	| 					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_LDW, 35, 34, 0));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| return	 | r35++		|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction(VEX_GOTOR, 0, 8));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_ADDi, 35, 35, 1));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		|				 | r35 = ldw 0(r34) 	|					| offset = offset<<24
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction(VEX_STW, 35, 34, 0));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);


	cycle++;
	return cycle;
}

void Profiler::profileBlock(IRBlock *oneBlock){
	int start = oneBlock->vliwStartAddress;


	int placeMOVI=-1, placeSLLI=-1, placeLD=-1, placeINCR=-1, placeSTW=-1, offMOVI, offSLLI, offINCR;

	for (int oneInstruction = start; oneInstruction<oneBlock->vliwEndAddress; oneInstruction++){
		uint32 instr64 = readInt(this->platform->vliwBinaries, oneInstruction*16+4);
		uint32 instr96 = readInt(this->platform->vliwBinaries, oneInstruction*16+0);

		if (placeINCR != -1){
			//We now place STW
			if (instr64 == 0){
				//We found a place
				uint32 instr = assembleRiInstruction(VEX_STW, 35, 34, numberProfiledBlocks<<2);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+4, instr);
				placeSTW = oneInstruction;
				break;
			}

		}
		else if (placeLD != -1){
			//We place INCR
			if (instr96 == 0){
				//We found a place
				uint32 instr = assembleRiInstruction(VEX_ADDi, 35, 35, 1);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+0, instr);
				placeINCR = oneInstruction;
			}
			else {
				//We found a place
				uint32 instr = assembleRiInstruction(VEX_ADDi, 35, 35, 1);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+4, instr);
				placeINCR = oneInstruction;
			}
		}
		else if (placeSLLI != -1){
			//We place LDW
			if (instr64 == 0){
				//We found a place
				uint32 instr = assembleRiInstruction(VEX_LDW, 35, 34, numberProfiledBlocks<<2);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+4, instr);
				placeLD = oneInstruction;
			}
		}
		else if (placeMOVI != -1){
			//We now place SLLi
			if (instr96 == 0){
				//We found a place
				uint32 instr = assembleRiInstruction(VEX_SLLi, 34, 34, 16);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+0, instr);
				placeSLLI = oneInstruction;
			}
			else {
				//We found a place
				uint32 instr = assembleRiInstruction(VEX_SLLi, 34, 34, 16);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+4, instr);
				placeSLLI = oneInstruction;
			}
		}
		else{
			//We now place MOVi
			if (instr96 == 0){
				//We found a place
				uint32 instr = assembleIInstruction(VEX_MOVI, 0x800, 34);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+0, instr);
				placeMOVI = oneInstruction;
			}
			else {
				//We found a place
				uint32 instr = assembleIInstruction(VEX_MOVI, 0x800, 34);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+4, instr);
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
		fprintf(stderr, "profiling block %x\n", (long int) oneBlock);


	}

	//We mark the block as being profiled
	oneBlock->blockState = IRBLOCK_STATE_PROFILED;

}

int Profiler::getNumberProfiledBlocks(){
	return numberProfiledBlocks;
}

int Profiler::getProfilingInformation(int ID){
	#ifndef __NIOS
	return this->platform->vexSimulator->ldw(0x8000000 + ID*4);
	#else
	return this->platform->vliwDataMemory[0];
	//TODO change this to place profiling information at a correct place
	#endif

}

IRBlock* Profiler::getBlock(int ID){
	return profiledBlocks[ID];
}

Profiler::Profiler(DBTPlateform *platform){
	this->platform = platform;
	this->numberProfiledBlocks = 0;
}

