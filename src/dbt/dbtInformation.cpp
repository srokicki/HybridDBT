/*
 * dbtInformation.cpp
 *
 *  Created on: 14 janv. 2019
 *      Author: simon
 */




#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>
#include <dbt/profiling.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/endianness.h>
#include <simulator/vexSimulator.h>
#include <simulator/vexTraceSimulator.h>
#include <simulator/loadQueueVexSimulator.h>
#include <simulator/riscvSimulator.h>

#include <transformation/irGenerator.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/buildControlFlow.h>
#include <transformation/reconfigureVLIW.h>
#include <transformation/buildTraces.h>
#include <transformation/rescheduleProcedure.h>
#include <transformation/memoryDisambiguation.h>

#include <lib/debugFunctions.h>

#include <isa/vexISA.h>
#include <isa/riscvISA.h>

#include <lib/config.h>
#include <lib/log.h>
#include <lib/traceQueue.h>
#include <lib/threadedDebug.h>
#include <transformation/firstPassTranslation.h>

#include <dbt/dbtInformation.h>


#ifndef __NIOS
#include <lib/elfFile.h>
#else
#include <system.h>
#endif

#include <isa/irISA.h>

extern "C"


typedef struct BlockInformation {
	IRBlock *block;
	int scheduleSizeOpt0 = -1;
	int scheduleSizeOpt1 = -1;
	int scheduleSizeOpt2 = -1;
};


BlockInformation *blockInfo;


DBTPlateform *platform;
IRApplication *application;
unsigned int  placeCode;

int translateOneSection(DBTPlateform &dbtPlateform, unsigned int placeCode, int sourceStartAddress, int sectionStartAddress, int sectionEndAddress){
	int previousPlaceCode = placeCode;
	unsigned int size = (sectionEndAddress - sectionStartAddress)>>2;
	placeCode = firstPassTranslator(&dbtPlateform,
			size,
			sourceStartAddress,
			sectionStartAddress,
			placeCode);



		return placeCode;
}


void readSourceBinaries(char* path, unsigned char *&code, unsigned int &addressStart, unsigned int &size, unsigned int &pcStart, DBTPlateform *platform){

	//We open the elf file and search for the section that is of interest for us
	ElfFile elfFile(path);

	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
		ElfSection *section = elfFile.sectionTable->at(sectionCounter);

		//The section we are looking for is called .text
		if (!section->getName().compare(".text")){

			code = section->getSectionCode();//0x3c
			addressStart = section->address + 0;
			size = section->size/4 - 0;

			if (size > MEMORY_SIZE){
				free(platform->vliwBinaries);
				free(platform->mipsBinaries);
				free(platform->blockBoundaries);
				platform->vliwBinaries = (unsigned int*) malloc(4*size*2*sizeof(unsigned int));
				platform->mipsBinaries = (unsigned int*) malloc(4*size*2*sizeof(unsigned int));
				platform->blockBoundaries = (unsigned char*) malloc(size*2*sizeof(unsigned char));
			}
		}
	}

}

int globalBinarySize;

