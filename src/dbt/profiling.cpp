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
#include <lib/log.h>



unsigned int Profiler::insertProfilingProcedure(int start, unsigned int startAddress){

	//This procedure will add code necessary for profiling through function call.
	//Using this way of profiling, we only need to insert a movi and a add in the BB in
	//to make profiling effective.
	//This method has a cost: it will take 6 cycles which can be expensive if in critical loop

	int cycle = start;
	this->destinationCallProfiling = start;
	//		| r35 = 0x800	 | 					| 				|
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction_sw(VEX_MOVI, 0x800, 35));
	writeInt(platform->vliwBinaries, cycle*16+4, 0);
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| r35 = r35 << 16	 | r34 = r34 << 2		| 				|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleRiInstruction_sw(VEX_SLLi, 35, 35, 16));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction_sw(VEX_SLLi, 34, 34, 2));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| 				 | r35 = ldw 0(r34) 	| 					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleMemoryInstruction_sw(VEX_LDW, 35, 34, 0, false, 0));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		| return	 | r35++		|					|
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, assembleIInstruction_sw(VEX_GOTOR, 0, 8));
	writeInt(platform->vliwBinaries, cycle*16+4, assembleRiInstruction_sw(VEX_ADDi, 35, 35, 1));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);

	//		|				 | r35 = ldw 0(r34) 	|					| offset = offset<<24
	cycle++;
	writeInt(platform->vliwBinaries, cycle*16+0, 0);
	writeInt(platform->vliwBinaries, cycle*16+4, assembleMemoryInstruction_sw(VEX_STW, 35, 34, 0, false, 0));
	writeInt(platform->vliwBinaries, cycle*16+8, 0);
	writeInt(platform->vliwBinaries, cycle*16+12, 0);


	cycle++;
	return cycle;
}

void Profiler::profileBlock(IRBlock *oneBlock){
	if (numberProfiledBlocks < 512){

		int start = oneBlock->vliwStartAddress;
		char successfullInsertion = 0;
		char incrementInBinaries = (getIssueWidth(this->platform->vliwInitialConfiguration) > 4) ? 2 : 1;


		for (unsigned int oneInstruction = start; oneInstruction<oneBlock->vliwEndAddress; oneInstruction+=incrementInBinaries){
			unsigned int instr64 = readInt(this->platform->vliwBinaries, oneInstruction*16+4);

			//We now place the profile instr
			if (instr64 == 0){
				//We found a place
				unsigned int instr = assembleMemoryInstruction_sw(VEX_PROFILE,0,0, numberProfiledBlocks, false, 0);
				writeInt(this->platform->vliwBinaries, oneInstruction*16+4, instr);
				successfullInsertion = 1;
				break;
			}


		}

		if (!successfullInsertion){
			fprintf(stderr, "Faile dprofiling %d\n", oneBlock->sourceStartAddress);

			Log::logWarning << "Failed at inserting profiling, need alternative method\n";
		}
		else{
			this->platform->vexSimulator->profileResult[numberProfiledBlocks] = 0;
			profiledBlocks[numberProfiledBlocks] = oneBlock;
			oneBlock->placeInProfiler = numberProfiledBlocks;
			numberProfiledBlocks++;
		}

	}

}

void Profiler::unprofileBlock(int ID){
	profiledBlocks[ID] = NULL;
}


int Profiler::getNumberProfiledBlocks(){
	return numberProfiledBlocks;
}

int Profiler::getProfilingInformation(int ID){
	#ifndef __NIOS

	if (profiledBlocks[ID] != NULL){
		return this->platform->vexSimulator->profileResult[ID];
	}

	return -1;
	//return this->platform->vexSimulator->ldw(0x8000000 + ID*4);
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
