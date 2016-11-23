/*
 * endianness.h
 *
 *  Created on: 28 juil. 2016
 *      Author: simon
 */

#ifndef ENDIANNESS__
#define ENDIANNESS__

#include <types.h>

#ifdef __USE_AC
#include <lib/ac_int.h>
#endif

void writeInt(unsigned char* bytecode, int place, unsigned int value);
void writeInt(ac_int<128, false>* bytecode, int place, unsigned int value);


unsigned int readInt(unsigned char* bytecode, int place);
uint32 readInt(uint128* bytecode, int place);



#endif
