/*
 * irGenerator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_IRGENERATOR_H_
#define INCLUDES_TRANSFORMATION_IRGENERATOR_H_

#include <dbt/dbtPlateform.h>
#include <types.h>

#ifndef __SW
#ifndef __HW

unsigned int irGenerator_hw(ac_int<128, false> srcBinaries[1024], ac_int<32, false> addressInBinaries,
                            ac_int<32, false> blockSize, ac_int<128, false> bytecode[1024],
                            ac_int<32, true> globalVariables[128], ac_int<32, false> globalVariableCounter);

#endif
#endif

unsigned int irGenerator_sw(unsigned int* srcBinaries, unsigned int addressInBinaries, unsigned int blockSize,
                            unsigned int* bytecode, int globalVariables[128], unsigned int globalVariableCounter);

unsigned int irGenerator(DBTPlateform* platform, unsigned int addressInBinaries, unsigned int blockSize,
                         unsigned int globalVariableCounter);

#ifndef __CATAPULT
// Performance simulation
extern int timeTakenIRGeneration;
#endif

#endif /* INCLUDES_TRANSFORMATION_IRGENERATOR_H_ */
