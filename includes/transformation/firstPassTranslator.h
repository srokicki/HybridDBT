/*
 * firstPassTranslator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */
#include <types.h>


#ifndef INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_
#define INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_

uint32 firstPassTranslator_MIPS(DBTPlateform *platform,
		uint32 size,
		uint32 codeSectionStart,
		uint32 addressStart,
		uint32 placeCode);

uint32 firstPassTranslator_RISCV(DBTPlateform *platform,
		uint32 size,
		uint32 codeSectionStart,
		uint32 addressStart,
		uint32 placeCode);

#endif /* INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_ */
