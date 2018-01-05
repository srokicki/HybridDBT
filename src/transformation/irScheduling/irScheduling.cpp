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


int irScheduler(DBTPlateform *platform, bool opt, unsigned char basicBlockSize, unsigned int addressInBinaries,
unsigned char numberFreeRegister, char configuration){

	//********************************************
	//Modelization of optimization time : here we need 4 cycles to schedule one instruction

	platform->optimizationCycles += basicBlockSize*4;
	platform->optimizationEnergy += ((int)basicBlockSize)*4*5.46;


	char issue_width = getIssueWidth(configuration)>4 ? 8 :4;
	unsigned int way_specialisation = getConfigurationForScheduler(configuration);


	#ifndef IR_SUCC

#ifndef __NIOS
	return irScheduler_list_hw(opt, basicBlockSize, platform->bytecode, platform->vliwBinaries, addressInBinaries, platform->placeOfRegisters,
	numberFreeRegister, platform->freeRegisters, issue_width, way_specialisation, platform->placeOfInstr);
#else
	unsigned int argA = opt + (basicBlockSize << 1) + (addressInBinaries << 16);
	unsigned int argB = issue_width + (way_specialisation << 4);
	return ALT_CI_COMPONENT_SCHEDULING_0(argA, argB);
#endif

	#else

#ifndef __NIOS
	return irScheduler_scoreboard_hw(opt, basicBlockSize, platform->bytecode, platform->vliwBinaries, addressInBinaries, platform->placeOfRegisters,
	numberFreeRegister, platform->freeRegisters, issue_width, way_specialisation, platform->placeOfInstr);
#else
	unsigned int argA = opt + (basicBlockSize << 1) + (addressInBinaries << 16);
	unsigned int argB = issue_width + (way_specialisation << 4);
	return ALT_CI_COMPONENT_SCHEDULING_0(argA, argB);
#endif
#endif

}


