#include <types.h>


/***********************************************************************
 * These three procedure are used to write an int inside a memory.
 * The addressing is made on byte.
 *
 * The input memory can be uint8*, uint32* or uint128*
 ***********************************************************************/

unsigned int endiannessMask[4] = {0xffffff, 0xff00ffff, 0xffff00ff, 0xffffff00};

void write128(unsigned int *bytecode, int place, struct uint128_struct value){
	bytecode[(place>>2)+0] = value.word96;
	bytecode[(place>>2)+1] = value.word64;
	bytecode[(place>>2)+2] = value.word32;
	bytecode[(place>>2)+3] = value.word0;

}

void writeInt(uint8* bytecode, int place, unsigned int value){
	unsigned int *bytecodeAsInt = (unsigned int *) bytecode;
	place = place >> 2;

	//FIXME endianness
	bytecode[place+3] = (value >> 24) & 0xff;
	bytecode[place+2] = (value >> 16) & 0xff;
	bytecode[place+1] = (value >> 8) & 0xff;
	bytecode[place+0] = (value >> 0) & 0xff;

}

void writeInt(ac_int<32, false>* bytecode, int place, unsigned int value){
	bytecode[(place>>2)/*+(3-(place&0x3))*/] = value;
}

void writeChar(unsigned int* bytecode, int place, unsigned char value){

	unsigned int longValue = value;

	unsigned int bytecodeValue = bytecode[(place>>2)];
	bytecodeValue = bytecodeValue & endiannessMask[place % 4];
	bytecodeValue = bytecodeValue | (longValue<<(8*(3-(place & 0x3))));
	bytecode[(place>>2)] = bytecodeValue;

}

void writeChar(ac_int<32, false>* bytecode, int place, unsigned char value){
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
	return bytecode[(place>>2)].slc<8>(8*(3-(place & 0x3)));
}

#ifndef __NIOS
uint32 readInt(uint128* bytecode, int place){
	return bytecode[place>>4].slc<32>(32*(3-((place>>2) & 0x3)));

}

void writeInt(unsigned char* bytecode, int place, unsigned int value){
	unsigned int *bytecodeAsInt = (unsigned int *) bytecode;
	place = place >> 2;

	//FIXME endianness
	bytecode[place+3] = (value >> 24) & 0xff;
	bytecode[place+2] = (value >> 16) & 0xff;
	bytecode[place+1] = (value >> 8) & 0xff;
	bytecode[place+0] = (value >> 0) & 0xff;

}

void writeInt(unsigned int* bytecode, int place, unsigned int value){
	bytecode[(place>>2)] = value;
}


unsigned int readInt(unsigned int* bytecode, int place){
	return bytecode[(place>>2)/*+(3-(place&0x3))*/];
}
unsigned char readChar(unsigned int* bytecode, int place){
	return bytecode[(place>>2)]>>8*(3-(place & 0x3));
}


void acintMemcpy(ac_int<128, false> *to, unsigned int *from, int sizeInByte){
	for (int oneDestValue = 0; oneDestValue < sizeInByte/16; oneDestValue++){
		ac_int<128, false> value = 0;

		to[oneDestValue].set_slc(96, ac_int<32, false>(from[4*oneDestValue+0]));
		to[oneDestValue].set_slc(64, ac_int<32, false>(from[4*oneDestValue+1]));
		to[oneDestValue].set_slc(32, ac_int<32, false>(from[4*oneDestValue+2]));
		to[oneDestValue].set_slc(0, ac_int<32, false>(from[4*oneDestValue+3]));

	}
}

void acintMemcpy(unsigned int *to, ac_int<128, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte/16; oneSourceValue++){
		ac_int<128, false> value = 0;

		writeInt(to, 16*oneSourceValue + 0, readInt(from, 16*oneSourceValue + 0));
		writeInt(to, 16*oneSourceValue + 4, readInt(from, 16*oneSourceValue + 4));
		writeInt(to, 16*oneSourceValue + 8, readInt(from, 16*oneSourceValue + 8));
		writeInt(to, 16*oneSourceValue + 12, readInt(from, 16*oneSourceValue + 12));
	}
}

