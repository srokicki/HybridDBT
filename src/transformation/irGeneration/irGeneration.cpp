/*
 * irGeneration.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#include <cstdio>
#include <cstdlib>
#include <transformation/irGenerator.h>
#include <lib/endianness.h>
#include <lib/tools.h>
#include <dbt/dbtPlateform.h>

#include <isa/vexISA.h>
#include <isa/irISA.h>

#include <lib/log.h>

/****************************************************************************
 *  Definition of the procedure irGenerator
****************************************************************************
 *
 *  This procedure is the one that is visible from the dbt framework, the one defined
 *  in the corresponding header file.
 *  This procedure will call the hardware accelerator or its cpp implementation.
 ****************************************************************************/

#ifndef __CATAPULT

unsigned int irGenerator(DBTPlateform *platform,
		unsigned int addressInBinaries,
		unsigned int blockSize,
		unsigned int globalVariableCounter){

	//Modelization of optimization time : here we need 5 cycles to generate IR for one instruction
	platform->optimizationCycles += blockSize*5;
	platform->optimizationEnergy += ((int)blockSize)*5*3.22f;

	#ifndef __NIOS


	ac_int<128, false> *localBytecode = (ac_int<128, false>*) malloc(256*sizeof(ac_int<128, false>));
	ac_int<128, false> *localVliwBinaries = (ac_int<128, false>*) malloc(MEMORY_SIZE*sizeof(ac_int<128, false>));
	ac_int<32, true> *localGlobalVariables = (ac_int<32, true>*) malloc(64*sizeof(ac_int<32, true>));

	acintMemcpy(localBytecode, platform->bytecode, 256*16);
	acintMemcpy(localVliwBinaries, platform->vliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(localGlobalVariables, platform->globalVariables, 64*4);

	ac_int<32, false> localGlobalVariableCounter = globalVariableCounter;

	unsigned int result = irGenerator_hw(localVliwBinaries,
			addressInBinaries,
			blockSize,
			localBytecode,
			localGlobalVariables,
			localGlobalVariableCounter);




	acintMemcpy(platform->bytecode, localBytecode, 256*16);
	acintMemcpy(platform->vliwBinaries, localVliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(platform->globalVariables, localGlobalVariables, 64*4);

	return result;

	#else

	int argA = blockSize + (addressInBinaries << 16);
	int argB = globalVariableCounter;
	return ALT_CI_COMPONENT_IRGENERATOR_HW_0(argA, argB);

	#endif


}

#endif