void initializeDBTInfo(char* fileName)
{


	int CONFIGURATION = 2;
	int VERBOSE = 0;
	Log::Init(VERBOSE, 0);


	/***********************************
	 *  Initialization of the DBT platform
	 ***********************************
	 * In the linux implementation, this is done by reading an elf file and copying binary instructions
	 * in the corresponding memory.
	 * In a real platform-> this may require no memory initialization as the binaries would already be stored in the
	 * system memory.
	 ************************************/

	//Definition of objects used for DBT process
	platform = new DBTPlateform(MEMORY_SIZE);

	unsigned char* code;
	unsigned int addressStart;
	unsigned int size;
	unsigned int pcStart;

	readSourceBinaries(fileName, code, addressStart, size, pcStart, platform);


	platform->vliwInitialConfiguration = CONFIGURATION;
	platform->vliwInitialIssueWidth = getIssueWidth(platform->vliwInitialConfiguration);

	//Preparation of required memories
	for (int oneFreeRegister = 33; oneFreeRegister<63; oneFreeRegister++)
		platform->freeRegisters[oneFreeRegister-33] = oneFreeRegister;

	for (int oneFreeRegister = 63-33; oneFreeRegister<63; oneFreeRegister++)
		platform->freeRegisters[oneFreeRegister] = 0;

	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;
	//same for FP registers
	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+64+onePlaceOfRegister] = onePlaceOfRegister;

  	platform->vexSimulator = new LoadQueueVexSimulator(platform->vliwBinaries, platform->specInfo);
	setVLIWConfiguration(platform->vexSimulator, platform->vliwInitialConfiguration);





	int numberOfSections = 1 + (size>>10);
	application = new IRApplication(numberOfSections);
	application->numberInstructions = size;
	Profiler profiler = Profiler(platform);



	//we copy the binaries in the corresponding memory
	for (int oneInstruction = 0; oneInstruction<size; oneInstruction++)
		platform->mipsBinaries[oneInstruction] = ((unsigned int*) code)[oneInstruction];


	//We declare the variable in charge of keeping a track of where we are writing
	placeCode = 0; //As 4 instruction bundle

	//We add initialization code to the vliw binaries
	placeCode = getInitCode(platform, placeCode, addressStart);
	placeCode = insertCodeForInsertions(platform, placeCode, addressStart);

	initializeInsertionsMemory(size*4);


	for (int i=0; i<placeCode; i++){
		platform->vexSimulator->typeInstr[i] = 3;
	}
	int endOfInitSection = placeCode;

	for (int oneSection=0; oneSection<(size>>10)+1; oneSection++){

		int startAddressSource = addressStart + oneSection*1024*4;
		int endAddressSource = startAddressSource + 1024*4;
		if (endAddressSource > addressStart + size*4)
			endAddressSource = addressStart + (size<<2);


		int effectiveSize = (endAddressSource - startAddressSource)>>2;
		for (int j = 0; j<effectiveSize; j++){
			platform->mipsBinaries[j] = ((unsigned int*) code)[j+oneSection*1024];
		}
		int oldPlaceCode = placeCode;

		placeCode = translateOneSection(*platform, placeCode, addressStart, startAddressSource, endAddressSource);

		buildBasicControlFlow(platform, oneSection, addressStart, startAddressSource, oldPlaceCode, placeCode, application, &profiler);

	}

	for (int oneUnresolvedJump = 0; oneUnresolvedJump<unresolvedJumpsArray[0]; oneUnresolvedJump++){
		unsigned int source = unresolvedJumpsSourceArray[oneUnresolvedJump+1];
		unsigned int initialDestination = unresolvedJumpsArray[oneUnresolvedJump+1];
		unsigned int type = unresolvedJumpsTypeArray[oneUnresolvedJump+1];

		unsigned char isAbsolute = ((type & 0x7f) != VEX_BR) && ((type & 0x7f) != VEX_BRF && (type & 0x7f) != VEX_BLTU) && ((type & 0x7f) != VEX_BGE && (type & 0x7f) != VEX_BGEU) && ((type & 0x7f) != VEX_BLT);
		unsigned int destinationInVLIWFromNewMethod = solveUnresolvedJump(platform, initialDestination);

		if (destinationInVLIWFromNewMethod == -1){
			Log::printf(LOG_ERROR, "A jump from %d to %x is still unresolved... (%d insertions)\n", source, initialDestination, insertionsArray[(initialDestination>>10)<<11]);
			exit(-1);
		}
		else{
			int immediateValue = (isAbsolute) ? (destinationInVLIWFromNewMethod) : ((destinationInVLIWFromNewMethod  - source));
			int mask = (isAbsolute) ? 0x7ffff : 0x1fff;

			writeInt(platform->vliwBinaries, 16*(source), type + ((immediateValue & mask)<<7));

			if (immediateValue > 0x7ffff){
				Log::fprintf(LOG_ERROR, stderr, "error in immediate size...\n");
				exit(-1);
			}
			unsigned int instructionBeforePreviousDestination = readInt(platform->vliwBinaries, 16*(destinationInVLIWFromNewMethod-1)+12);
			if (instructionBeforePreviousDestination != 0)
				writeInt(platform->vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
		}
	}

	for (int i=endOfInitSection; i<placeCode; i++){
		platform->vexSimulator->typeInstr[i] = 0;
	}


	/********************** We store the size of each block ***********************/
	globalBinarySize = 10*size;
	blockInfo = (BlockInformation *) malloc(10*size*sizeof(BlockInformation));
	for (int oneBlockInfo = 0; oneBlockInfo<10*size; oneBlockInfo++)
		blockInfo[oneBlockInfo].block = NULL;

	int numberOfBlocks = 0;
	for (int oneSection = 0; oneSection<numberOfSections; oneSection++){
		for (int oneBlock = 0; oneBlock<application->numbersBlockInSections[oneSection]; oneBlock++){
			IRBlock* block = application->blocksInSections[oneSection][oneBlock];
			blockInfo[block->sourceStartAddress].block = block;
			blockInfo[block->sourceStartAddress].scheduleSizeOpt0 = block->vliwEndAddress - block->vliwStartAddress;
			blockInfo[block->sourceStartAddress].scheduleSizeOpt1 = -1;
			blockInfo[block->sourceStartAddress].scheduleSizeOpt2 = -1;
			numberOfBlocks++;
		}
	}





	/********************** We compute all schedules ******************************/

	//We perform aggressive level 1 optimization: if a block takes more than 8 cycle we schedule it.
//	//If it has a backward loop, we also profile it.
//	for (int oneSection = 0; oneSection<numberOfSections; oneSection++){
//		for (int oneBlock = 0; oneBlock<application->numbersBlockInSections[oneSection]; oneBlock++){
//			IRBlock* block = application->blocksInSections[oneSection][oneBlock];
//
//
//			if (block != NULL && block->sourceStartAddress != -1){
//				if (OPTLEVEL >= 1 && block->sourceEndAddress - block->sourceStartAddress>8){
//					optimizeBasicBlock(block, &platform-> &application, placeCode);
//					platform->blockScheduleCounter++;
//fprintf(stderr, "test %d\n",platform->blockScheduleCounter);
//				}
//			}
//		}
//	}
//	fprintf(stderr, "Scheduled %d blocks\n", platform->blockScheduleCounter);

}

