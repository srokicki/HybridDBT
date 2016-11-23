#include <stdio.h>
#include <stdlib.h>
#include <lib/tools.h>
#include <lib/ac_int.h>

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
	printf("/**********************************************/ \n");
	printf("/*  Extraction of code from %s \n", argv[1]);
	printf("/**********************************************/ \n");

	void* bytecode = openReadFile(argv[1]);
	struct bytecodeHeader* bytecodeHeader = (struct bytecodeHeader*) readFile((void*) bytecode, 0, sizeof(struct bytecodeHeader), 1);

	int bbcnt = 0;

	printf("Bytecode contains %d procedure(s).\nProcedure table starts at offset %x in binary file.\n\n", bytecodeHeader->functionTableSize, bytecodeHeader->functionTablePtr);


	unsigned char *bytecodeTest = (unsigned char*) bytecode;

	for(int procedureNumber = 0; procedureNumber < bytecodeHeader->functionTableSize; procedureNumber++){
		printf("/****************************************/ \n");
		printf("/*  Disassembling procedure %d \n", procedureNumber);
		printf("/****************************************/ \n");
		//Reading the function header
		struct functionHeader* functionHeader = (struct functionHeader*) readFile((void*) bytecode, bytecodeHeader->functionTablePtr + procedureNumber * sizeof(struct functionHeader), sizeof(struct functionHeader), 1);

		//Reading block headers
		struct blockHeader* basicBlockHeaders = (struct blockHeader*) readFile((void*) bytecode, functionHeader->blockTablePtr, sizeof(struct blockHeader), functionHeader->nbrBlock);

		//Allocating a table for block successors (TODO is it needed in nios ?)
		struct successor** blocksSuccessors = (struct successor**) malloc(functionHeader->nbrBlock * sizeof(struct successor *));

		//****************************************************************************************************************
		// Then we compile every trace, ignoring the control flow

		printf("Procedure contains %d basic blocks : \n",functionHeader->nbrBlock);

		for (int basicBlockNumber=0; basicBlockNumber<functionHeader->nbrBlock; basicBlockNumber++){

			struct blockHeader blockHeader = basicBlockHeaders[basicBlockNumber];

			printf("\nBlock %d\n", basicBlockNumber);

			int* code = (int*) readFile(bytecode, blockHeader.placeInstr, sizeof(int), blockHeader.nbrInstr*4);

			for (int oneInstruction = 0; oneInstruction < blockHeader.nbrInstr; oneInstruction++){

				ac_int<32, false> instructionPart1 = code[4*oneInstruction+0];
				ac_int<32, false> instructionPart2 = code[4*oneInstruction+1];
				ac_int<32, false> instructionPart3 = code[4*oneInstruction+2];
				ac_int<32, false> instructionPart4 = code[4*oneInstruction+3];



				ac_int<2, false> stageCode = instructionPart1.slc<2>(30);
				ac_int<2, false> typeCode = instructionPart1.slc<2>(28);
				ac_int<1, false> alloc = instructionPart1[27];
				ac_int<1, false> allocBr = instructionPart1[26];
				ac_int<7, false> opCode = instructionPart1.slc<7>(19);
				ac_int<1, false> isImm = instructionPart1[18];
				ac_int<1, false> isBr = instructionPart1[17];
				ac_int<9, false> virtualRDest = instructionPart2.slc<9>(14);
				ac_int<9, false> virtualRIn2 = instructionPart2.slc<9>(23);
				ac_int<9, false> virtualRIn1_imm9 = instructionPart1.slc<9>(0);
				ac_int<11, false> imm11 = instructionPart1.slc<11>(0);
				ac_int<19, false> imm19 = 0;
				imm19.set_slc(0, instructionPart2.slc<9>(23));
				imm19.set_slc(9, instructionPart1.slc<10>(0));
				ac_int<9, false> brCode = instructionPart1.slc<9>(9);

				ac_int<8, false> nbDep = instructionPart2.slc<8>(6);
				ac_int<3, false> nbSucc = instructionPart2.slc<3>(3);
				ac_int<3, false> nbDSucc = instructionPart2.slc<3>(0);

				printf("%d : ", oneInstruction);

				if (typeCode == 0){
					//R type
					printf("op=%x, r%d = r%d, ", (int) opCode, (int) virtualRDest, (int) virtualRIn2);
					if (isImm)
						printf("%d ", (int) imm11);
					else
						printf("r%d ", (int) virtualRIn1_imm9);


				}
				else if (typeCode == 1){
					//Rext Type
				}
				else {
					//I type
					printf("op=%x, r%d %d, ", (int) opCode, (int) virtualRDest, (int) imm19);

				}

				printf("nbDep=%d, nbDSucc = %d, nbSucc = %d, ", (int) nbDep, (int) nbSucc, (int) nbDSucc);

				printf("%x %x    alloc=%d\n", (int)instructionPart3, (int)instructionPart4, (int) alloc);

//				printf("%x %x %x %x\n", code[4*oneInstruction+0], code[4*oneInstruction+1], code[4*oneInstruction+2], code[4*oneInstruction+3]);
			}


				printf("Successors are blocks ");

				int placeOfSucc = blockHeader.placeOfSucc;
	//			placeOfSucc = FIX_INT(placeOfSucc);

				unsigned char realNumberOfSuccessor = blockHeader.numberSucc;
				if (realNumberOfSuccessor==0xff)
					realNumberOfSuccessor=1;

				printf("Place of succ is %x\n", placeOfSucc);
				struct successor* successors = (struct successor*) readFile((void*) bytecode, blockHeader.placeOfSucc, sizeof(struct successor), realNumberOfSuccessor);

				int succ;
				for (succ = 0; succ<realNumberOfSuccessor; succ++){

					printf("state %d : ", successors[succ].state);
					if (successors[succ].state == 255){
						//We are solving the case of a call block
						int dest = successors[succ].callDest;
						dest = FIX_INT(dest);
						printf("procedure %x ", dest);

					}
					else if (successors[succ].state == 254){
						short dest = successors[succ].dest;
						dest = FIX_SHORT(dest);
						printf(" implicit %x (state = %d) ", dest, successors[succ].state);
					}
					else{
						short dest = successors[succ].dest;
						dest = FIX_SHORT(dest);
						printf("%x (state %d) ", dest, successors[succ].state);
					}
				}
			printf(" \n");


		}
	}
}
