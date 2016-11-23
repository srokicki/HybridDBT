


#ifndef __FRONTEND
#define __FRONTEND


int firstPassTranslator(unsigned char* code, unsigned int *size, unsigned int addressStart,
		unsigned char* destinationBinaries, unsigned int *placeCode,
		unsigned int &numberInsertions, int *insertions,
		short* blocksBoundaries, short* proceduresBoundaries);
int registerAllocation(unsigned char *bytecode, int numberBlocks, struct blockHeader *blockTable, unsigned long long *registersUsage, int globalVariables[32][33]);
int generateRenamedBytecode(unsigned char* code, unsigned int *size, unsigned int addressStart,
		unsigned char* bytecode, unsigned int *placeCode,
		short* blocksBoundaries, short* proceduresBoundaries);

extern unsigned int returnedValuesBytecodeForSim[1024];

#endif


