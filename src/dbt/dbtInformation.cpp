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


#ifndef __NIOS
#include <lib/elfFile.h>
#else
#include <system.h>
#endif




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



int getDBTInformation(char* fileName, int OPTLEVEL, int DBT_TYPE)
{


	int CONFIGURATION = 2;
	int VERBOSE = 0;
	Log::Init(VERBOSE, 0);


	/***********************************
	 *  Initialization of the DBT platform
	 ***********************************
	 * In the linux implementation, this is done by reading an elf file and copying binary instructions
	 * in the corresponding memory.
	 * In a real platform, this may require no memory initialization as the binaries would already be stored in the
	 * system memory.
	 ************************************/

	//Definition of objects used for DBT process
	DBTPlateform dbtPlateform(MEMORY_SIZE);

	unsigned char* code;
	unsigned int addressStart;
	unsigned int size;
	unsigned int pcStart;

	readSourceBinaries(fileName, code, addressStart, size, pcStart, &dbtPlateform);


	dbtPlateform.vliwInitialConfiguration = CONFIGURATION;
	dbtPlateform.vliwInitialIssueWidth = getIssueWidth(dbtPlateform.vliwInitialConfiguration);
	dbtPlateform.dbtType = DBT_TYPE;

	//Preparation of required memories
	for (int oneFreeRegister = 33; oneFreeRegister<63; oneFreeRegister++)
		dbtPlateform.freeRegisters[oneFreeRegister-33] = oneFreeRegister;

	for (int oneFreeRegister = 63-33; oneFreeRegister<63; oneFreeRegister++)
		dbtPlateform.freeRegisters[oneFreeRegister] = 0;

	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		dbtPlateform.placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;
	//same for FP registers
	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		dbtPlateform.placeOfRegisters[256+64+onePlaceOfRegister] = onePlaceOfRegister;

  	dbtPlateform.vexSimulator = new LoadQueueVexSimulator(dbtPlateform.vliwBinaries, dbtPlateform.specInfo);
	setVLIWConfiguration(dbtPlateform.vexSimulator, dbtPlateform.vliwInitialConfiguration);





	int numberOfSections = 1 + (size>>10);
	IRApplication application = IRApplication(numberOfSections);
	Profiler profiler = Profiler(&dbtPlateform);
	application.numberInstructions = size;



	//we copy the binaries in the corresponding memory
	for (int oneInstruction = 0; oneInstruction<size; oneInstruction++)
		dbtPlateform.mipsBinaries[oneInstruction] = ((unsigned int*) code)[oneInstruction];


	//We declare the variable in charge of keeping a track of where we are writing
	unsigned int placeCode = 0; //As 4 instruction bundle

	//We add initialization code to the vliw binaries
	placeCode = getInitCode(&dbtPlateform, placeCode, addressStart);
	placeCode = insertCodeForInsertions(&dbtPlateform, placeCode, addressStart);

	//We modify the initialization call
	writeInt(dbtPlateform.vliwBinaries, 0*16, assembleIInstruction_sw(VEX_CALL, placeCode, 63));

	initializeInsertionsMemory(size*4);
//	for (int oneInsertion=0; oneInsertion<placeCode; oneInsertion++){
//			Log::fprintf(0, stderr, "insert;%d\n", oneInsertion);
//	}
	/********************************************************
	 * First part of DBT: generating the first pass translation of binaries
	 *******************************************************
	 * TODO: description
	 *
	 *
	 ********************************************************/

	for (int i=0; i<placeCode; i++){
		dbtPlateform.vexSimulator->typeInstr[i] = 3;
	}
	int endOfInitSection = placeCode;

	for (int oneSection=0; oneSection<(size>>10)+1; oneSection++){

		int startAddressSource = addressStart + oneSection*1024*4;
		int endAddressSource = startAddressSource + 1024*4;
		if (endAddressSource > addressStart + size*4)
			endAddressSource = addressStart + (size<<2);


		int effectiveSize = (endAddressSource - startAddressSource)>>2;
		for (int j = 0; j<effectiveSize; j++){
			dbtPlateform.mipsBinaries[j] = ((unsigned int*) code)[j+oneSection*1024];
		}
		int oldPlaceCode = placeCode;

		placeCode =  translateOneSection(dbtPlateform, placeCode, addressStart, startAddressSource,endAddressSource);

		buildBasicControlFlow(&dbtPlateform, oneSection,addressStart, startAddressSource, oldPlaceCode, placeCode, &application, &profiler);

//		int** insertions = (int**) malloc(sizeof(int **));
//		int nbIns = getInsertionList(oneSection*1024, insertions);
//		for (int oneInsertion=0; oneInsertion<nbIns; oneInsertion++){
//				Log::fprintf(0, stderr, "insert;%d\n", (*insertions)[oneInsertion]+(*insertions)[-1]);
//		}
	}

	for (int oneUnresolvedJump = 0; oneUnresolvedJump<unresolvedJumpsArray[0]; oneUnresolvedJump++){
		unsigned int source = unresolvedJumpsSourceArray[oneUnresolvedJump+1];
		unsigned int initialDestination = unresolvedJumpsArray[oneUnresolvedJump+1];
		unsigned int type = unresolvedJumpsTypeArray[oneUnresolvedJump+1];

		unsigned char isAbsolute = ((type & 0x7f) != VEX_BR) && ((type & 0x7f) != VEX_BRF && (type & 0x7f) != VEX_BLTU) && ((type & 0x7f) != VEX_BGE && (type & 0x7f) != VEX_BGEU) && ((type & 0x7f) != VEX_BLT);
		unsigned int destinationInVLIWFromNewMethod = solveUnresolvedJump(&dbtPlateform, initialDestination);

		if (destinationInVLIWFromNewMethod == -1){
			Log::printf(LOG_ERROR, "A jump from %d to %x is still unresolved... (%d insertions)\n", source, initialDestination, insertionsArray[(initialDestination>>10)<<11]);
			exit(-1);
		}
		else{
			int immediateValue = (isAbsolute) ? (destinationInVLIWFromNewMethod) : ((destinationInVLIWFromNewMethod  - source));
			int mask = (isAbsolute) ? 0x7ffff : 0x1fff;

			writeInt(dbtPlateform.vliwBinaries, 16*(source), type + ((immediateValue & mask)<<7));

			if (immediateValue > 0x7ffff){
				Log::fprintf(LOG_ERROR, stderr, "error in immediate size...\n");
				exit(-1);
			}
			unsigned int instructionBeforePreviousDestination = readInt(dbtPlateform.vliwBinaries, 16*(destinationInVLIWFromNewMethod-1)+12);
			if (instructionBeforePreviousDestination != 0)
				writeInt(dbtPlateform.vliwBinaries, 16*(source+1)+12, instructionBeforePreviousDestination);
		}
	}

	for (int i=endOfInitSection; i<placeCode; i++){
		dbtPlateform.vexSimulator->typeInstr[i] = 0;
	}


	//We also add information on insertions
	int insertionSize = 65536;
	int areaCodeStart=1;
	int areaStartAddress = 0;


	//We change the init code to jump at the correct place
	unsigned int translatedStartPC = solveUnresolvedJump(&dbtPlateform, (pcStart-addressStart)>>2);
	unsigned int instruction = assembleIInstruction_sw(VEX_CALL, translatedStartPC, 63);
	writeInt(dbtPlateform.vliwBinaries, 0, instruction);




	int runStatus=0;
	int abortCounter = 0;


	//We modelize the translation time

	//SW version
//	dbtPlateform.vexSimulator->cycle = 1000*425;
//	runStatus = run(&dbtPlateform, size*425);

	//HW version
	dbtPlateform.vexSimulator->cycle = 1000;
	runStatus = 1;//= run(&dbtPlateform, dbtPlateform.optimizationCycles);

	int profileGap = 32;

	while (runStatus == 0){



		int oldOptimizationCount = dbtPlateform.optimizationCycles;
		bool optimizationPerformed = false;


		updateSpeculationsStatus(&dbtPlateform, placeCode);

		if (OPTLEVEL >= 5){
			for (int oneProcedure = 0; oneProcedure < application.numberProcedures; oneProcedure++){
				IRProcedure *procedure = application.procedures[oneProcedure];
				if (procedure->state == 0){
					char oldPrevious = procedure->previousConfiguration;
					char oldConf = procedure->configuration;

					changeConfiguration(procedure);
					if (procedure->configuration != oldConf || procedure->configurationScores[procedure->configuration] == 0){
						IRProcedure *scheduledProcedure = rescheduleProcedure_schedule(&dbtPlateform, procedure, placeCode);
						suggestConfiguration(procedure, scheduledProcedure);

						int score = computeScore(scheduledProcedure);
						procedure->configurationScores[procedure->configuration] = score;

						if (score > procedure->configurationScores[procedure->previousConfiguration]){
							placeCode = rescheduleProcedure_commit(&dbtPlateform, procedure, placeCode, scheduledProcedure);
						}
						else{
							procedure->configuration = procedure->previousConfiguration;
							procedure->previousConfiguration = oldPrevious;
						}
					}
					optimizationPerformed = true;
					break;
				}
				else if (procedure->state==1){
					char maxConf = 0;
					int bestScore = 0;
					for (int oneConfiguration = 1; oneConfiguration<12; oneConfiguration++){
						char configuration = validConfigurations[oneConfiguration];
						float score = procedure->configurationScores[configuration];
						float energy = (score * getPowerConsumption(configuration))/10;


					}
					procedure->previousConfiguration = procedure->configuration;
					procedure->configuration = maxConf;
					placeCode = rescheduleProcedure(&dbtPlateform, procedure, placeCode);
					procedure->state = 2;
					dbtPlateform.procedureOptCounter++;
					optimizationPerformed = true;
				}
			}
		}
		for (int oneBlock = 0; oneBlock<profiler.getNumberProfiledBlocks(); oneBlock++){
			int profileResult = profiler.getProfilingInformation(oneBlock);
			IRBlock* block = profiler.getBlock(oneBlock);

			if (block != NULL && OPTLEVEL >= 2 && profileResult >= 15 && (block->blockState == IRBLOCK_STATE_SCHEDULED || block->blockState == IRBLOCK_STATE_PROFILED)){

				int errorCode = buildAdvancedControlFlow(&dbtPlateform, block, &application);
				block->blockState = IRBLOCK_PROC;

				if (!errorCode){
					buildTraces(&dbtPlateform, application.procedures[application.numberProcedures-1], OPTLEVEL);
					placeCode = rescheduleProcedure(&dbtPlateform, application.procedures[application.numberProcedures-1], placeCode);
					dbtPlateform.procedureOptCounter++;

				}
				else{
				}


				optimizationPerformed = true;
				break;
			}

		}

		//We perform aggressive level 1 optimization: if a block takes more than 8 cycle we schedule it.
		//If it has a backward loop, we also profile it.
		for (int oneSection = 0; oneSection<numberOfSections; oneSection++){
			for (int oneBlock = 0; oneBlock<application.numbersBlockInSections[oneSection]; oneBlock++){
				IRBlock* block = application.blocksInSections[oneSection][oneBlock];

				if (block != NULL && block->sourceStartAddress != -1){
					if (OPTLEVEL >= 1/* && block->sourceEndAddress - block->sourceStartAddress > profileGap */&& (block->sourceEndAddress - block->sourceStartAddress > 4 || (block->sourceDestination != -1 && block->sourceDestination <= block->sourceStartAddress) )  && block->blockState < IRBLOCK_STATE_SCHEDULED){

						optimizeBasicBlock(block, &dbtPlateform, &application, placeCode);
						dbtPlateform.blockScheduleCounter++;

						if ((block->sourceDestination != -1 && block->sourceDestination <= block->sourceStartAddress) || block->nbInstr > 32){

							profiler.profileBlock(block);
						}

						optimizationPerformed = true;
						break;
					}

				}

			}

			if (optimizationPerformed)
				break;

		}

		int cyclesToRun = dbtPlateform.optimizationCycles - oldOptimizationCount;
		if (cyclesToRun == 0){
			cyclesToRun = 1000;
			profileGap = profileGap>>1;
		}

		if (dbtPlateform.vexSimulator->cycle > 2000000000)
			break;

//		runStatus = run(&dbtPlateform, cyclesToRun);


	}

	//We compute pareto domains
	for (int oneProcedure = 0; oneProcedure < application.numberProcedures; oneProcedure++){
		IRProcedure *procedure = application.procedures[oneProcedure];

		for (int oneConfiguration = 0; oneConfiguration<12; oneConfiguration++){
			char configuration = validConfigurations[oneConfiguration];
			float score = procedure->configurationScores[configuration];
			float energy = (score * getPowerConsumption(configuration))/10;
			bool isPareto = true;

			for (int oneOtherConf = 0; oneOtherConf<12; oneOtherConf++){
				char otherConf = validConfigurations[oneOtherConf];
				float otherScore = procedure->configurationScores[otherConf];
				float otherEnergy = (otherScore * getPowerConsumption(otherConf))/10;

				if ((otherScore >= score && otherEnergy < energy) || (otherEnergy <= energy && otherScore > score)){
					isPareto = false;
					break;
				}
			}

			if (isPareto)
				dbtPlateform.nbTimesInPareto[configuration]++;
		}
	}

	//We clean the last performance counters
	dbtPlateform.vexSimulator->timeInConfig[dbtPlateform.vexSimulator->currentConfig] += (dbtPlateform.vexSimulator->cycle - dbtPlateform.vexSimulator->lastReconf);

	Log::printStat(&dbtPlateform, &application);



	int nbFirstPass = 0;
	int nbScheduled = 0;
	int nbProc = 0;




	//We print profiling result
	delete dbtPlateform.vexSimulator;


  return 0;
}

int main(int argc, char** argv){
	getDBTInformation(argv[1], 5, 0);
}

