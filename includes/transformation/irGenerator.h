/*
 * irGenerator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_IRGENERATOR_H_
#define INCLUDES_TRANSFORMATION_IRGENERATOR_H_

int irGenerator(unsigned char* code, unsigned int *size, unsigned int addressStart,
		unsigned char* bytecode, unsigned int *placeCode,
		short* blocksBoundaries, short* proceduresBoundaries, int* insertions);



#endif /* INCLUDES_TRANSFORMATION_IRGENERATOR_H_ */
