#include <parameters.h>
#include <types.h>

#ifndef __IR_SCHEDULER
#define __IR_SCHEDULER

int scheduling(uint1 optLevel, uint8 basicBlockSize, uint128 bytecode[256], uint128 binaries[1024], uint16 addressInBinaries,
		uint6 placeOfRegisters[512], uint6 numberFreeRegister, uint6 freeRegisters[64], uint4 issue_width,
		uintIW way_specialisation, uint32 placeOfInstr[256]);

#endif
