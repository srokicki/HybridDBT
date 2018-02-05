/*
 * irScheduling.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#include <transformation/irScheduler.h>
#include <dbt/dbtPlateform.h>
#include <types.h>
#include <transformation/reconfigureVLIW.h>
#include <lib/endianness.h>
#include <lib/dbtProfiling.h>

#ifndef __CATAPULT
//Performance simulation
int timeTakenIRScheduler;
#endif

int irScheduler(DBTPlateform *platform, bool opt, unsigned char basicBlockSize, unsigned int addressInBinaries,
unsigned char numberFreeRegister, char configuration){

	//********************************************
	//Modelization of optimization time : here we need 4 cycles to schedule one instruction

	platform->optimizationCycles += basicBlockSize*4;
	platform->optimizationEnergy += ((int)basicBlockSize)*4*5.46;


	char issue_width = getIssueWidth(configuration)>4 ? 8 :4;
	unsigned int way_specialisation = getConfigurationForScheduler(configuration);

	#ifdef __SW_HW_SIM

	/********************************************************************************************
	 * First version of sources for _SW_HW_SIM
	 *
	 * The pass is done in SW and in HW and the outputs are compared. If they are different
	 * it returns an error message.
	 * This should be the default mode
	 ********************************************************************************************/

	ac_int<128, false> *localBytecode = (ac_int<128, false>*) malloc(256*sizeof(ac_int<128, false>));
	ac_int<128, false> *localVliwBinaries = (ac_int<128, false>*) malloc(MEMORY_SIZE*sizeof(ac_int<128, false>));
	ac_int<6, false> *localPlaceOfRegisters = (ac_int<6, false>*) malloc(512*sizeof(ac_int<6, false>));
	ac_int<6, false> *localFreeRegisters = (ac_int<6, false>*) malloc(64*sizeof(ac_int<6, false>));
	ac_int<32, false> *localPlaceOfInstr = (ac_int<32, false>*) malloc(256*sizeof(ac_int<32, false>));

	acintMemcpy(localBytecode, platform->bytecode, 256*16);
	acintMemcpy(localVliwBinaries, platform->vliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(localPlaceOfRegisters, platform->placeOfRegisters, 512);
	acintMemcpy(localFreeRegisters, platform->freeRegisters, 64);
	acintMemcpy(localPlaceOfInstr, platform->placeOfInstr, 256*4);


	ac_int<32, false> result =  irScheduler_scoreboard_hw(opt, basicBlockSize, localBytecode, localVliwBinaries, addressInBinaries, localPlaceOfRegisters,
	numberFreeRegister, localFreeRegisters, issue_width, way_specialisation, localPlaceOfInstr);

	unsigned int swResult = irScheduler_scoreboard_sw(opt, basicBlockSize, platform->bytecode, platform->vliwBinaries, addressInBinaries, platform->placeOfRegisters, numberFreeRegister, platform->freeRegisters, issue_width, way_specialisation, platform->placeOfInstr);

	if (result != swResult){
		fprintf(stderr, "Result of schedulers are different : %d vs %d\n", result, swResult);
		exit(-1);
	}

//	for (int oneSourceValue = addressInBinaries; oneSourceValue<addressInBinaries+result+15; oneSourceValue++)
//	fprintf(stderr, "%d   %x %x %x %x vs %x %x %x %x\n", oneSourceValue, platform->vliwBinaries[4*oneSourceValue+0], platform->vliwBinaries[4*oneSourceValue+1], platform->vliwBinaries[4*oneSourceValue+2], platform->vliwBinaries[4*oneSourceValue+3],
//			readInt(localVliwBinaries, 16*oneSourceValue + 0), readInt(localVliwBinaries, 16*oneSourceValue + 4), readInt(localVliwBinaries, 16*oneSourceValue + 8), readInt(localVliwBinaries, 16*oneSourceValue + 12));
//	fprintf(stderr, "\n");

	if (!acintCmp(platform->bytecode, localBytecode, 256*16)){
		fprintf(stderr, "Error: After scheduling, bytecode are different...\n");
		exit(-1);
	}

	if (!acintCmp(platform->vliwBinaries, localVliwBinaries, MEMORY_SIZE*16)){
		fprintf(stderr, "Error: After scheduling, vliw binaries are different...\n");
		exit(-1);
	}

	if (!acintCmp(platform->placeOfRegisters, localPlaceOfRegisters, basicBlockSize)){
		fprintf(stderr, "Error: After scheduling, place of reg are different...\n");
		exit(-1);
	}

	if (!acintCmp(platform->freeRegisters, localFreeRegisters, 64)){
		fprintf(stderr, "Error: After scheduling, free reg are different...\n");
		exit(-1);
	}

	if (!acintCmp(platform->placeOfInstr, localPlaceOfInstr, 256*4)){
		fprintf(stderr, "Error: After scheduling, place of instr are different...\n");
		exit(-1);
	}


	free(localBytecode);
	free(localVliwBinaries);
	free(localPlaceOfRegisters);
	free(localPlaceOfInstr);
	free(localFreeRegisters);

	return (unsigned int) result;

	#endif


	#ifdef __SW

	/********************************************************************************************
	 * Second version of sources for __SW
	 *
	 * The pass is done in SW only and the result is returned
	 *
	 ********************************************************************************************/

	startProfiler(2);
	int result = irScheduler_scoreboard_sw(opt, basicBlockSize, platform->bytecode, platform->vliwBinaries, addressInBinaries, platform->placeOfRegisters, numberFreeRegister, platform->freeRegisters, issue_width, way_specialisation, platform->placeOfInstr);
	stopProfiler(2);

	return result;
	#endif



	#ifdef __HW_SIM

	/********************************************************************************************
	 * Second version of sources for __SW
	 *
	 * The pass is done in HW simulation only: all values coming from normal memories (unsigned int
	 * and not ac_int types) are copied into newly allocated arrays using the correct type.
	 * Then the pass is called in and the result is copied back to the normal memories.
	 *
	 ********************************************************************************************/

	ac_int<128, false> *localBytecode = (ac_int<128, false>*) malloc(256*sizeof(ac_int<128, false>));
	ac_int<128, false> *localVliwBinaries = (ac_int<128, false>*) malloc(MEMORY_SIZE*sizeof(ac_int<128, false>));
	ac_int<6, false> *localPlaceOfRegisters = (ac_int<6, false>*) malloc(512*sizeof(ac_int<6, false>));
	ac_int<6, false> *localFreeRegisters = (ac_int<6, false>*) malloc(64*sizeof(ac_int<6, false>));
	ac_int<32, false> *localPlaceOfInstr = (ac_int<32, false>*) malloc(256*sizeof(ac_int<32, false>));

	acintMemcpy(localBytecode, platform->bytecode, 256*16);
	acintMemcpy(localVliwBinaries, platform->vliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(localPlaceOfRegisters, platform->placeOfRegisters, 512);
	acintMemcpy(localFreeRegisters, platform->freeRegisters, 64);
	acintMemcpy(localPlaceOfInstr, platform->placeOfInstr, 256*4);


	ac_int<32, false> result =  irScheduler_scoreboard_hw(opt, basicBlockSize, localBytecode, localVliwBinaries, addressInBinaries, localPlaceOfRegisters,
	numberFreeRegister, localFreeRegisters, issue_width, way_specialisation, localPlaceOfInstr);

	acintMemcpy(platform->bytecode, localBytecode, 256*16);
	acintMemcpy(platform->vliwBinaries, localVliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(platform->placeOfRegisters, localPlaceOfRegisters, basicBlockSize);
	acintMemcpy(platform->freeRegisters, localFreeRegisters, 64);
	acintMemcpy(platform->placeOfInstr, localPlaceOfInstr, 256*4);


	free(localBytecode);
	free(localVliwBinaries);
	free(localPlaceOfRegisters);
	free(localPlaceOfInstr);
	free(localFreeRegisters);

	return (unsigned int) result;

	#endif
}


