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

#ifndef __NIOS
void writeInt(ac_int<128, false>* bytecode, int place, unsigned int value);
void writeInt(ac_int<32, false>* bytecode, int place, unsigned int value);
void writeChar(ac_int<32, false>* bytecode, int place, unsigned char value);
void writeInt(ac_int<8, false>* bytecode, int place, unsigned int value);
#endif

void write128(unsigned int *bytecode, int place, struct uint128_struct value);
void writeInt(unsigned int* bytecode, int place, unsigned int value);
void writeChar(unsigned int* bytecode, int place, unsigned char value);

void writeInt(unsigned char* bytecode, int place, unsigned int value);

/***********************************************************************
 * These three procedure are used to read an int inside a memory.
 * The addressing is made on byte.
 *
 * The input memory can be uint8*, uint32* or uint128*
 ***********************************************************************/


unsigned int readInt(unsigned int* bytecode, int place);
unsigned char readChar(unsigned int* bytecode, int place);

#ifndef __NIOS
ac_int<32, false> readInt(ac_int<128, false>* bytecode, int place);
ac_int<32, false> readInt(ac_int<8, false>* bytecode, int place);
ac_int<32, false> readInt(ac_int<32, false>* bytecode, int place);
ac_int<8, false> readChar(ac_int<32, false>* bytecode, int place);
#endif


/***********************************************************************
 * These procedure are used to copy the content of a ac_int memory to a normal memory.
 * It is supposed to cover all combinations used for the different accelerators.
 *
 * TODO: it should not be defined when running fully software or hw accelerated DBT, only one hw simulation
 ***********************************************************************/
void acintMemcpy(ac_int<128, false> *to, unsigned int *from, int sizeInByte);
void acintMemcpy(unsigned int *to, ac_int<128, false>  *from, int sizeInByte);
void acintMemcpy(ac_int<32, false> *to, unsigned int *from, int sizeInByte);
void acintMemcpy(unsigned int *to, ac_int<32, false>  *from, int sizeInByte);
void acintMemcpy(ac_int<8, false> *to, unsigned char *from, int sizeInByte);
void acintMemcpy(unsigned char *to, ac_int<8, false>  *from, int sizeInByte);
void acintMemcpy(ac_int<6, false> *to, unsigned char *from, int sizeInByte);
void acintMemcpy(unsigned char *to, ac_int<6, false>  *from, int sizeInByte);
void acintMemcpy(ac_int<1, false> *to, unsigned char *from, int sizeInByte);
void acintMemcpy(unsigned char *to, ac_int<1, false>  *from, int sizeInByte);
void acintMemcpy(ac_int<32, true> *to, int *from, int sizeInByte);
void acintMemcpy(int *to, ac_int<32, true>  *from, int sizeInByte);

#endif