int getBlockSize(int address, int optLevel, int timeFromSwitch, int *nextBlock){
//	fprintf(stderr, "Address is %x, block is %llx\n", address, blockInfo[address>>2].block);

	if ((address>>2) >= globalBinarySize){
//		fprintf(stderr, "Acessing out of bounds -- %x - %x\n", address>>2, globalBinarySize);
		return 0;
	}

	if (blockInfo[address>>2].block == NULL)
		return 0;

	*nextBlock = blockInfo[address>>2].block->sourceEndAddress*4;

//	fprintf(stderr, "Next is %x\n", *nextBlock);

	if (optLevel == 1){
		if (blockInfo[address>>2].scheduleSizeOpt1 == -1){
			optimizeBasicBlock(blockInfo[address>>2].block, platform, application, placeCode);
			blockInfo[address>>2].scheduleSizeOpt1 = blockInfo[address>>2].block->vliwEndAddress - blockInfo[address>>2].block->vliwStartAddress;
		}
		return blockInfo[address>>2].scheduleSizeOpt1;
	}
	else if (optLevel >= 2){

		if (blockInfo[address>>2].scheduleSizeOpt2 == -1){

			if (blockInfo[address>>2].block->blockState >= IRBLOCK_PROC){
					blockInfo[address>>2].scheduleSizeOpt1 = blockInfo[address>>2].block->vliwEndAddress - blockInfo[address>>2].block->vliwStartAddress;
					blockInfo[address>>2].scheduleSizeOpt2 = blockInfo[address>>2].scheduleSizeOpt1;
			}
			else {
				int errorCode = buildAdvancedControlFlow(platform, blockInfo[address>>2].block, application);
				blockInfo[address>>2].block->blockState = IRBLOCK_PROC;

				if (!errorCode){
					buildTraces(platform, application->procedures[application->numberProcedures-1], 100);
					placeCode = rescheduleProcedure(platform, application->procedures[application->numberProcedures-1], placeCode);
					for (int oneBlock = 0; oneBlock<application->procedures[application->numberProcedures-1]->nbBlock; oneBlock++){
						IRBlock* block = application->procedures[application->numberProcedures-1]->blocks[oneBlock];
						IRBlock* blockInOtherList = blockInfo[address>>2].block;



						if (block == blockInOtherList){
							blockInfo[address>>2].scheduleSizeOpt2 = block->vliwEndAddress - block->vliwStartAddress;

							if (block->blockState = IRBLOCK_UNROLLED && block->unrollingFactor != 0)
								blockInfo[address>>2].scheduleSizeOpt2 = blockInfo[address>>2].scheduleSizeOpt2 / block->unrollingFactor;

							int currentjumpId = 0;
							for (int oneSourceCycle = block->sourceStartAddress+1; oneSourceCycle<block->sourceEndAddress; oneSourceCycle++){
								if (blockInfo[oneSourceCycle].block != NULL){
									if (currentjumpId >= block->nbJumps){
//										fprintf(stderr, "Block info : number jump : %d number succ : %d\n", block->nbJumps, block->nbSucc);
//										fprintf(stderr, "Block limits : %x to %x\n", block->sourceStartAddress, block->sourceEndAddress);
//										fprintf(stderr, "Current cycle : %x\n", oneSourceCycle);
//										fprintf(stderr, "Failed while trying to extrapolate block size from traces (DBTInfo line 332)\n");
										blockInfo[oneSourceCycle].scheduleSizeOpt2 = 0;
										//										exit(-1);
									}
									else{
										blockInfo[oneSourceCycle].scheduleSizeOpt2 = block->vliwEndAddress - block->jumpPlaces[currentjumpId];
									}
									currentjumpId++;
								}
							}
						}


					}
				}
			}
		}

		return blockInfo[address>>2].scheduleSizeOpt2;
	}

	return blockInfo[address>>2].scheduleSizeOpt0;
}

char useIndirectionTable(int address){
	//Address is the RISCV address of the jump
	//We find the nearest block above
	int currentAddress = address;
	while (blockInfo[currentAddress>>2].block == NULL){
		currentAddress -= 4;
	}

	if (blockInfo[currentAddress>>2].block->blockState > IRBLOCK_STATE_SCHEDULED)
		return 0;
}



int main(int argc, char** argv){
	initializeDBTInfo(argv[1]);
}

