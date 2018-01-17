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

#ifdef __USE_MIPS

uint32 firstPassTranslator_MIPS(DBTPlateform *platform,
		uint32 size,
		uint32 codeSectionStart,
		uint32 addressStart,
		uint32 placeCode);
#endif

unsigned int firstPassTranslator(DBTPlateform *platform,
		unsigned int size,
		unsigned int codeSectionStart,
		unsigned int addressStart,
		unsigned int placeCode);


int firstPassTranslator_riscv_sw(unsigned int code[1024],
		unsigned int size,
		unsigned char conf,
		unsigned int addressStart,
		unsigned int codeSectionStart,
		unsigned int destinationBinaries[4*1024],
		unsigned int placeCode,
		unsigned int insertions[256],
		unsigned char blocksBoundaries[65536],
		unsigned int unresolvedJumps_src[512],
		unsigned int unresolvedJumps_type[512],
		int unresolvedJumps[512]);


#ifndef __SW
#ifndef __HW

int firstPassTranslator_riscv_hw(ac_int<32, false> code[1024],
		ac_int<32, false> size,
		ac_int<8, false> conf,
		ac_int<32, false> addressStart,
		ac_int<32, false> codeSectionStart,
		ac_int<128, false> destinationBinaries[1024],
		ac_int<32, false> placeCode,
		ac_int<32, false> insertions[256],
		ac_int<1, false> blocksBoundaries[65536],
		ac_int<32, false> unresolvedJumps_src[512],
		ac_int<32, false> unresolvedJumps_type[512],
		ac_int<32, true> unresolvedJumps[512]);

#endif
#endif



#endif /* INCLUDES_TRANSFORMATION_FIRSTPASSTRANSLATOR_H_ */
