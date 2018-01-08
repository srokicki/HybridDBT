/*
 * irGenerator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_IRGENERATOR_H_
#define INCLUDES_TRANSFORMATION_IRGENERATOR_H_

#include <types.h>
#include <dbt/dbtPlateform.h>

unsigned int irGenerator_hw(ac_int<128, false> srcBinaries[1024], ac_int<32, false> addressInBinaries, ac_int<32, false> blockSize,
		ac_int<128, false> bytecode[1024], ac_int<32, true> globalVariables[128],
		ac_int<32, false> globalVariableCounter);

unsigned int irGenerator(DBTPlateform *platform,
		unsigned int addressInBinaries,
		unsigned int blockSize,
		unsigned int globalVariableCounter);


#endif /* INCLUDES_TRANSFORMATION_IRGENERATOR_H_ */
