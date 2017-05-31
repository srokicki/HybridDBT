/*
 * endianness.h
 *
 *  Created on: 28 juil. 2016
 *      Author: simon
 */

#ifndef ENDIANNESS__
#define ENDIANNESS__

#include <types.h>

/***********************************************************************
 * These three procedure are used to write an int inside a memory.
 * The addressing is made on byte.
 *
 * The input memory can be uint8*, uint32* or uint128*
 ***********************************************************************/

void writeInt(uint8* bytecode, int place, unsigned int value);
void writeInt(uint32* bytecode, int place, unsigned int value);
void writeChar(uint32* bytecode, int place, unsigned char value);

#ifndef __NIOS
void writeInt(uint128* bytecode, int place, unsigned int value);
#endif


/***********************************************************************
 * These three procedure are used to read an int inside a memory.
 * The addressing is made on byte.
 *
 * The input memory can be uint8*, uint32* or uint128*
 ***********************************************************************/

uint32 readInt(uint8* bytecode, int place);
uint32 readInt(uint32* bytecode, int place);
uint8 readChar(uint32* bytecode, int place);

#ifndef __NIOS
uint32 readInt(uint128* bytecode, int place);
#endif


#endif
