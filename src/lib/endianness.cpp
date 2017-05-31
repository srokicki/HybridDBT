#include <types.h>


/***********************************************************************
 * These three procedure are used to write an int inside a memory.
 * The addressing is made on byte.
 *
 * The input memory can be uint8*, uint32* or uint128*
 ***********************************************************************/

void writeInt(uint8* bytecode, int place, unsigned int value){
	unsigned int *bytecodeAsInt = (unsigned int *) bytecode;
	//bytecodeAsInt[place>>2] = value;

	//FIXME endianness
	bytecode[place+3] = (value >> 24) & 0xff;
	bytecode[place+2] = (value >> 16) & 0xff;
	bytecode[place+1] = (value >> 8) & 0xff;
	bytecode[place+0] = (value >> 0) & 0xff;

}

void writeInt(uint32* bytecode, int place, unsigned int value){
	bytecode[(place>>2)/*+(3-(place&0x3))*/] = value;
}

void writeChar(uint32* bytecode, int place, unsigned char value){
	uint8 value_ac = value;
	bytecode[(place>>2)].set_slc(8*(3-(place & 0x3)), value_ac);
}

#ifndef __NIOS
void writeInt(uint128* bytecode, int place, unsigned int value){

	ac_int<32, false> valueAsAcInt = value;
	bytecode[place>>4].set_slc(32*(3-((place>>2) & 0x3)), valueAsAcInt);
}
#endif


/***********************************************************************
 * These three procedure are used to read an int inside a memory.
 * The addressing is made on byte.
 *
 * The input memory can be uint8*, uint32* or uint128*
 ***********************************************************************/

//uint32 readInt(uint8* bytecode, int place){
//
//	unsigned int result = 0;
//	//FIXME endianness
//	result = (bytecode[place+3] << 24);
//	result += (bytecode[place+2] << 16);
//	result += (bytecode[place+1] << 8);
//	result += (bytecode[place+0] << 0);
//
//	return result;
//
//}

uint32 readInt(uint32* bytecode, int place){
	return bytecode[(place>>2)/*+(3-(place&0x3))*/];
}

uint8 readChar(uint32* bytecode, int place){
	fprintf(stderr, "reading char : %x %d -> %u\n", bytecode[place>>2], 8*(3-(place & 0x3)), bytecode[(place>>2)].slc<8>(8*(3-(place & 0x3))));
	return bytecode[(place>>2)].slc<8>(8*(3-(place & 0x3)));
}

#ifndef __NIOS
uint32 readInt(uint128* bytecode, int place){
	return bytecode[place>>4].slc<32>(32*(3-((place>>2) & 0x3)));

}
#endif
