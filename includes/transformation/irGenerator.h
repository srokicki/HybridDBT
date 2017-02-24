/*
 * irGenerator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_IRGENERATOR_H_
#define INCLUDES_TRANSFORMATION_IRGENERATOR_H_

#include <types.h>

unsigned int irGenerator(DBTPlateform *platform,
		uint16 addressInBinaries,
		uint32 blockSize,
		uint32 globalVariableCounter);


#endif /* INCLUDES_TRANSFORMATION_IRGENERATOR_H_ */
