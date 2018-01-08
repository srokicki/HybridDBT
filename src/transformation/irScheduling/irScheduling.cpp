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


int irScheduler(DBTPlateform *platform, bool opt, unsigned char basicBlockSize, unsigned int addressInBinaries,
unsigned char numberFreeRegister, char configuration){

	//********************************************
	//Modelization of optimization time : here we need 4 cycles to schedule one instruction

	platform->optimizationCycles += basicBlockSize*4;
	platform->optimizationEnergy += ((int)basicBlockSize)*4*5.46;


	char issue_width = getIssueWidth(configuration)>4 ? 8 :4;
	unsigned int way_specialisation = getConfigurationForScheduler(configuration);





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
	acintMemcpy(platform->placeOfRegisters, localPlaceOfRegisters, 512);
	acintMemcpy(platform->freeRegisters, localFreeRegisters, 64);
	acintMemcpy(platform->placeOfInstr, localPlaceOfInstr, 256*4);

	return (unsigned int) result;


}


