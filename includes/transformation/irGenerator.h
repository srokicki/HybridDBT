/*
 * irGenerator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_IRGENERATOR_H_
#define INCLUDES_TRANSFORMATION_IRGENERATOR_H_

#include <types.h>

int irGenerator(unsigned char* code, unsigned int *size, unsigned int addressStart,
		unsigned char* bytecode, unsigned int *placeCode,
		short* blocksBoundaries, short* proceduresBoundaries, int* insertions);

unsigned int irGenerator_hw(uint128 srcBinaries[1024], uint16 addressInBinaries, uint32 blockSize,
		uint128 bytecode[1024], int32 globalVariables[64],
		uint64 registersUsage[1], uint32 globalVariableCounter);

#endif /* INCLUDES_TRANSFORMATION_IRGENERATOR_H_ */
