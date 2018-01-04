/*
 * firstPassTranslator.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */
#include <types.h>
#include <dbt/dbtPlateform.h>

#ifndef INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_
#define INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_

//Parameters:
#define MEMORY_LATENCY 3
#define MULT_LATENCY 3
#define SIMPLE_LATENCY 2

uint32 firstPassTranslator_MIPS(DBTPlateform *platform,
		uint32 size,
		uint32 codeSectionStart,
		uint32 addressStart,
		uint32 placeCode);

unsigned int firstPassTranslator(DBTPlateform *platform,
		unsigned int size,
		unsigned int codeSectionStart,
		unsigned int addressStart,
		unsigned int placeCode);


int firstPassTranslation_riscv_sw(unsigned int code[1024],
		unsigned int size,
		unsigned char conf,
		unsigned int addressStart,
		unsigned int codeSectionStart,
		unsigned int destinationBinaries[4*1024],
		unsigned int placeCode,
		unsigned int insertions[256],
		bool blocksBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned int unresolvedJumps_type[512],
		unsigned int unresolvedJumps[512]);

int firstPassTranslator_riscv_hw(uint32 code[1024],
		uint32 size,
		uint8 conf,
		uint32 addressStart,
		uint32 codeSectionStart,
		uint128 destinationBinaries[1024],
		uint32 placeCode,
		uint32 insertions[256],
		uint1 blocksBoundaries[65536],
		uint32 unresolvedJumps_src[512],
		uint32 unresolvedJumps_type[512],
		int32 unresolvedJumps[512]);





#endif /* INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_ */
