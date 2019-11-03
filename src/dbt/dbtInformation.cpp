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
#include <simulator/emptySimulator.h>

#include <transformation/irGenerator.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/buildControlFlow.h>
#include <transformation/reconfigureVLIW.h>
#include <transformation/buildTraces.h>
#include <transformation/rescheduleProcedure.h>
#include <transformation/memoryDisambiguation.h>

#include <isa/vexISA.h>
#include <isa/riscvISA.h>

#include <lib/config.h>
#include <lib/log.h>

#include <transformation/firstPassTranslation.h>

#include <dbt/dbtInformation.h>
#include <vector>

#ifndef __NIOS
#include <lib/elfFile.h>
#else
#include <system.h>
#endif

#include <isa/irISA.h>
#include <assert.h>
extern "C"


/****************************************************************************************************************************/

#define IT_NB_SET 8
#define IT_NB_WAY 8

#define COST_OPT_1 10
#define COST_OPT_2 100

#define SIZE_TC 0

/****************************************************************************************************************************/

typedef struct BlockInformation {
	IRBlock *block;
	int scheduleSizeOpt0 = -1;
	int scheduleSizeOpt1 = -1;
	int scheduleSizeOpt2 = -1;
} BlockInformation;


struct entryInTranslationCache {
	IRBlock *block;
	IRProcedure *procedure;
	bool isBlock;
	unsigned int size;
	unsigned int cost;
};


struct indirectionTableEntry {
	uint64_t address;
	uint8_t counter;
	bool isInTC;
	char optLevel;
	unsigned int sizeInTC; //Reprensent the size of the binaries in the TC (counter as the number of instruction)
	uint64_t timeAvailable; //Reprensent the timestamp where the optimization is finished. If current cycle number is lower we take the optimization level just below
	unsigned int costOfBinaries; // Cost function that represent the time spent on the binaries.
};

/****************************************************************************************************************************/

BlockInformation *blockInfo;


DBTPlateform *platform, *nonOptPlatform;
IRApplication *application;
unsigned int  placeCode;

struct indirectionTableEntry indirectionTable[IT_NB_WAY][IT_NB_SET];
uint64_t nextAvailabilityDBTProc = 0;
int sizeLeftInTC = 8192;

std::vector<struct entryInTranslationCache> *translationCacheContent = NULL;
int globalBinarySize;

/****************************************************************************************************************************/
//Definition of internal function that are not visible from outside

bool allocateInTranslationCache(int size, IRProcedure *procedure, IRBlock *block);

/****************************************************************************************************************************/


