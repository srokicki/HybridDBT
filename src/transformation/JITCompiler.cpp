#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************
 * Importing other files :
 * 	- listSchedulingunrolling is the scheduler
 * 	- classicJIT.cpp is read/write file specific to a Linux run
 *******************************************************************/
#include <types.h>
#include <backend.h>
#include "../../includes/lib/tools.h"

#define LINUX_VERSION

#define ISSUE_WIDTH 4
int NUMBER_REG = 64	;

#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#define SWAP_4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )
#define FIX_SHORT(x) (x) = SWAP_2(x)
#define FIX_INT(x)   (x)   = SWAP_4(x)





	void print8hex(int value){
		if (value >> 28 == 0)
			printf("0");
		if (value >> 24 == 0)
			printf("0");
		if (value >> 20 == 0)
			printf("0");
		if (value >> 16 == 0)
			printf("0");
		if (value >> 12 == 0)
			printf("0");
		if (value >> 8 == 0)
			printf("0");
		if (value >> 4 == 0)
			printf("0");
	    printf("%x", value);
	}

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


/**
 * Function JITCompilation handle the compilation of procedures in the bytecode
 * Arguments are paths to the bytecode and binaries files
 */
int JITCompilation(void* bytecodePath, void* binariesPath){


	uint6 freeRegisters[64];
	uint6 numberOfFreeRegisters;

	startPerformances(0);

	uint32 finalBytecode[4*65536];
	finalBytecode[0] = 0x747fffd8;
	finalBytecode[1] = 0;
	finalBytecode[2] = 0;
	finalBytecode[3] = 0;
	finalBytecode[4] = 0x2a3;
	finalBytecode[5] = 0;
	finalBytecode[6] = 0;
	finalBytecode[7] = 0;
	finalBytecode[8] = 0;
	finalBytecode[9] = 0;
	finalBytecode[10] = 0;
	finalBytecode[11] = 0;
	finalBytecode[12] = 0x2f;
	finalBytecode[13] = 0;
	finalBytecode[14] = 0;
	finalBytecode[15] = 0;
	finalBytecode[16] = 0;
	finalBytecode[17] = 0;
	finalBytecode[18] = 0;
	finalBytecode[19] = 0;
	int writeInFinalBytecode = 20;
	int vliwWidth = ISSUE_WIDTH;

	int addressOfBasicBlocks[256];
	int endOfBasicBlocks[256];
	int basicBlockNumber;
	char compiledProcedures[256];
	int callPlaces[256][16];
	char numberUnresolvedCall[256];
	int placeOfProcedures[256];
	uint6 global[512];


	void* output = openWriteFile(binariesPath);
	void* bytecode = openReadFile(bytecodePath);

	//Reading bytecode header
	struct bytecodeHeader* bytecodeHeader = (struct bytecodeHeader*) readFile((void*) bytecode, 0, sizeof(struct bytecodeHeader), 1);

	int bbcnt = 0;


	int procedureNumber;
	for(procedureNumber = 0; procedureNumber < bytecodeHeader->functionTableSize; procedureNumber++){
		numberUnresolvedCall[procedureNumber] = 0;
	}


	unsigned char *bytecodeTest = (unsigned char*) bytecode;

	for(procedureNumber = 0; procedureNumber < bytecodeHeader->functionTableSize; procedureNumber++){

		//Reading the function header
		struct functionHeader* functionHeader = (struct functionHeader*) readFile((void*) bytecode, bytecodeHeader->functionTablePtr + procedureNumber * sizeof(struct functionHeader), sizeof(struct functionHeader), 1);

		//Reading block headers
		struct blockHeader* basicBlockHeaders = (struct blockHeader*) readFile((void*) bytecode, functionHeader->blockTablePtr, sizeof(struct blockHeader), functionHeader->nbrBlock);

		//Allocating a table for block successors (TODO is it needed in nios ?)
		struct successor** blocksSuccessors = (struct successor**) malloc(functionHeader->nbrBlock * sizeof(struct successor *));

		placeOfProcedures[procedureNumber] = writeInFinalBytecode/vliwWidth;

		//**********************************************************************************************************
		// The first step of the compilation is to handle global variables by allocating them registers

		int registerIdent = 0;



		int globalVariableNumber;
		for (globalVariableNumber=0; globalVariableNumber < functionHeader->nbrGlobalVariable; globalVariableNumber++){
			global[256+globalVariableNumber] = registerIdent;
			registerIdent++;
		}

		global[511] = 63;




		numberOfFreeRegisters = NUMBER_REG - registerIdent;

		//****************************************************************************************************************
		// Then we compile every trace, ignoring the control flow


		for (basicBlockNumber=0; basicBlockNumber<functionHeader->nbrBlock; basicBlockNumber++){

			struct blockHeader blockHeader = basicBlockHeaders[basicBlockNumber];
			int i;

			uint32 localBytecode[4*256];
			uint32 localBinaries[4*512];
			uint32 placeOfInstr[256];

			//TODO: uncomment following lines
//			//Once every global variables is allocated, we generate the new free registers list
			int registerNumber;
			for(registerNumber = 0; registerNumber<NUMBER_REG-registerIdent-1; registerNumber++) //The minus 1 is for r63
				freeRegisters[registerNumber] = registerNumber + registerIdent;

			int* code = (int*) readFile(bytecode, blockHeader.placeInstr, sizeof(int), blockHeader.nbrInstr*4);
			//copyMem(localBytecode, code, 4*4*blockHeader.nbrInstr); //TODO memcopy is not used here because of the ac_int bit format
			for (i = 0; i<4*blockHeader.nbrInstr; i++){
				localBytecode[i] = code[i];
			}

//			printf("block size is %d\n",blockHeader.nbrInstr);
//			for (i = 0; i<4*blockHeader.nbrInstr; i++){
//				if (i%4==0)
//					printf("\n");
//				printf("0x%xl,",(int)localBytecode[i]);
//			}

			startPerformances(1);
			unsigned int cycle = genericScheduling(1, blockHeader.nbrInstr, localBytecode, localBinaries, global, numberOfFreeRegisters, freeRegisters, ISSUE_WIDTH, 0xadb4,placeOfInstr);
			stopPerformances(1);

			cycle = cycle & 0x7fffffff;

//			printf("Size is %d\n",cycle);

			addressOfBasicBlocks[basicBlockNumber] = writeInFinalBytecode/vliwWidth;
			for (i=0; i<(cycle+1)*vliwWidth; i++){
				finalBytecode[writeInFinalBytecode] = localBinaries[i];
				writeInFinalBytecode++;
			}
			endOfBasicBlocks[basicBlockNumber] = writeInFinalBytecode - vliwWidth;

			//This phase handle the control flow instruction : we will check the number of successor of the current basic block and if this one is greater than 1, then
			// we will need to add some nop instruction to wait for the jump to terminate (pipeline hazard)

			int placeOfSucc = blockHeader.placeOfSucc;
			placeOfSucc = FIX_INT(placeOfSucc);

			char numberOfSuccessor = blockHeader.numberSucc;

			struct successor* successors = (struct successor*) readFile((void*) bytecode, blockHeader.placeOfSucc, sizeof(struct successor), numberOfSuccessor);
			blocksSuccessors[basicBlockNumber] = successors;



			int succ;
			for (succ = 0; succ<numberOfSuccessor; succ++){
				short dest = successors[succ].dest;
				dest = FIX_SHORT(dest);

				unsigned int callDest = successors[succ].callDest;
				//callDest = FIX_INT(callDest); TODO : why fix not needed here ?


				if (blockHeader.numberSucc!=255){
					short place = placeOfInstr[successors[succ].instrNumber];

					if (successors[succ].state == 255)
						successors[succ].dest = FIX_SHORT(place);
					else
						successors[succ].callDest = place;

				}
			}

			if (blockHeader.numberSucc == 2 || blockHeader.numberSucc == 255 || (blockHeader.numberSucc == 1 && successors[0].dest != basicBlockNumber + 1))
				writeInFinalBytecode += vliwWidth;
			else
				writeInFinalBytecode += vliwWidth;

		}

		//****************************************************************************************************************
		//When every basic block code is generated, we handle the control flow by modifying the branch instructions

		for (basicBlockNumber=0; basicBlockNumber<functionHeader->nbrBlock; basicBlockNumber++){
			struct blockHeader blockHeader = basicBlockHeaders[basicBlockNumber];

			int i;

			int placeOfSucc = blockHeader.placeOfSucc;
			placeOfSucc = FIX_INT(placeOfSucc);

			unsigned char realNumberOfSuccessor = blockHeader.numberSucc;
			if (realNumberOfSuccessor==0xff)
				realNumberOfSuccessor=1;

			struct successor* successors = blocksSuccessors[basicBlockNumber];

			int succ;
			for (succ = 0; succ<realNumberOfSuccessor; succ++){


				unsigned int dest = successors[succ].dest;
				dest = FIX_SHORT(dest);

				unsigned int functionNumber = successors[succ].callDest;

				if (successors[succ].state == 255){
					//We are solving the case of a call block

					//TODO gérer le cas ou le call n'est pas la dernière instruction du bloc en faisant comme pour les jump.
					// Normalement tout est déjà pret : il ne reste qu'à modifier cet endroit en utilisant l'emplacement de l'instr qui est dans successors[succ].dest
					functionNumber = FIX_INT(functionNumber);

					if (compiledProcedures[functionNumber] == 1)
						finalBytecode[endOfBasicBlocks[basicBlockNumber]] += (placeOfProcedures[functionNumber] << 7);
					else{
						callPlaces[functionNumber][numberUnresolvedCall[functionNumber]] = endOfBasicBlocks[basicBlockNumber];
						numberUnresolvedCall[functionNumber]++;
					}
				}
				else if (successors[succ].state != 254 && functionNumber == 0 && dest != basicBlockNumber+1){
					finalBytecode[endOfBasicBlocks[basicBlockNumber]] += (addressOfBasicBlocks[dest] << 7);
				}
				else if (successors[succ].state != 254 && functionNumber != 0){
					short place = functionNumber;
					if (succ == 0 & realNumberOfSuccessor == 2){

						finalBytecode[(addressOfBasicBlocks[basicBlockNumber]<<2)+(place)] += (((addressOfBasicBlocks[dest] - ((place>>2) + addressOfBasicBlocks[basicBlockNumber])) & 0x7ffff) << 7); //TODO retrouver la place du branch
					}
					else if ((succ & 0x1) != 1){

						finalBytecode[(addressOfBasicBlocks[basicBlockNumber]<<2)+(place)] += (addressOfBasicBlocks[dest] << 7); //TODO retrouver la place du branch

					}
				}
			}

		}

		int unresolvedCallNumber;
		for (unresolvedCallNumber = 0; unresolvedCallNumber < numberUnresolvedCall[procedureNumber]; unresolvedCallNumber++){
			int address = placeOfProcedures[procedureNumber];
			finalBytecode[callPlaces[procedureNumber][unresolvedCallNumber]] += (address << 7);
		}

		compiledProcedures[procedureNumber] = 1;

	}

	stopPerformances(0);


	int i;		  int j;
	int cycleNumber = 0;
	int localIssueWidth = 1+ISSUE_WIDTH/4;
	if (localIssueWidth == 3)
		localIssueWidth = 2;
	if (localIssueWidth > 4)
		localIssueWidth = 4;


	#ifdef __LINUX_API
	int* newFinalBinaries = (int*) malloc(writeInFinalBytecode * sizeof(int));
	for (int instruction = 0; instruction < writeInFinalBytecode; instruction++){
		newFinalBinaries[instruction] = (int) finalBytecode[instruction];
	}
	#endif

	#ifndef __LINUX_API
	uint32* newFinalBinaries = finalBytecode;
	#endif

	writeFile(output, 0, 4, 1, (void*) &writeInFinalBytecode);
	writeFile(output, 4, 4, writeInFinalBytecode, (void*) newFinalBinaries);

	#ifdef __LINUX_API
	free(newFinalBinaries);
	#endif

	//recopy of datas
	char* datas = (char*) readFile((void*) bytecode, bytecodeHeader->symbolTablePtr, sizeof(char), bytecodeHeader->symbolTableSize);
	writeFile(output, 4*writeInFinalBytecode + 4, 4, 1, (void*) &bytecodeHeader->symbolTableSize);
	writeFile(output, 4*writeInFinalBytecode + 8, sizeof(char), bytecodeHeader->symbolTableSize, (void*) datas);

	//fprintf(output,"#include \"ac_int.h\"\n\n ac_int<32, false> binaries[65536] = {	");
//	for (i = 0; i<writeInFinalBytecode; i+=1){
//		if(i%ISSUE_WIDTH == 0){
//			printf("\n");
//			cycleNumber++;
//		}
//
//	  printf("0x%xl, ", (int)finalBytecode[i]);
//	  int value = (int) finalBytecode[i];
//
//	 printf(" ");
//	}
//	fprintf(output,"0};");
	//printf("\nScheduled in %d cycles\n", cycleNumber);
return 0;
}