void acintMemcpy(ac_int<32, false> *to, unsigned int *from, int sizeInByte){
	for (int oneDestValue = 0; oneDestValue < sizeInByte/4; oneDestValue++){
		to[oneDestValue] = from[oneDestValue];
	}
}

void acintMemcpy(unsigned int *to, ac_int<32, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte/4; oneSourceValue++){
		to[oneSourceValue] = from[oneSourceValue];
	}
}

void acintMemcpy(ac_int<32, true> *to, int *from, int sizeInByte){
	for (int oneDestValue = 0; oneDestValue < sizeInByte/4; oneDestValue++){
		to[oneDestValue] = from[oneDestValue];
	}
}

void acintMemcpy(int *to, ac_int<32, true>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte/4; oneSourceValue++){
		to[oneSourceValue] = from[oneSourceValue];
	}
}


void acintMemcpy(ac_int<8, false> *to, unsigned char *from, int sizeInByte){
	for (int oneDestValue = 0; oneDestValue < sizeInByte; oneDestValue++){
		to[oneDestValue] = from[oneDestValue];
	}
}

void acintMemcpy(unsigned char *to, ac_int<8, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte; oneSourceValue++){
		to[oneSourceValue] = from[oneSourceValue];
	}
}

void acintMemcpy(ac_int<6, false> *to, unsigned char *from, int sizeInByte){
	for (int oneDestValue = 0; oneDestValue < sizeInByte; oneDestValue++){
		to[oneDestValue] = from[oneDestValue];
	}
}

void acintMemcpy(unsigned char *to, ac_int<6, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte; oneSourceValue++){
		to[oneSourceValue] = from[oneSourceValue];
	}
}

void acintMemcpy(ac_int<1, false> *to, unsigned char *from, int sizeInByte){
	for (int oneDestValue = 0; oneDestValue < sizeInByte; oneDestValue++){
		if (from[oneDestValue])
			to[oneDestValue] = 1;
		else
			to[oneDestValue] = 0;

	}
}

void acintMemcpy(unsigned char *to, ac_int<1, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte; oneSourceValue++){
		if (from[oneSourceValue])
			to[oneSourceValue] = 1;
		else
			to[oneSourceValue] = 0;
	}
}




/*************************************************************************************
 * acintCmp function will compare two arrays, one of acint, the other of normal types
 * and send back a boolean saying if they are the same.
 *
 * These functions are used to compare sw and hw implementations.
 *************************************************************************************/




bool acintCmp(unsigned int *to, ac_int<128, false>  *from, int sizeInByte){

	for (int oneSourceValue = 0; oneSourceValue < sizeInByte/16; oneSourceValue++){
		ac_int<128, false> value = 0;
		if (to[4*oneSourceValue+0] != readInt(from, 16*oneSourceValue + 0)
				|| to[4*oneSourceValue+1] != readInt(from, 16*oneSourceValue + 4)
				|| to[4*oneSourceValue+2] != readInt(from, 16*oneSourceValue + 8)
				|| to[4*oneSourceValue+3] != readInt(from, 16*oneSourceValue + 12))
			return false;

	}
	return true;
}


bool acintCmp(unsigned int *to, ac_int<32, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte/4; oneSourceValue++){
		if (to[oneSourceValue] != from[oneSourceValue])
			return false;
	}
	return true;
}



bool acintCmp(int *to, ac_int<32, true>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte/4; oneSourceValue++){
		if (to[oneSourceValue] != from[oneSourceValue])
			return false;
	}
	return true;
}



bool acintCmp(unsigned char *to, ac_int<8, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte; oneSourceValue++){
		if (to[oneSourceValue] != from[oneSourceValue])
			return false;
	}
	return true;
}


bool acintCmp(unsigned char *to, ac_int<6, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte; oneSourceValue++){
		if (to[oneSourceValue] != from[oneSourceValue])
			return false;
	}
	return true;
}



bool acintCmp(unsigned char *to, ac_int<1, false>  *from, int sizeInByte){
	for (int oneSourceValue = 0; oneSourceValue < sizeInByte; oneSourceValue++){
		if (from[oneSourceValue]){
			if (!to[oneSourceValue])
				return false;
		}
		else
			if (to[oneSourceValue])
				return false;
	}
	return true;
}








#endif
