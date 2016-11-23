#include <stdio.h>

#include <tools.h>
#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#define SWAP_4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )
#define FIX_SHORT(x) (x) = SWAP_2(x)
#define FIX_INT(x)   (x)   = SWAP_4(x)


	struct bytecodeHeader {
		unsigned int functionTableSize;
		unsigned int functionTablePtr;
		unsigned int symbolTableSize;
		unsigned int symbolTablePtr;
	};

	struct functionHeader {
		unsigned char nbrGlobalVariable;
		unsigned char dummy;
		unsigned short nbrBlock;
		unsigned int blockTablePtr;
	};

	struct blockHeader {
		unsigned char nbrInstr;
		unsigned char numberSucc;
		unsigned char numberGR;
		unsigned char numberGW;
		unsigned int placeOfSucc;
		unsigned int placeInstr;
		unsigned int placeG;
	};

	struct successor {
		unsigned char instrNumber;
		unsigned char state;
		unsigned short dest;
		unsigned int callDest;
	};

int main(int argc, char *argv[])
{


	void* bytecode = openReadFile(argv[1]);
	struct bytecodeHeader* bytecodeHeader = (struct bytecodeHeader*) readFile((void*) bytecode, 0, sizeof(struct bytecodeHeader), 1);

	int bbcnt = 0;



	unsigned char *bytecodeTest = (unsigned char*) bytecode;

	printf("\n unsigned int returnedValuesBytecodeForSim[1024] = {");
	for(int procedureNumber = 0; procedureNumber < bytecodeHeader->functionTableSize; procedureNumber++){

		//Reading the function header
		struct functionHeader* functionHeader = (struct functionHeader*) readFile((void*) bytecode, bytecodeHeader->functionTablePtr + procedureNumber * sizeof(struct functionHeader), sizeof(struct functionHeader), 1);

		//Reading block headers
		struct blockHeader* basicBlockHeaders = (struct blockHeader*) readFile((void*) bytecode, functionHeader->blockTablePtr, sizeof(struct blockHeader), functionHeader->nbrBlock);

		//Allocating a table for block successors (TODO is it needed in nios ?)
		struct successor** blocksSuccessors = (struct successor**) malloc(functionHeader->nbrBlock * sizeof(struct successor *));

		//****************************************************************************************************************
		// Then we compile every trace, ignoring the control flow


		int nbrBlock = functionHeader->nbrBlock;
		int totalSize = 0;

		for (int basicBlockNumber=0; basicBlockNumber<functionHeader->nbrBlock; basicBlockNumber++){

			struct blockHeader blockHeader = basicBlockHeaders[basicBlockNumber];

			totalSize += blockHeader.nbrInstr;

		}
		printf("%d,", (nbrBlock<<16) + totalSize);
	}
	printf("0};\n");
}
