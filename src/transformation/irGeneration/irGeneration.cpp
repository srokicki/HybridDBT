/*
 * irGeneration.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#ifndef __CATAPULT
//Includes not required by catapult
#include <cstdio>
#include <cstdlib>

#include <lib/endianness.h>
#include <lib/tools.h>
#include <dbt/dbtPlateform.h>
#endif

//Includes required by catapult
#include <isa/vexISA.h>
#include <isa/irISA.h>

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
		uint32 addressInBinaries,
		uint32 blockSize,
		uint32 globalVariableCounter){

	//Modelization of optimization time : here we need 5 cycles to generate IR for one instruction
	platform->optimizationCycles += blockSize*5;
	platform->optimizationEnergy += ((int)blockSize)*5*3.22f;

	#ifndef __NIOS

	return irGenerator_hw(platform->vliwBinaries,
			addressInBinaries,
			blockSize,
			platform->bytecode,
			platform->globalVariables,
			globalVariableCounter);
	#else

	int argA = blockSize + (addressInBinaries << 16);
	int argB = globalVariableCounter;
	return ALT_CI_COMPONENT_IRGENERATOR_HW_0(argA, argB);

	#endif
}

#endif

