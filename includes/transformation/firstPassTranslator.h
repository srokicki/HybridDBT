/*
 * firstPassTranslator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */
#include <types.h>


#ifndef INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_
#define INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_

int firstPassTranslator(uint32 *code,
		uint32 *size,
		uint32 addressStart,
		uint128 *destinationBinaries,
		uint32 *placeCode,
		uint32 *insertions,
		int16 *blocksBoundaries,
		int16 *proceduresBoundaries);

#endif /* INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_ */