int translateOneSection(DBTPlateform &dbtPlateform, unsigned int placeCode, int sourceStartAddress, int sectionStartAddress, int sectionEndAddress){
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


int contentTranslationCache(){
	int size = 0;
	for (int oneElement = 0; oneElement < translationCacheContent->size(); oneElement++){
		struct entryInTranslationCache entry = translationCacheContent->at(oneElement);
		size += entry.size;
	}

	return size;
}

void updateOpt2BlockSize(IRBlock *block){
	blockInfo[block->sourceStartAddress].scheduleSizeOpt2 = block->vliwEndAddress - block->vliwStartAddress;


	if (block->blockState == IRBLOCK_UNROLLED && block->unrollingFactor != 0)
		blockInfo[block->sourceStartAddress].scheduleSizeOpt2 = blockInfo[block->sourceStartAddress].scheduleSizeOpt2 / block->unrollingFactor;

	int currentjumpId = 0;

	int nbBlockAdded = 0;
	if (block->blockState == IRBLOCK_TRACE)
		for (int oneSourceCycle = block->sourceStartAddress+1; oneSourceCycle<block->sourceEndAddress; oneSourceCycle++){
			if (blockInfo[oneSourceCycle].block != NULL){
				if (currentjumpId >= block->nbJumps){
					blockInfo[oneSourceCycle].scheduleSizeOpt2 = (block->vliwEndAddress - block->vliwStartAddress) / 2;
				}
				else{
					blockInfo[oneSourceCycle].scheduleSizeOpt2 = block->vliwEndAddress - block->jumpPlaces[currentjumpId];
				}
				currentjumpId++;
				nbBlockAdded++;
			}
		}

	if (nbBlockAdded != block->nbMergedBlocks){
		printf("Added %d block infor while there are %d merged blocks\n", nbBlockAdded, block->nbMergedBlocks);
	}
}

IRProcedure* optimizeLevel2(unsigned int address){

	//We check if the block has already been optimized
	if (blockInfo[address>>2].scheduleSizeOpt2 == -1){

		if (blockInfo[address>>2].block->blockState >= IRBLOCK_PROC){
				blockInfo[address>>2].scheduleSizeOpt1 = blockInfo[address>>2].block->vliwEndAddress - blockInfo[address>>2].block->vliwStartAddress;
				blockInfo[address>>2].scheduleSizeOpt2 = blockInfo[address>>2].scheduleSizeOpt1;
				return NULL;
		}
		else {
			int errorCode = buildAdvancedControlFlow(platform, blockInfo[address>>2].block, application);
			blockInfo[address>>2].block->blockState = IRBLOCK_PROC;

			if (!errorCode){
				buildTraces(platform, application->procedures[application->numberProcedures-1], 100);
				placeCode = rescheduleProcedure(platform, application->procedures[application->numberProcedures-1], placeCode);
				updateSpeculationsStatus(platform, placeCode);

				for (int oneBlock = 0; oneBlock<application->procedures[application->numberProcedures-1]->nbBlock; oneBlock++){
					IRBlock* block = application->procedures[application->numberProcedures-1]->blocks[oneBlock];
					IRBlock* blockInOtherList = blockInfo[block->sourceStartAddress].block;


					if (block != blockInOtherList)
						 blockInfo[block->sourceStartAddress].block = block;

					updateOpt2BlockSize(block);

				}

				return application->procedures[application->numberProcedures-1];
			}
			else{
				printf("Met an error while trying to go to opt level 2 for %x\n", address);
				return NULL;
			}
		}
	}
	else {
		for (int oneProcedure = 0; oneProcedure < application->numberProcedures; oneProcedure++){
			IRProcedure *procedure = application->procedures[oneProcedure];
			for (int oneBlock = 0; oneBlock < procedure->nbBlock; oneBlock++){
				IRBlock *block = procedure->blocks[oneBlock];
				if (block->sourceStartAddress * 4 == address){
					return procedure;
				}
			}
		}
		return NULL;
	}


}


unsigned int addressStart;

void initializeDBTInfo(char* fileName)
{


	int CONFIGURATION = 2;
	int VERBOSE = 3;
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

  	platform->vexSimulator = new EmptySimulator(platform->vliwBinaries, platform->specInfo);
	setVLIWConfiguration(platform->vexSimulator, platform->vliwInitialConfiguration);


	//Setting blocks as impossible to destroy
	IRBlock::isUndestroyable = true;


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
		int destinationInVLIWFromNewMethod = solveUnresolvedJump(platform, initialDestination);

		if (destinationInVLIWFromNewMethod == -1){
			Log::logError << "A jump from " << source << " to " << std::hex << initialDestination << " is still unresolved... (" << insertionsArray[(initialDestination>>10)<<11] << " insertions)\n";
			exit(-1);
		}
		else{
			int immediateValue = (isAbsolute) ? (destinationInVLIWFromNewMethod) : ((destinationInVLIWFromNewMethod  - source));
			int mask = (isAbsolute) ? 0x7ffff : 0x1fff;

			writeInt(platform->vliwBinaries, 16*(source), type + ((immediateValue & mask)<<7));

			if (immediateValue > 0x7ffff){

				Log::logError << "Error in immediate size... Should be corrected in real life\n";
				immediateValue &= 0x7ffff;
			}
			unsigned int instructionBeforePreviousDestination = readInt(platform->vliwBinaries, 16*(destinationInVLIWFromNewMethod-1)+12);
			if (instructionBeforePreviousDestination != 0)
				writeInt(platform->vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
		}
	}


	/********************** We store the size of each block ***********************/
	globalBinarySize = 10*size;
	blockInfo = (BlockInformation *) malloc(10*size*sizeof(BlockInformation));
	for (int oneBlockInfo = 0; oneBlockInfo<10*size; oneBlockInfo++){
		blockInfo[oneBlockInfo].block = NULL;
		blockInfo[oneBlockInfo].scheduleSizeOpt0 = -1;
		blockInfo[oneBlockInfo].scheduleSizeOpt1 = -1;
		blockInfo[oneBlockInfo].scheduleSizeOpt2 = -1;

	}

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


	/************************** We create a copy of the platform which is used to re-optimize ***********************/

	nonOptPlatform = (DBTPlateform*) malloc(sizeof(DBTPlateform));
	memcpy(nonOptPlatform, platform, sizeof(DBTPlateform));

	nonOptPlatform->vliwBinaries = (unsigned int*) malloc(4*MEMORY_SIZE*sizeof(unsigned int));
	memcpy(nonOptPlatform->vliwBinaries, platform->vliwBinaries, 4*MEMORY_SIZE*sizeof(unsigned int));

	fprintf(stderr, "Greatest instr written is %d\n", placeCode);

}

/******************************************************************************************************
 *  Function useIndirectionTable(int address)
 ******************************************************************************************************
 * This function is the first function called by Qemu to verify whether the current branch should be
 * considered like a traced jump or not.
 * If it is, we have to go through all the indirection table, otherwise we stay in the same mode and opt
 * level than before.
 *
 * The function just check the optimization level of the said block : if it is level 2 then the jump
 * are no longer checked
 *
 ******************************************************************************************************/

char useIndirectionTable(int address){
	//Address is the RISCV address of the jump
	//We find the nearest block above
	int currentAddress = address;
	while (blockInfo[currentAddress>>2].block == NULL){
		currentAddress -= 4;
	}

	if (blockInfo[currentAddress>>2].block->blockState > IRBLOCK_STATE_SCHEDULED){
		if (blockInfo[currentAddress>>2].block->nbSucc > 0)
			return 0;
	}

	return 1;
}


/******************************************************************************************************
 *  Function getBlockSize (int address, int optLevel, int timeFromSwitch, int *nextBlock)
 ******************************************************************************************************
 * This function is used to get size of a given block (and thus its execution time). This function is
 * the last one called. Consequently, we know the exact opt level to use for it.
 *
 * The function receive an address and an optimization level and find the corresponding block size.
 * The function also modifies nextBlock in order to provide the address of the next block to execute.
 *
 ******************************************************************************************************/

int getBlockSize(int address, int optLevel, int timeFromSwitch, int *nextBlock){

	if ((address>>2) >= globalBinarySize){
		fprintf(stderr, "too large !!\n");
		return 0;
	}

	if (blockInfo[address>>2].block == NULL){

		int currentAddress = address;
		while (blockInfo[currentAddress>>2].block == NULL){
			currentAddress -= 4;
		}
		if (currentAddress - address < 9)
			address = currentAddress;
		else{
			fprintf(stderr, "Failed at finding block at %x   nearest is in %x\n", address>>2, currentAddress>>2);
			return 0;
		}
	}

	unsigned int end = blockInfo[address>>2].block->sourceEndAddress;
	while (blockInfo[end].block == NULL){
		end++;
	}
	*nextBlock = end*4;

	if (optLevel == 1){ //Opt level 1
		if (blockInfo[address>>2].scheduleSizeOpt1 == -1){
			//We have to schedule the block
			optimizeBasicBlock(blockInfo[address>>2].block, platform, application, placeCode);
			blockInfo[address>>2].scheduleSizeOpt1 = blockInfo[address>>2].block->vliwEndAddress - blockInfo[address>>2].block->vliwStartAddress;
		}

		return blockInfo[address>>2].scheduleSizeOpt1;
	}
	else if (optLevel >= 2){ //Opt level 2

		if (blockInfo[address>>2].scheduleSizeOpt2 == -1){
			if (blockInfo[address>>2].block->blockState > IRBLOCK_STATE_SCHEDULED)
				updateOpt2BlockSize(blockInfo[address>>2].block);
			else {
				if (blockInfo[address>>2].block->nbInstr == 0){
					int start = (address>>2)-1;
					while (blockInfo[start].block==NULL)
						start--;

					printf("Previous start was %d and its original size are %d %d\n", start, blockInfo[start].block->sourceStartAddress,  blockInfo[start].block->sourceEndAddress);

					fprintf(stderr, "the block has zero instructions and a type of %d and type of pred is %d\n", blockInfo[address>>2].block->blockState, blockInfo[start].block->blockState);
				}
				fprintf(stderr, "While asking for size at opt level 2, block is not found (this should never happen ?!)  %d\n", address>>2);
				return 0;
			}
		}

//		if (blockInfo[address>>2].block->blockState == IRBLOCK_UNROLLED)
//			fprintf(stderr, "Executing an unrolled loop -- Cost is %d instead of %d\n", blockInfo[address>>2].scheduleSizeOpt2, blockInfo[address>>2].block->vliwEndAddress-blockInfo[address>>2].block->vliwStartAddress);

		//We just return the size of the block at optimization level 2
		return blockInfo[address>>2].scheduleSizeOpt2;
	}

	if (blockInfo[address>>2].scheduleSizeOpt0 == 0)
		fprintf(stderr, "size 0 is equal to zero for %d\n", address>>2);
	//If we are neither at opt level 1 or 2, we return the size at opt level 0
	return blockInfo[address>>2].scheduleSizeOpt0;
}




/**********************************************************************************
 *  Function getOptLevel(int address, uint64_t nb_cycle)
 ****************************
 * This function is the first called after a jump is simulated in QEMU. The objective
 * is to simulate the indirection table and to decide whether or not the execution goes
 * through the translation cache.
 * This function should go through:
 * 	+ Simulation of the indirection table
 * 	+ Simulation of the translation cache & of the replacement policy
 * 	+ Simulation of the return stack
 *
 **********************************************************************************/


char getOptLevel(int address, uint64_t nb_cycle){
	bool inTranslationCache= false;
	char optLevel = 0;

	/************************************************************************************
	*	Step 1: Simulation of the indirection table: is the branch profiled ? 			*/


	int setNumber = (address>>2) & 0x7;
	bool found = 0;
	for (int oneWay = 0; oneWay < IT_NB_WAY; oneWay++){
		if (indirectionTable[oneWay][setNumber].address == address){
			//The destination of the branch is in the indirection table.


			//We increment the use counter
			indirectionTable[oneWay][setNumber].counter++;
			if (indirectionTable[oneWay][setNumber].timeAvailable < nb_cycle)
				optLevel = indirectionTable[oneWay][setNumber].optLevel;
			else
				optLevel = indirectionTable[oneWay][setNumber].optLevel - 1;


//				fprintf(stderr, "[%d] for %d, %d\n",nb_cycle, (address>>2), optLevel);
			found = true;
		}
	}

	if (!found){
		inTranslationCache = false;
		for (int oneWay = 0; oneWay < IT_NB_WAY; oneWay++){
			if (indirectionTable[oneWay][setNumber].counter == 0){
				indirectionTable[oneWay][setNumber].address = address;
				indirectionTable[oneWay][setNumber].counter = 1;
				indirectionTable[oneWay][setNumber].optLevel = 0;
				indirectionTable[oneWay][setNumber].timeAvailable = nb_cycle;
				break;

			}
			else{
				indirectionTable[oneWay][setNumber].counter--;
			}
		}

	}

	/***********************************************************************************
	 * Triggering the optimization if the DBT proc is available						   */


	for (int oneWay = 0; oneWay < IT_NB_WAY; oneWay++){
		for (int oneSet = 0; oneSet < IT_NB_SET; oneSet++){
			if (nextAvailabilityDBTProc <= nb_cycle){

				unsigned int oneAddress = indirectionTable[oneWay][oneSet].address;

				if (indirectionTable[oneWay][oneSet].counter >= 3 && indirectionTable[oneWay][oneSet].optLevel <= 0){
					//We should trigger opt level 1

					//TODO: measuring how much place there is in the TC
					//TODO: Optionally remove something from the cache

					if (blockInfo[oneAddress>>2].block != NULL){

						int size = blockInfo[oneAddress>>2].block->sourceEndAddress - blockInfo[oneAddress>>2].block->sourceStartAddress;
						bool fitsInTranslationCache = allocateInTranslationCache(size, NULL, blockInfo[oneAddress>>2].block);

						if (fitsInTranslationCache){
							indirectionTable[oneWay][oneSet].optLevel = 1;
							indirectionTable[oneWay][oneSet].isInTC = true;
							indirectionTable[oneWay][oneSet].timeAvailable = nb_cycle + COST_OPT_1 * size;
							indirectionTable[oneWay][oneSet].costOfBinaries = COST_OPT_1 * size;
							indirectionTable[oneWay][oneSet].sizeInTC = size;
							sizeLeftInTC -= size;
							nextAvailabilityDBTProc = nb_cycle + COST_OPT_1 * size;
						}
					}
				}
				else if (indirectionTable[oneWay][oneSet].counter >= 7 && indirectionTable[oneWay][oneSet].optLevel <= 1){
					//We trigger opt level 2

					//TODO: measuring how much place there is in the TC
					//TODO: Optionally remove something from the cache

					if (blockInfo[oneAddress>>2].block != NULL && blockInfo[oneAddress>>2].block->blockState<IRBLOCK_PROC){

						IRProcedure *procedure = optimizeLevel2(oneAddress);

						//We compute the size
						if (procedure != NULL){

							//We measure the sum of all blocks in the procedure
							int size=0;
							for (int oneBlock = 0; oneBlock < procedure->nbBlock; oneBlock++)
								size += procedure->blocks[oneBlock]->nbInstr;

							bool fitsInTranslationCache = allocateInTranslationCache(size, procedure, NULL);

							//We see if it fits the TC
							if (fitsInTranslationCache){
								indirectionTable[oneWay][oneSet].optLevel = 2;
								indirectionTable[oneWay][oneSet].isInTC = true;
								indirectionTable[oneWay][oneSet].timeAvailable = nb_cycle + COST_OPT_2 * size;
								indirectionTable[oneWay][oneSet].costOfBinaries = COST_OPT_2 * size;
								indirectionTable[oneWay][oneSet].sizeInTC = size;
								sizeLeftInTC -= size;
								nextAvailabilityDBTProc = nb_cycle + COST_OPT_2 * size;
							}

						}
					}
				}
			}
		}
	}

	/************************************************************************************
	*	Step 3: Simulation of the call stack: we have to decide whether the return goes *
	*			inside the translation cache or inside the first pass translation		*/

	//TODO

	if (optLevel < 0)
		optLevel = 0;
	return optLevel;
}


bool allocateInTranslationCache(int size, IRProcedure *procedure, IRBlock *block){

	//If first use, we initialize the data structure
	if (translationCacheContent == NULL)
		translationCacheContent = new std::vector<struct entryInTranslationCache>();

	//We create the new element
	struct entryInTranslationCache newEntry;
	newEntry.procedure = procedure;
	newEntry.block = block;
	newEntry.size = size;
	if (procedure != NULL)
		newEntry.isBlock = false;
	else
		newEntry.isBlock = true;


	int currentSize = contentTranslationCache();
	if (SIZE_TC == 0 || currentSize + size < SIZE_TC){
		//We can store the translated element in the translation cache without having to evict anything
		translationCacheContent->push_back(newEntry);
		return true;
	}
	else{
		//We have to evict something
		fprintf(stderr, "Stopped optimizing because replacement policy has not been encoded\n");
		return false;
	}
}


void verifyBranchDestination(int addressOfJump, int dest){

	if (blockInfo[dest>>2].block == NULL){
		//The destination of the jumps is not a block entry point. We have to modify everything.
		fprintf(stderr, "Correcting a wrong block boundary (address is %x)!\n", dest);

		unsigned int correspondingVliwAddress = solveUnresolvedJump(platform, (dest-addressStart)/4);

		//We find the corresponding block
		int currentStart = dest>>2;
		while (blockInfo[currentStart].block == NULL)
			currentStart--;



		IRBlock *containingBlock = blockInfo[currentStart].block;
		assert((dest>>2) < containingBlock->sourceEndAddress);
		assert(correspondingVliwAddress < containingBlock->vliwEndAddress && correspondingVliwAddress > containingBlock->vliwStartAddress);

		int initialState = containingBlock->blockState;



		IRBlock *newBlock = new IRBlock(correspondingVliwAddress, solveUnresolvedJump(platform, containingBlock->sourceEndAddress-addressStart*4), containingBlock->section);
		newBlock->sourceStartAddress = dest >> 2;
		newBlock->sourceEndAddress = containingBlock->sourceEndAddress;
		newBlock->sourceDestination = containingBlock->sourceDestination;
		newBlock->blockState = IRBLOCK_STATE_FIRSTPASS;

		fprintf(stderr, "Adding block in section : section %d has %d allocated and %d blocks\n", containingBlock->section, application->numbersAllocatedBlockInSections[containingBlock->section], application->numbersBlockInSections[containingBlock->section]);
		application->addBlock(newBlock, containingBlock->section);


		fprintf(stderr, "Adding block in section : section %d has %d allocated and %d blocks\n", containingBlock->section, application->numbersAllocatedBlockInSections[containingBlock->section], application->numbersBlockInSections[containingBlock->section]);

		//We add the block in the application
		//application->addBlock(newBlock, containingBlock->section);

		containingBlock->sourceDestination = -1;
		containingBlock->sourceEndAddress = dest >> 2;
		containingBlock->vliwStartAddress = solveUnresolvedJump(platform, containingBlock->sourceStartAddress-addressStart*4);
		containingBlock->vliwEndAddress = solveUnresolvedJump(platform, containingBlock->sourceEndAddress-addressStart*4);
		containingBlock->blockState = IRBLOCK_STATE_FIRSTPASS;

		//We update blockInfo data
		blockInfo[newBlock->sourceStartAddress].block = newBlock;
		blockInfo[newBlock->sourceStartAddress].scheduleSizeOpt0 = newBlock->vliwEndAddress - newBlock->vliwStartAddress;
		blockInfo[containingBlock->sourceStartAddress].scheduleSizeOpt0 = containingBlock->vliwEndAddress - containingBlock->vliwStartAddress;

		fprintf(stderr, "While correcting a block %d -- %d = %d -- %d\n", containingBlock->vliwStartAddress, containingBlock->vliwEndAddress,newBlock->vliwStartAddress,  newBlock->vliwEndAddress);

		if (initialState > IRBLOCK_STATE_FIRSTPASS){

			optimizeBasicBlock(containingBlock, nonOptPlatform, application, placeCode);
			optimizeBasicBlock(newBlock, nonOptPlatform, application, placeCode);

			blockInfo[containingBlock->sourceStartAddress].scheduleSizeOpt1 = containingBlock->vliwEndAddress - containingBlock->vliwStartAddress;;
			blockInfo[newBlock->sourceStartAddress].scheduleSizeOpt1 = newBlock->vliwEndAddress - newBlock->vliwStartAddress;


			if (containingBlock->blockState >= IRBLOCK_PROC){
					//The containing block has been optimized at procedure level. Have to de-optimize it

				//We search for the containing procedure
					IRProcedure *containingProcedure = NULL;
					for (int oneProcedure = 0; oneProcedure < application->numberProcedures; oneProcedure++){
						IRProcedure *procedure = application->procedures[oneProcedure];
						for (int oneBlockInProc = 0; oneBlockInProc < procedure->nbBlock; oneBlockInProc++){
							if (containingBlock == procedure->blocks[oneBlockInProc]){
								containingProcedure = procedure;
								break;
							}

						}

						if (containingProcedure != NULL)
							break;
					}




					//we insert the block in the procedure
					IRBlock ** blocks = (IRBlock**) malloc((containingProcedure->nbBlock + 1) * sizeof(IRBlock *));
					int blockToCopy = 0;
					int blockInDest = 0;
					while (blockToCopy != containingProcedure->nbBlock){
						if (containingProcedure->blocks[blockToCopy] == containingBlock){
							blocks[blockInDest] = containingBlock;
							blocks[blockInDest + 1] = newBlock;
							blockInDest++;
						}
						blocks[blockInDest] = containingProcedure->blocks[blockToCopy];
						blockInDest++;
						blockToCopy++;
					}

					free(containingProcedure->blocks);
					containingProcedure->blocks = blocks;
					containingProcedure->nbBlock++;

					//We reschedule th eprocedure
					rescheduleProcedure(platform, containingProcedure, placeCode);

					//We update information
					blockInfo[containingBlock->sourceStartAddress].scheduleSizeOpt2 = containingBlock->vliwEndAddress - containingBlock->vliwStartAddress;;
					blockInfo[newBlock->sourceStartAddress].scheduleSizeOpt2 = newBlock->vliwEndAddress - newBlock->vliwStartAddress;

				}

		}
	}


}
