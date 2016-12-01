#include <lib/endianness.h>
#include <lib/ac_int.h>
#include <types.h>


/*
 * Two procedures to write inside memory in correct endianness
 * Note: address given is based on byte
 */
void writeInt(unsigned char* bytecode, int place, unsigned int value){
	unsigned int *bytecodeAsInt = (unsigned int *) bytecode;
	//bytecodeAsInt[place>>2] = value;

	//FIXME endianness
	bytecode[place+3] = (value >> 24) & 0xff;
	bytecode[place+2] = (value >> 16) & 0xff;
	bytecode[place+1] = (value >> 8) & 0xff;
	bytecode[place+0] = (value >> 0) & 0xff;

}

void writeInt(ac_int<128,false>* bytecode, int place, unsigned int value){

	ac_int<32, false> valueAsAcInt = value;
	bytecode[place>>4].set_slc(32*(3-((place>>2) & 0x3)), valueAsAcInt);
}

unsigned int readInt(unsigned char* bytecode, int place){

	unsigned int result = 0;
	//FIXME endianness
	result = (bytecode[place+3] << 24);
	result += (bytecode[place+2] << 16);
	result += (bytecode[place+1] << 8);
	result += (bytecode[place+0] << 0);

	return result;

}

uint32 readInt(uint128* bytecode, int place){
	return bytecode[place>>4].slc<32>(32*(3-((place>>2) & 0x3)));

}
