//#include <cstdio>
//#include <cstdlib>
//
//#include <ElfFile.h>
//#include <frontend.h>
//#include <tools.h>
//#include <mips.h>
//#include <endianness.h>
//
////#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
////#define SWAP_4(x) ( (x << 24) | \
////         ((x << 8) & 0x00ff0000) | \
////         ((x >> 8) & 0x0000ff00) | \
////         (x >> 24) )
////#define FIX_SHORT(x) (x) = SWAP_2(x)
////#define FIX_INT(x)   (x)   = SWAP_4(x)
//
//
//	struct bytecodeHeader {
//		unsigned int functionTableSize;
//		unsigned int functionTablePtr;
//		unsigned int symbolTableSize;
//		unsigned int symbolTablePtr;
//	};
//
//	struct functionHeader {
//		unsigned char nbrGlobalVariable;
//		unsigned char dummy;
//		unsigned short nbrBlock;
//		unsigned int blockTablePtr;
//	};
//
//	struct blockHeader {
//		unsigned char nbrInstr;
//		unsigned char numberSucc;
//		unsigned char numberGR;
//		unsigned char numberGW;
//		unsigned int placeOfSucc;
//		unsigned int placeInstr;
//		unsigned int placeG;
//	};
//
//	struct successor {
//		unsigned char instrNumber;
//		unsigned char state;
//		unsigned short dest;
//		unsigned int callDest;
//	};
//
////#define __DEBUG
//
//
//using namespace std;
//
//const unsigned int debugLevel = 0;
//
//#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
//#define SWAP_4(x) ( (x << 24) | \
//         ((x << 8) & 0x00ff0000) | \
//         ((x >> 8) & 0x0000ff00) | \
//         (x >> 24) )
//#define FIX_SHORT(x) (x) = SWAP_2(x)
//#define FIX_INT(x)   (x)   = SWAP_4(x)
//
//#ifdef __USE_AC
//unsigned int DBTFrontend_loop(ac_int<128, false> code[1024], unsigned int procedureSize, unsigned int addressStart,
//		ac_int<128, false> bytecode_code[1024], unsigned char bytecode_blocks[512], unsigned char bytecode_succ[512],
//		unsigned int indexStart, short blocksBoundaries[1024], short proceduresBoundaries[1024],int globalVariables[32][33],
//		ac_int<64, false> registersUsage[256], int globalVariableCounter);
//#endif
//char visitBlocks(unsigned char *bytecode, struct blockHeader *blockTable, char* blockVisited, unsigned char* isAlive, int currentBlock,
//		int sourceBlock, int registerNumber, unsigned long long *registersUsage, int globalVariables[32][33]);
//
//
//
//char visitBlocks(unsigned char *bytecode, struct blockHeader *blockTable, char* blockVisited, unsigned char* isAlive, int currentBlock,
//		int sourceBlock, int registerNumber, unsigned long long *registersUsage, int globalVariables[32][33]){
//	char result = 0;
//
////	printf("Visiting %d\n", currentBlock);
//
//
//	if (globalVariables[currentBlock][registerNumber] != -1){
////		printf("Yeah !\n");
//
//		result = 1;
//		isAlive[(((sourceBlock<<5) + registerNumber)<<5) + currentBlock] = 1;
//	}
//
//	if (blockTable[currentBlock].numberSucc != 0){
//		if (blockVisited[currentBlock] == 0){
//			if (currentBlock == sourceBlock || !((registersUsage[currentBlock]>>registerNumber) & 1 )){
//				char numberSuccessors = blockTable[currentBlock].numberSucc;
//				int placeSucc = blockTable[currentBlock].placeOfSucc;
//				//placeSucc = FIX_INT(placeSucc);
//				struct successor *listSuccessors = (struct successor *) &(bytecode[placeSucc]);
//
//				if (!result)
//					blockVisited[currentBlock] = -1;
//				else
//					blockVisited[currentBlock] = 1;
//
//
//				int numberUnsolvedSuccessors = -1;
//				int oldNumberUnsolvedSuccessors = 0;
//
//
//				do {
////					printf("Number unresolved register is not null (%d - %d)\n", numberUnsolvedSuccessors, numberSuccessors);
//					oldNumberUnsolvedSuccessors = numberUnsolvedSuccessors;
//					numberUnsolvedSuccessors = 0;
//					for (int oneSuccessor = 0; oneSuccessor < numberSuccessors; oneSuccessor++){
//						if (listSuccessors[oneSuccessor].state != 255){
//							//If successor is not a call
//
//							short destination = listSuccessors[oneSuccessor].dest;
//							destination = FIX_SHORT(destination);
//							char subResult = visitBlocks(bytecode, blockTable, blockVisited, isAlive, destination, sourceBlock, registerNumber, registersUsage, globalVariables);
//
//							if (subResult == -1)
//								numberUnsolvedSuccessors++;
//							else{
//								isAlive[(((sourceBlock<<5) + registerNumber)<<5) + currentBlock] |= subResult;
//								blockVisited[currentBlock] = 1;
//							}
//
//							result = result | (subResult == 1);
//						}
//					}
//				}
//				while (numberUnsolvedSuccessors != 0 && numberUnsolvedSuccessors != numberSuccessors && oldNumberUnsolvedSuccessors != numberUnsolvedSuccessors);
//
//				if ((numberUnsolvedSuccessors == numberSuccessors || oldNumberUnsolvedSuccessors == numberUnsolvedSuccessors) && !result){
//					blockVisited[currentBlock] = 0;
////					printf("Returning bis %d from %d\n", -1, currentBlock);
//					return -1;
//				}
//			}
//			else{
////				printf("No eligible succ from %d\n", currentBlock);
//			}
//		}
//		else if (blockVisited[currentBlock] == -1){
////			printf("Unresolved %d!\n", currentBlock);
//			return -1;
//		}
//		else{
////			printf("Returning bis %d from %d\n", result, currentBlock);
//			return result;
//
//		}
//	}
//	if (result){
//		isAlive[(((sourceBlock<<5) + registerNumber)<<5) + currentBlock] = 1;
////		printf("Reg %d of block %d is alive in %d\n", registerNumber, sourceBlock, currentBlock);
//	}
//	else{
//		isAlive[(((sourceBlock<<5) + registerNumber)<<5) + currentBlock] = 0;
//	}
//	blockVisited[currentBlock] = 1;
//
////	printf("Returning %d from %d\n", result, currentBlock);
//	return result;
//
//
//}
//
//int registerAllocation(unsigned char *bytecode, int numberBlocks, struct blockHeader *blockTable, unsigned long long *registersUsage, int globalVariables[32][33]){
//
////	blockTable[numberBlocks].placeOfSucc = blockTable[numberBlocks-1].placeOfSucc + 8*blockTable[numberBlocks-1].numberSucc;
////	blockTable[numberBlocks].numberSucc = 1;
////	successor *oneSuccessor = (successor *) &(bytecode[blockTable[numberBlocks].placeOfSucc]);
////	oneSuccessor->dest = 0;
////	long long *fakeUsage = (long long *)registersUsage;
////	fakeUsage[numberBlocks] = -1;
////	numberBlocks++;
//
//	int numberWrites = 0;
//	for (int oneBlock = 0; oneBlock < numberBlocks; oneBlock++){
//		for (int oneRegister = 8; oneRegister<16; oneRegister++){
//			if ((registersUsage[oneBlock]>>oneRegister)&0x1)
//				numberWrites++;
//		}
//
//	}
//
////	for (int oneBlock = 0; oneBlock < numberBlocks; oneBlock++){
////		printf("For %d : ", oneBlock);
////		for (int oneRegister = 8; oneRegister<16; oneRegister++){
////			printf("%d ", globalVariables[oneBlock][oneRegister] != -1);
////		}
////		printf("\n");
////		printf("For %d : ", oneBlock);
////		for (int oneRegister = 8; oneRegister<16; oneRegister++){
////			printf("%d ", (registersUsage[oneBlock]>>oneRegister) & 0x1);
////		}
////		printf("\n\n");
////
////	}
//
//	/*
//	 * First we compute the live range of all writes using a recursive function. Writes that have a liverange greater
//	 * than one will be seen as write for the following
//	 */
//	unsigned char *isAlive = (unsigned char*) malloc(32*numberBlocks*numberBlocks);
//	char *blockVisited = (char*) malloc(numberBlocks);
//	unsigned short* writeList = (unsigned short*) malloc(2*numberWrites);
//	unsigned char* writeDest = (unsigned char*) malloc(numberWrites);
//
//
//	numberWrites = 0;
//
//	for (int oneBlock = 0; oneBlock < numberBlocks; oneBlock++){
//		for (int oneRegister = 8; oneRegister<16; oneRegister++){
//			if ((registersUsage[oneBlock]>>oneRegister)&0x1){
//
//				//We initialize blockVisited with zeros
//				for (int oneOtherBlock = 0; oneOtherBlock < numberBlocks; oneOtherBlock++){
//					blockVisited[oneOtherBlock] = 0;
//					isAlive[(((oneBlock<<5) + oneRegister)<<5) + oneOtherBlock] = 0;
//				}
//
////				printf("\ndepart from %d\n", oneBlock);
//
//				isAlive[(((oneBlock<<5) + oneRegister)<<5) + oneBlock] = 1;
//				char subResult = visitBlocks(bytecode, blockTable, blockVisited, isAlive, oneBlock, oneBlock, oneRegister, registersUsage, globalVariables);
//
////				printf("%d \t\t For register %d in block %d, alive in : ", numberWrites, oneRegister, oneBlock);
////				for (int oneOtherBlock = 0; oneOtherBlock < numberBlocks; oneOtherBlock++)
////					if (isAlive[(((oneBlock<<5) + oneRegister)<<5) + oneOtherBlock])
////						printf("%d ", oneOtherBlock);
////
////				printf("\n");
//
//				if (isAlive[(((oneBlock<<5) + oneRegister)<<5) + oneBlock]){
//					writeList[numberWrites] = ((oneBlock<<5) + oneRegister)<<5;
//					numberWrites++;
//				}
//
//			}
//		}
//
//	}
//
//	/*
//	 * Then we assign values to each of them using greedy algorithm.
//	 * As we deal with writes, representation is SSA like and thus the greedy algoritm will give optimal allocation
//	 */
//
//	int registerName = 0;
//
//	for (int oneWriteValue = 0; oneWriteValue < numberWrites; oneWriteValue++){
//		char assigned = 0;
//
//		for (int onePossibleDestination = 3; onePossibleDestination < registerName; onePossibleDestination++){
//			char haveConflict = 0;
//
//
//			for (int oneOtherWriteValue = 0; oneOtherWriteValue < oneWriteValue; oneOtherWriteValue++){
//				if (writeDest[oneOtherWriteValue] == onePossibleDestination){
//
//
//					for (int oneBlock = 0; oneBlock < numberBlocks; oneBlock++){
//						if (isAlive[writeList[oneWriteValue] + oneBlock]
//									&& isAlive[writeList[oneOtherWriteValue] + oneBlock]){
//							haveConflict = 1;
//							break;
//						}
//					}
//					if (haveConflict)
//						break;
//
//				}
//			}
//
//			if (!haveConflict){
//				assigned = 1;
//				writeDest[oneWriteValue] = onePossibleDestination;
//				break;
//			}
//
//		}
//
//		if (!assigned)
//			writeDest[oneWriteValue] = registerName++;
//
//	}
//
//	/*
//	 * We fill the globalVariables information in order to use it in next generation of bytecode
//	 */
//
//	for (int oneBlock = 0; oneBlock < numberBlocks; oneBlock++){
//		for (int oneRegister = 0; oneRegister < 33; oneRegister++){
//			if (oneRegister == 0)
//				globalVariables[oneBlock][oneRegister] = 256;
//			else if (oneRegister == 1)
//				globalVariables[oneBlock][oneRegister] = 257;
//			else if (oneRegister == 2)
//				globalVariables[oneBlock][oneRegister] = 258;
//			else if (oneRegister == 3)
//				globalVariables[oneBlock][oneRegister] = 259;
//			else if (oneRegister == 4)
//				globalVariables[oneBlock][oneRegister] = 260;
//			else if (oneRegister == 5)
//				globalVariables[oneBlock][oneRegister] = 261;
//			else if (oneRegister == 6)
//				globalVariables[oneBlock][oneRegister] = 262;
//			else if (oneRegister == 7)
//				globalVariables[oneBlock][oneRegister] = 263;
//
////			else if (oneRegister == 8)
////				globalVariables[oneBlock][oneRegister] = 279;
////			else if (oneRegister == 9)
////				globalVariables[oneBlock][oneRegister] = 280;
////			else if (oneRegister == 10)
////				globalVariables[oneBlock][oneRegister] = 281;
////			else if (oneRegister == 11)
////				globalVariables[oneBlock][oneRegister] = 282;
////			else if (oneRegister == 12)
////				globalVariables[oneBlock][oneRegister] = 283;
////			else if (oneRegister == 13)
////				globalVariables[oneBlock][oneRegister] = 284;
////			else if (oneRegister == 14)
////				globalVariables[oneBlock][oneRegister] = 285;
////			else if (oneRegister == 15)
////				globalVariables[oneBlock][oneRegister] = 286;
//
//			else if (oneRegister == 16)
//				globalVariables[oneBlock][oneRegister] = 264;
//			else if (oneRegister == 17)
//				globalVariables[oneBlock][oneRegister] = 265;
//			else if (oneRegister == 18)
//				globalVariables[oneBlock][oneRegister] = 266;
//			else if (oneRegister == 19)
//				globalVariables[oneBlock][oneRegister] = 267;
//			else if (oneRegister == 20)
//				globalVariables[oneBlock][oneRegister] = 268;
//			else if (oneRegister == 21)
//				globalVariables[oneBlock][oneRegister] = 269;
//			else if (oneRegister == 22)
//				globalVariables[oneBlock][oneRegister] = 270;
//			else if (oneRegister == 23)
//				globalVariables[oneBlock][oneRegister] = 271;
//			else if (oneRegister == 24)
//				globalVariables[oneBlock][oneRegister] = 272;
//			else if (oneRegister == 25)
//				globalVariables[oneBlock][oneRegister] = 273;
//			else if (oneRegister == 26)
//				globalVariables[oneBlock][oneRegister] = 274;
//			else if (oneRegister == 27)
//				globalVariables[oneBlock][oneRegister] = 275;
//			else if (oneRegister == 28)
//				globalVariables[oneBlock][oneRegister] = 276;
//			else if (oneRegister == 29)
//				globalVariables[oneBlock][oneRegister] = 277;
//			else if (oneRegister == 30)
//				globalVariables[oneBlock][oneRegister] = 278;
//			else if (oneRegister == 31)
//				globalVariables[oneBlock][oneRegister] = 511;
//			else if (oneRegister == 32)
//				globalVariables[oneBlock][oneRegister] = 288;
//			else{
//				globalVariables[oneBlock][oneRegister] = -1;
//				for (int oneWrite = 0; oneWrite < numberWrites; oneWrite++){
//					if (((writeList[oneWrite] >> 5) & 0x1f) == oneRegister && isAlive[writeList[oneWrite] + oneBlock]){
//						globalVariables[oneBlock][oneRegister] = 279 + writeDest[oneWrite];
//						break;
//					}
//				}
//			}
//
//		}
//	}
//
////	for (int oneBlock = 0; oneBlock < numberBlocks; oneBlock++){
////		printf("Globals for block %d : ", oneBlock);
////		for (int oneRegister = 0; oneRegister < 32; oneRegister++){
////			printf("%d, ", globalVariables[oneBlock][oneRegister]);
////		}
////		printf("\n");
////	}
////	for (int oneWriteValue = 0; oneWriteValue < numberWrites; oneWriteValue++){
////		printf("value %d is assined in %d\n", oneWriteValue, writeDest[oneWriteValue]);
////	}
//
//	free(isAlive);
//	free(blockVisited);
//	free(writeList);
//	free(writeDest);
//
//	return 288 + registerName;
//
//}
//#ifdef __USE_AC
//
//int generateRenamedBytecode(unsigned char* code, unsigned int *size, unsigned int addressStart,
//		unsigned char* bytecode, unsigned int *placeCode,
//		short* blocksBoundaries, short* proceduresBoundaries){
//
////	int globalVariables[32][33] = {256,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,257,258,511, 287};
//	int globalVariables[32][33] = {256,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,257,258,511};
//	for (int i=1; i<32; i++)
//		for (int j=0; j<33; j++)
//			globalVariables[i][j] = globalVariables[0][j];
//
//	int globalVariableCounter = 259;
//	unsigned long long registersUsage[256];
//
//	printf("%d\n", *placeCode);
//
//
//
//	ac_int<32, false> localCode[1024];
//	ac_int<128, false> localBytecode_code[1024];
//	unsigned char localBytecode_block[1024];
//	unsigned char localBytecode_succ[1024];
//
//
//	/****** Metadata on procedures & blocks******/
//	int procedureNumber = 0;
//	int procedureEnds[250];
//	int procedureStarts[250];
//	procedureStarts[0] = 0;
//
//	int basicBlockNumber = 0;
//	int totalBasicBlockNumber = 0;
//
//
//	for (int oneInstructionIndex = 0; oneInstructionIndex<*size; oneInstructionIndex++){
//		if (proceduresBoundaries[oneInstructionIndex] & 0x1 == 1){
//			if (procedureNumber != 0){
//				procedureEnds[procedureNumber-1] = oneInstructionIndex;
//			}
//			procedureStarts[procedureNumber] = oneInstructionIndex;
//			procedureNumber++;
//
//			basicBlockNumber = 0;
//		}
//		if (blocksBoundaries[oneInstructionIndex] & 0x1 == 1){
//			basicBlockNumber++;
//			totalBasicBlockNumber++;
//		}
//
//	}
//	procedureEnds[procedureNumber-1] = *size;
//
//
//	/*We start generating the bytecode*/
//	bytecodeHeader *oneBytecodeHeader = (bytecodeHeader*) &(bytecode[0]);
//
//	printf("Procedure size is %d\n", procedureNumber);
//	oneBytecodeHeader->functionTableSize = procedureNumber;
//	oneBytecodeHeader->functionTablePtr = 16; // = FIX_INT(12) TODO : change for nios
//
//	//TODO : fill information on symbol table when ready
//	struct functionHeader *funtionHeaders = (struct functionHeader *) &(bytecode[16]);
//
//
//
//	//Declaration of placeInBlockTable which will keep track of the place where we write
//	int placeInBlockTable = 16 + (procedureNumber<<3);
//	int placeInBlockTableSaved = placeInBlockTable;
//	*placeCode = placeInBlockTable + (totalBasicBlockNumber << 4);
//
//
//	unsigned char numberSucc;
//	unsigned char numberGR;
//	unsigned char numberGW;
//	for (unsigned int oneProcedure = 0; oneProcedure<procedureNumber; oneProcedure++){
//
//		//**************************************************************************
//		//Procedure header is placed at 16 + oneProcedure * 8
//		//Its size is 8 bytes...
//		// 1 byte for the number of global variables
//		// 1 blank byte
//		// 2 bytes for the number of basic block
//		// 4 bytes pointing to the block table. The block table should start at 16 + procedureNumber*8
//
//		int placeInBlockTableSaved = placeInBlockTable;
//		int savedPlaceBytecode = *placeCode;
//
//		struct blockHeader *blockHeaders = (struct blockHeader *) &(bytecode[placeInBlockTable]);
//
//		ac_int<16, false> localBoundaries[1024];
//
//
//		funtionHeaders[oneProcedure].blockTablePtr = placeInBlockTable;
//		unsigned int procedureSize = procedureEnds[oneProcedure] - procedureStarts[oneProcedure];
//		for (int i = procedureStarts[oneProcedure]; i<procedureEnds[oneProcedure]; i++){
//			unsigned int oneInstruction = (code[4*i+3] << 0) + (code[4*i+2] << 8) + (code[4*i+1] << 16) + (code[4*i+0] << 24);
//			localCode[i - procedureStarts[oneProcedure]] = oneInstruction;
//			localBoundaries[i - procedureStarts[oneProcedure]] = blocksBoundaries[i];
//		}
//
//		ac_int<64, false> local_registersUsage[256];
//
//
//		unsigned int result = DBTFrontend_loop((ac_int<128, false>*) code, procedureSize, addressStart, localBytecode_code, localBytecode_block, localBytecode_succ, procedureStarts[oneProcedure], blocksBoundaries, proceduresBoundaries, globalVariables, local_registersUsage, globalVariableCounter);
//		unsigned short numberOfBlocks = result >> 16;
//		unsigned short numberBytecodeInstr = result & 0xffff;
//
//		funtionHeaders[oneProcedure].nbrBlock = numberOfBlocks;
//		funtionHeaders[oneProcedure].nbrGlobalVariable = 32;
//
//		/** Now we rebuild the bytecode **/
//		struct blockHeader *srcBlockTable = (struct blockHeader*) localBytecode_block;
//		struct blockHeader *dstBlockTable = (struct blockHeader*) &(bytecode[placeInBlockTable]); //TODO handle place in block table
//
//		//	We increment place in block table
//		placeInBlockTable += numberOfBlocks<<4;
//
//		unsigned int indexInCode = 0;
//		unsigned int indexInSucc = 0;
//
//
//		for (int oneBlock = 0; oneBlock < numberOfBlocks; oneBlock++){
//
//			registersUsage[oneBlock] = local_registersUsage[oneBlock];
//
//
//
//			dstBlockTable[oneBlock] = srcBlockTable[oneBlock];
//			dstBlockTable[oneBlock].placeInstr = *placeCode;
//			dstBlockTable[oneBlock].placeOfSucc = *placeCode + srcBlockTable[oneBlock].nbrInstr * 16;
//
//			for (int oneInstruction = 0; oneInstruction < srcBlockTable[oneBlock].nbrInstr; oneInstruction++){
//				bytecode[*placeCode + 16*oneInstruction + 0] = localBytecode_code[oneInstruction + indexInCode].slc<8>(96);
//				bytecode[*placeCode + 16*oneInstruction + 1] = localBytecode_code[oneInstruction + indexInCode].slc<8>(104);
//				bytecode[*placeCode + 16*oneInstruction + 2] = localBytecode_code[oneInstruction + indexInCode].slc<8>(112);
//				bytecode[*placeCode + 16*oneInstruction + 3] = localBytecode_code[oneInstruction + indexInCode].slc<8>(120);
//				bytecode[*placeCode + 16*oneInstruction + 4] = localBytecode_code[oneInstruction + indexInCode].slc<8>(64);
//				bytecode[*placeCode + 16*oneInstruction + 5] = localBytecode_code[oneInstruction + indexInCode].slc<8>(72);
//				bytecode[*placeCode + 16*oneInstruction + 6] = localBytecode_code[oneInstruction + indexInCode].slc<8>(80);
//				bytecode[*placeCode + 16*oneInstruction + 7] = localBytecode_code[oneInstruction + indexInCode].slc<8>(88);
//				bytecode[*placeCode + 16*oneInstruction + 8] = localBytecode_code[oneInstruction + indexInCode].slc<8>(32);
//				bytecode[*placeCode + 16*oneInstruction + 9] = localBytecode_code[oneInstruction + indexInCode].slc<8>(40);
//				bytecode[*placeCode + 16*oneInstruction + 10] = localBytecode_code[oneInstruction + indexInCode].slc<8>(48);
//				bytecode[*placeCode + 16*oneInstruction + 11] = localBytecode_code[oneInstruction + indexInCode].slc<8>(56);
//				bytecode[*placeCode + 16*oneInstruction + 12] = localBytecode_code[oneInstruction + indexInCode].slc<8>(0);
//				bytecode[*placeCode + 16*oneInstruction + 13] = localBytecode_code[oneInstruction + indexInCode].slc<8>(8);
//				bytecode[*placeCode + 16*oneInstruction + 14] = localBytecode_code[oneInstruction + indexInCode].slc<8>(16);
//				bytecode[*placeCode + 16*oneInstruction + 15] = localBytecode_code[oneInstruction + indexInCode].slc<8>(24);
//			}
//			*placeCode = *placeCode + 16*srcBlockTable[oneBlock].nbrInstr;
//
//			for (int oneByte = 0; oneByte < 8*srcBlockTable[oneBlock].numberSucc; oneByte++){
//
//				bytecode[*placeCode + oneByte] = localBytecode_succ[indexInSucc + oneByte];
//			}
//
//			*placeCode = *placeCode + 8*srcBlockTable[oneBlock].numberSucc;
//
//			indexInCode += srcBlockTable[oneBlock].nbrInstr;
//			indexInSucc += 8*srcBlockTable[oneBlock].numberSucc;
//
//
//		}
//
//
//
//		globalVariableCounter = registerAllocation(bytecode, numberOfBlocks, dstBlockTable, registersUsage, globalVariables);
//
//
//
//		placeInBlockTable = placeInBlockTableSaved;
//		*placeCode = savedPlaceBytecode;
//
//
//		funtionHeaders[oneProcedure].blockTablePtr = placeInBlockTable;
//		for (int i = procedureStarts[oneProcedure]; i<procedureEnds[oneProcedure]; i++){
//			unsigned int oneInstruction = (code[4*i+3] << 0) + (code[4*i+2] << 8) + (code[4*i+1] << 16) + (code[4*i+0] << 24);
//			localCode[i - procedureStarts[oneProcedure]] = oneInstruction;
//			localBoundaries[i - procedureStarts[oneProcedure]] = blocksBoundaries[i];
//		}
//
//		blockHeaders = (struct blockHeader *) &(bytecode[placeInBlockTable]);
//
//		result = DBTFrontend_loop((ac_int<128, false>*) code, procedureSize, addressStart, localBytecode_code, localBytecode_block, localBytecode_succ, procedureStarts[oneProcedure], blocksBoundaries, proceduresBoundaries, globalVariables, local_registersUsage, globalVariableCounter);
//		numberOfBlocks = result >> 16;
//		numberBytecodeInstr = result & 0xffff;
//
//		funtionHeaders[oneProcedure].nbrBlock = numberOfBlocks;
//		funtionHeaders[oneProcedure].nbrGlobalVariable = 32;
//
//		/** Now we rebuild the bytecode **/
//		srcBlockTable = (struct blockHeader*) localBytecode_block;
//		dstBlockTable = (struct blockHeader*) &(bytecode[placeInBlockTable]); //TODO handle place in block table
//
//		//	We increment place in block table
//		placeInBlockTable += numberOfBlocks<<4;
//
//		indexInCode = 0;
//		indexInSucc = 0;
//
//
//		for (int oneBlock = 0; oneBlock < numberOfBlocks; oneBlock++){
//
//			registersUsage[oneBlock] = local_registersUsage[oneBlock];
//
//
//
//			dstBlockTable[oneBlock] = srcBlockTable[oneBlock];
//			dstBlockTable[oneBlock].placeInstr = *placeCode;
//			dstBlockTable[oneBlock].placeOfSucc = *placeCode + srcBlockTable[oneBlock].nbrInstr * 16;
//
//			for (int oneInstruction = 0; oneInstruction < srcBlockTable[oneBlock].nbrInstr; oneInstruction++){
//				bytecode[*placeCode + 16*oneInstruction + 0] = localBytecode_code[oneInstruction + indexInCode].slc<8>(96);
//				bytecode[*placeCode + 16*oneInstruction + 1] = localBytecode_code[oneInstruction + indexInCode].slc<8>(104);
//				bytecode[*placeCode + 16*oneInstruction + 2] = localBytecode_code[oneInstruction + indexInCode].slc<8>(112);
//				bytecode[*placeCode + 16*oneInstruction + 3] = localBytecode_code[oneInstruction + indexInCode].slc<8>(120);
//				bytecode[*placeCode + 16*oneInstruction + 4] = localBytecode_code[oneInstruction + indexInCode].slc<8>(64);
//				bytecode[*placeCode + 16*oneInstruction + 5] = localBytecode_code[oneInstruction + indexInCode].slc<8>(72);
//				bytecode[*placeCode + 16*oneInstruction + 6] = localBytecode_code[oneInstruction + indexInCode].slc<8>(80);
//				bytecode[*placeCode + 16*oneInstruction + 7] = localBytecode_code[oneInstruction + indexInCode].slc<8>(88);
//				bytecode[*placeCode + 16*oneInstruction + 8] = localBytecode_code[oneInstruction + indexInCode].slc<8>(32);
//				bytecode[*placeCode + 16*oneInstruction + 9] = localBytecode_code[oneInstruction + indexInCode].slc<8>(40);
//				bytecode[*placeCode + 16*oneInstruction + 10] = localBytecode_code[oneInstruction + indexInCode].slc<8>(48);
//				bytecode[*placeCode + 16*oneInstruction + 11] = localBytecode_code[oneInstruction + indexInCode].slc<8>(56);
//				bytecode[*placeCode + 16*oneInstruction + 12] = localBytecode_code[oneInstruction + indexInCode].slc<8>(0);
//				bytecode[*placeCode + 16*oneInstruction + 13] = localBytecode_code[oneInstruction + indexInCode].slc<8>(8);
//				bytecode[*placeCode + 16*oneInstruction + 14] = localBytecode_code[oneInstruction + indexInCode].slc<8>(16);
//				bytecode[*placeCode + 16*oneInstruction + 15] = localBytecode_code[oneInstruction + indexInCode].slc<8>(24);
//			}
//			*placeCode = *placeCode + 16*srcBlockTable[oneBlock].nbrInstr;
//
//			for (int oneByte = 0; oneByte < 8*srcBlockTable[oneBlock].numberSucc; oneByte++){
//
//				bytecode[*placeCode + oneByte] = localBytecode_succ[indexInSucc + oneByte];
//			}
//
//			*placeCode = *placeCode + 8*srcBlockTable[oneBlock].numberSucc;
//
//			indexInCode += srcBlockTable[oneBlock].nbrInstr;
//			indexInSucc += 8*srcBlockTable[oneBlock].numberSucc;
//
//
//
//		}
//
//	}
//}
//#endif
//
//#ifndef __USE_AC
//
///* Global values */
//char rename_isOutsideNext = 0;
//char rename_droppedInstruction = 0;
//unsigned int rename_outsideNext_bytecode1;
//unsigned int rename_outsideNext_pred1_reg;
//unsigned int rename_outsideNext_pred1;
//unsigned int rename_outsideNext_pred2_reg;
//unsigned int rename_outsideNext_dest_reg;
//unsigned int rename_outsideNext_imm;
//
//unsigned int rename_outsideNext_isImm;
//unsigned int rename_outsideNext_isLongImm;
//
//unsigned char rename_outsideNext_pred1_ena;
//unsigned char rename_outsideNext_pred1_solved;
//unsigned char rename_outsideNext_pred2_ena;
//unsigned char rename_outsideNext_dest_ena;
//unsigned char rename_outsideNext_dest_alloc;
//
//
//inline void writeShort(unsigned char* bytecode, int place, unsigned short value){
//	unsigned short *bytecodeAsShort = (unsigned short *) bytecode;
//	//bytecodeAsShort[place>>2] = value;
//
//	//FIXME endianness
//	bytecode[place+1] = (value >> 8) & 0xff;
//	bytecode[place+0] = (value >> 0) & 0xff;
//
//}
//
//inline void writeSuccessor(unsigned char *bytecode, unsigned char srcInstr, unsigned char destInstr, int offsetCode){
//	unsigned char* bytecodeAsChar = (unsigned char*) bytecode;
//	int offset[4] = {3, 1, -1, -3};
//
//	if (debugLevel >= 1)
//		printf("Writing successor from %d to %d and data is %d\n", srcInstr, destInstr, 0);
//
//	unsigned char wordWithNbDataSucc = bytecode[offsetCode + srcInstr*16 + 4]; //FIXME endianness
//	unsigned char nbSucc = wordWithNbDataSucc & 0x7;
//	unsigned char nbDSucc = (wordWithNbDataSucc >> 3) & 0x7;
//
//	if (nbSucc == 7){
//		printf("\n\n\n\t\t\t\tError, too many successors for %d, exiting... \n", srcInstr);
////		exit(-1);
//	}
//	if (nbSucc == 3){
//		//TODO
//	}
//
//	unsigned int index = offsetCode + 16*srcInstr + 8 + 1 + (6 - nbSucc) + offset[(7-nbSucc) & 0x3]; //FIXME endianness
//
//	bytecodeAsChar[index] = destInstr;
//	nbSucc++;
//
//	wordWithNbDataSucc = (wordWithNbDataSucc & 0xc0) + (nbDSucc << 3) + nbSucc;
//	bytecode[offsetCode + srcInstr*16+4] = wordWithNbDataSucc;//FIXME endianness
//}
//
//inline unsigned int writeDataSuccessor(unsigned char *bytecode, unsigned char srcInstr, unsigned char destInstr, int offsetCode){
//
//	unsigned char* bytecodeAsChar = (unsigned char*) bytecode;
//	int offset[4] = {3, 1, -1, -3};
//
//	if (debugLevel >= 1)
//		printf("Writing successor from %d to %d and data is %d\n", srcInstr, destInstr, 1);
//
//	unsigned int wordWithNbDataSucc = bytecode[offsetCode + srcInstr*16 + 4];//FIXME endianness
//	unsigned char nbSucc = wordWithNbDataSucc & 0x7;
//	unsigned char nbDSucc = (wordWithNbDataSucc >> 3) & 0x7;
//
//	if (nbSucc == 7){
//		printf("\n\n\n\t\t\t\tError, too many successors for %d, exiting... \n", srcInstr);
////		exit(-1);
//	}
//
//	unsigned int index = offsetCode + 16*srcInstr + 8 + 1 + (6-nbSucc) + offset[(7-nbSucc) & 0x3];//FIXME endianness
//
//	bytecodeAsChar[index] = destInstr;
//	nbSucc++;
//	nbDSucc++;
//
//	wordWithNbDataSucc = (wordWithNbDataSucc & 0xc0) + (nbDSucc << 3) + nbSucc;
//	bytecode[offsetCode + srcInstr*16+4] = wordWithNbDataSucc;//FIXME endianness
//
//	return nbSucc;
//}
//
//
//int generateRenamedBytecode(unsigned char* code, unsigned int *size, unsigned int addressStart,
//		unsigned char* bytecode, unsigned int *placeCode,
//		short* blocksBoundaries, short* proceduresBoundaries){
//
//	/****** Metadata on procedures & blocks******/
//	int procedureNumber = 0;
//	int procedureEnds[250];
//	int procedureStarts[250];
//	procedureStarts[0] = 0;
//
//	int basicBlockNumber = 0;
//	int totalBasicBlockNumber = 0;
//
//
//	for (int oneInstructionIndex = 0; oneInstructionIndex<*size; oneInstructionIndex++){
//		if (proceduresBoundaries[oneInstructionIndex] & 0x1 == 1){
//			if (procedureNumber != 0){
//				procedureEnds[procedureNumber-1] = oneInstructionIndex;
//			}
//			procedureStarts[procedureNumber] = oneInstructionIndex;
//			procedureNumber++;
//
//			basicBlockNumber = 0;
//		}
//		if (blocksBoundaries[oneInstructionIndex] & 0x1 == 1){
//			basicBlockNumber++;
//			totalBasicBlockNumber++;
//		}
//
//	}
//	procedureEnds[procedureNumber-1] = *size;
//
//	/*We start generating the bytecode*/
//	bytecodeHeader *oneBytecodeHeader = (bytecodeHeader*) &(bytecode[0]);
//	oneBytecodeHeader->functionTableSize = procedureNumber;
//	oneBytecodeHeader->functionTablePtr = 16; // = FIX_INT(12) TODO : change for nios
//
//	//TODO : fill information on symbol table when ready
//	struct functionHeader *funtionHeaders = (struct functionHeader *) &(bytecode[16]);
//
//
//
//	//Declaration of placeInBlockTable which will keep track of the place where we write
//	int placeInBlockTable = 16 + (procedureNumber<<3);
//	*placeCode = placeInBlockTable + (totalBasicBlockNumber << 4);
//
//
//	unsigned char numberSucc;
//	unsigned char numberGR;
//	unsigned char numberGW;
//	for (unsigned int oneProcedure = 0; oneProcedure<procedureNumber; oneProcedure++){
//
//		//**************************************************************************
//		//Procedure header is placed at 16 + oneProcedure * 8
//		//Its size is 8 bytes...
//		// 1 byte for the number of global variables
//		// 1 blank byte
//		// 2 bytes for the number of basic block
//		// 4 bytes pointing to the block table. The block table should start at 16 + procedureNumber*8
//
//
//		struct blockHeader *blockHeaders = (struct blockHeader *) &(bytecode[placeInBlockTable]);
//
//
//		funtionHeaders[oneProcedure].blockTablePtr = placeInBlockTable;
//		//**************************************************************************
//
//
//		int procedureEnd = procedureEnds[oneProcedure];
//		int start = procedureStarts[oneProcedure];
//
//		int oneInstructionIndex = start;
//
//		/* Data for global variables */
//		int globalVariables[32][33] = {256,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,257,258,511};
//		//int globalVariables[32][33] = {256,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,257,258,511};
//		for (int i=1; i<32; i++)
//			for (int j=0; j<33; j++)
//				globalVariables[i][j] = globalVariables[0][j];
//
//		int globalVariableCounter = 259;
//		unsigned int reg1_mul = 0, reg2_mul = 0, imm_mul = 0, is_imm_mul = 0;
//
//
//		//We save values to restore them after the first generation
//		int savedPlaceCode = *placeCode;
//		int savedPlaceBlock = placeInBlockTable;
//
//		unsigned long long registersUsage[32];
//
//		while (oneInstructionIndex < procedureEnd){
//
//			unsigned long long currentRegistresUsageWord = 0;
//
//
//			/* Basic Block metadata */
//			int numberSuccessors = 0;
//			int successor1;
//			int successor2;
//			unsigned char indexInCurrentBlock = 0;
//
//			/* Datastructures for dag construction*/
//			int registers[64] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
//
//			/* Generated code */
//			unsigned char numbersSuccessor[256];
//			unsigned char numbersDataSuccessor[256];
//			unsigned char successors[256][30];
//
//			unsigned char numbersPredecessor[256];
//			int predecessors[256][8];
//
//			/* Datastructure for RAW dependencies on global registers */
//			int lastWriterOnGlobal[32] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
//			int lastReaderOnGlobalCounter[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//			int lastReaderOnGlobalPlaceToWrite[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//			int lastReaderOnGlobal[32][4];
//
//			/* Datastructure for control dependencies on memories */
//			int lastWriterOnMemory = -1;
//			int lastReaderOnMemoryCounter = 0;
//			int lastReaderOnMemoryPlaceToWrite = 0;
//			int lastReaderOnMemory[4];
//
//			int isCallBlock = 0;
//			basicBlockNumber = blocksBoundaries[oneInstructionIndex]>>2;
//
//			char haveJump = 0;
//			char jumpID = 0;
//
//			short reglo, reghi = 0;
//
//
//
//
//			do {
//
//				char insertMove_ena = 0;
//				short insertMove_src;
//
//				rename_droppedInstruction = 0;
//
//				int wasOutside = 0;
//
//				unsigned int oneInstruction = 0;
//				if (rename_isOutsideNext){
//					wasOutside = 1;
//					rename_isOutsideNext = 0;
//				}
//				else{
//					oneInstruction = (code[4*oneInstructionIndex] << 24)
//							+ (code[4*oneInstructionIndex+1] << 16)
//							+ (code[4*oneInstructionIndex+2] << 8)
//							+ (code[4*oneInstructionIndex+3]);
//				}
//
//				if (debugLevel >= 1)
//					printf("Source instruction is %x\n", oneInstruction);
//
//
//				char op = (oneInstruction >> 26);
//
//				char funct = (oneInstruction & 0x3f);
//				int shamt = ((oneInstruction >> 6) & 0x1f);
//				char rd = ((oneInstruction >> 11) & 0x1f);
//				char rt = ((oneInstruction >> 16) & 0x1f);
//				char rs = ((oneInstruction >> 21) & 0x1f);
//				int regimm = rt;
//				int address = (oneInstruction & 0xffff);
//
//				int tgtadr = (oneInstruction & 0x3ffffff);
//
//				int correctedTgtadr = tgtadr - (addressStart>>2);
//
//
//				numbersSuccessor[indexInCurrentBlock] = 0;
//				numbersPredecessor[indexInCurrentBlock] = 0;
//
//				/* Local value for numberDependencies */
//				char numberDependencies = 0;
//
//				char pred1_ena = 0, pred2_ena = 0, dest_ena = 0;
//				short pred1_reg = rs, pred2_reg = rt, dest_reg;
//
//
//
//				//We determine whether to use pred1
//				if (wasOutside){
//					pred1_ena = rename_outsideNext_pred1_ena;
//					pred1_reg = rename_outsideNext_pred1_reg;
//				}
//				else if (op == R && ((funct == SLL && rd != 0) || funct == SRA || funct == SRL)){
//					pred1_ena = 1;
//					pred1_reg = rt;
//				}
//
//				else if (!(op == J || op == JAL ) &&  !(op == 0 && funct == 0 && rd == 0)){
//					pred1_ena = 1;
//				}
//
//				//We determine whether to use pred2
//				if (wasOutside){
//					pred2_ena = rename_outsideNext_pred2_ena;
//					pred2_reg = rename_outsideNext_pred2_reg;
//				}
//				else if (op == R && (funct == MFHI || funct == MFLO)){
//					pred2_ena = 0;
//				}
//				else if (op == BEQ || op == BNE || op == BLEZ || op == BGTZ || op == SB || op == SH || op == SW ||
//						(op == R && !(funct == SLL || funct == SRL || funct == SRA || funct == 0)))
//					pred2_ena = 1;
//
//				//We determine whether to use dest
//				if (wasOutside){
//					dest_ena = rename_outsideNext_dest_ena;
//					dest_reg = rename_outsideNext_dest_reg;
//				}
//				else if (op == R && (funct == MULT || funct == MULTU)){
//					dest_ena = 0;
//				}
//				else if (op == R){
//
//					if (oneInstruction == 0){
//						rename_droppedInstruction = 1;
//					}
//					else if (funct != JALR && funct != JR){
//						dest_ena = 1;
//						dest_reg = rd;
//					}
//
//				}
//				else if (! (op == SB || op == SH || op == SW || op == J || op == JAL || op == BEQ || op == BNE || op == BLEZ || op == BGTZ )){
//					dest_ena = 1;
//					dest_reg = rt;
//				}
//
//
//				//******************************************
//				//We find predecessors and correct its value
//
//				int temp_pred1 = registers[pred1_reg];
//				if ((wasOutside && rename_outsideNext_pred1_solved))
//					temp_pred1 = rename_outsideNext_pred1;
//				if (op == 0 && funct == MFHI)
//					temp_pred1 = reghi;
//				if (op == 0 && funct == MFLO)
//					temp_pred1 = reglo;
//
//
//				int pred1;
//				if (!pred1_ena & temp_pred1 == -1) //If value comes from global register
//					temp_pred1 = globalVariables[basicBlockNumber & 0x1f][pred1_reg];
//
//
//				if (pred1_ena){
//
//					//To gather succ information on different path
//					char succ_ena = 0;
//					char succ_src;
//					char succ_isData = 0;
//
//					if (temp_pred1 == -1){ //If value comes from global register
//						if (globalVariables[basicBlockNumber & 0x1f][pred1_reg] == -1){ //If value is not assigned yet, we allocate the value from globalVariableCounter
//							temp_pred1 = globalVariableCounter;
//							globalVariables[basicBlockNumber & 0x1f][pred1_reg] = globalVariableCounter++;
//						}
//						else{ //Otherwise we use the already allocated value
//							temp_pred1 = globalVariables[basicBlockNumber & 0x1f][pred1_reg];
//						}
//
//						//** We also mark this node as a reader of the global value. Should this value be modified
//						//** in the block, we will add dependencies
//						if (lastReaderOnGlobalCounter[pred1_reg] < 3){
//							lastReaderOnGlobalCounter[pred1_reg]++;
//							//** We handle successors: if the value in a global register has been written in the same
//							//** basic block, we add a dependency from the writer node to this instruction.
//							if (lastWriterOnGlobal[pred1_reg] != -1){
//								succ_ena = 1;
//								succ_src = lastWriterOnGlobal[pred1_reg];
//								numberDependencies++;
//							}
//						}
//						else{
//							int readerToEvince = lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite[pred1_reg]];
//							succ_ena = 1;
//							succ_src = readerToEvince;
//							numberDependencies++;
//						}
//						lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite[pred1_reg]] = indexInCurrentBlock;
//						lastReaderOnGlobalPlaceToWrite[pred1_reg] = (lastReaderOnGlobalPlaceToWrite[pred1_reg] + 1) % 3;
//
//					}
//					else{ //We add a successor to the predecessor
//						succ_ena = 1;
//						succ_src = temp_pred1;
//						succ_isData = 1;
//						numberDependencies++;
//					}
//
//					pred1 = temp_pred1;
//					if (succ_ena)
//						if (succ_isData){
//							char nbSucc = writeDataSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//							if (nbSucc == 7){
//								insertMove_ena = 1;
//								insertMove_src = pred1;
//								dest_reg = pred1_reg;
//								dest_ena = 1;
//							}
//						}
//						else
//							writeSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//
//
//
//
//
//					predecessors[indexInCurrentBlock][numbersPredecessor[indexInCurrentBlock]++] = pred1;
//
//				}
//
//
//				int temp_pred2 = registers[pred2_reg];
//				int pred2;
//				if (!pred2_ena & temp_pred2 == -1) //If value comes from global register
//					temp_pred2 = globalVariables[basicBlockNumber & 0x1f][pred2_reg];
//
//				if (pred2_ena){
//
//					//To gather succ information on different path
//					char succ_ena = 0;
//					char succ_src;
//					char succ_isData = 0;
//
//					if (temp_pred2 == -1){ //If value comes from global register
//						if (globalVariables[basicBlockNumber & 0x1f][pred2_reg] == -1){
//							temp_pred2 = globalVariableCounter;
//							globalVariables[basicBlockNumber & 0x1f][pred2_reg] = globalVariableCounter++;
//						}
//						else
//							temp_pred2 = globalVariables[basicBlockNumber & 0x1f][pred2_reg];
//
//						//If the global register has been used in the current block, we add a control dependency
//						if (lastReaderOnGlobalCounter[pred2_reg] < 3){
//							lastReaderOnGlobalCounter[pred2_reg]++;
//							if (lastWriterOnGlobal[pred2_reg] != -1){
//								succ_ena = 1;
//								succ_src = lastWriterOnGlobal[pred2_reg];
//								numberDependencies++;
//							}
//						}
//						else{
//							int readerToEvince = lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite[pred2_reg]];
//							succ_ena = 1;
//							succ_src = readerToEvince;
//							numberDependencies++;
//						}
//						lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite[pred2_reg]] = indexInCurrentBlock;
//						lastReaderOnGlobalPlaceToWrite[pred2_reg] = (lastReaderOnGlobalPlaceToWrite[pred2_reg] + 1) % 3;
//					}
//					else{ //We add a successor to the predecessor
//						succ_ena = 1;
//						succ_src = temp_pred2;
//						succ_isData = 1;
//						numberDependencies++;
//
//					}
//
//					pred2 = temp_pred2;
//
//					if (succ_ena)
//						if (succ_isData){
//							char nbSucc = writeDataSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//							if (nbSucc == 7){
//								insertMove_ena = 1;
//								insertMove_src = pred2;
//								dest_reg = pred2_reg;
//								dest_ena = 1;
//							}
//						}
//						else
//							writeSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//
//					predecessors[indexInCurrentBlock][numbersPredecessor[indexInCurrentBlock]++] = pred2;
//				}
//
//
//				//******************************************
//				//We set the destination
//
//				int temp_destination = indexInCurrentBlock;
//				int destination;
//				if (!dest_ena & temp_destination == -1) //If value comes from global register
//					temp_destination = globalVariables[basicBlockNumber & 0x1f][dest_reg];
//
//
//				char alloc = 1;
//
//				if (dest_ena) {
//					if (globalVariables[basicBlockNumber & 0x1f][dest_reg] == -1 || rename_outsideNext_dest_alloc || insertMove_ena){
//						registers[dest_reg] = indexInCurrentBlock;
//
//						//We mark the value as a write which is potentially not read
//						currentRegistresUsageWord |= 1<<dest_reg;
//					}
//					else{
//						alloc = 0;
//						temp_destination = globalVariables[basicBlockNumber & 0x1f][dest_reg];
//						lastWriterOnGlobal[dest_reg] = indexInCurrentBlock;
//						for (int oneReader = 0; oneReader < lastReaderOnGlobalCounter[dest_reg]; oneReader++)
//							if (lastReaderOnGlobal[dest_reg][oneReader] != indexInCurrentBlock){
//								writeSuccessor(bytecode, lastReaderOnGlobal[dest_reg][oneReader], indexInCurrentBlock, *placeCode);
//								numberDependencies++;
//							}
//						lastReaderOnGlobalCounter[dest_reg] = 0;
//
//					}
//				}
//				destination = temp_destination;
//
//			//	printf("DEBUG : for instr %d : op = %x funct = %x pred1_reg = %d pred1_ena = %d pred2_reg = %d, pred2_ena = %d dest_reg = %d dest_ena = %d\n", indexInCurrentBlock, op, funct, pred1_reg, pred1_ena, pred2_reg, pred2_ena, dest_reg,dest_ena);
//
//				/***************************************************************/
//				/*  We generate the instruction  */
//				unsigned int bytecode1=0, bytecode2=0, bytecode3=0, bytecode4=0;
//
//				if (insertMove_ena){
//					bytecode1 += 0x8<<28;
//					bytecode1 += alloc << 27;
//					bytecode1 += 0x41 << 19;
//
//					bytecode1 += insertMove_src;
//
//					bytecode2 += destination << 14;
//					bytecode2 += numberDependencies << 6;
//
//					rename_isOutsideNext = wasOutside;
//					wasOutside = 1;
//				}
//				else if (wasOutside){
//					bytecode1 = rename_outsideNext_bytecode1;
//					if (rename_outsideNext_isImm){
//						bytecode1 += (rename_outsideNext_imm & 0x7ff);
//						bytecode2 += pred1 << 23;
//						bytecode2 += destination << 14;
//					}
//					else if (rename_outsideNext_isLongImm){
//						bytecode1 += ((rename_outsideNext_imm>>9) & 0x3ff);
//
//						bytecode2 += (rename_outsideNext_imm & 0x1ff) << 23;
//
//						if (pred1_ena)
//							bytecode2 += pred1 << 14;
//						else
//							bytecode2 += destination << 14;
//
//					}
//					else{
//						bytecode1 += pred1;
//
//						bytecode2 += pred2 << 23;
//						bytecode2 += destination << 14;
//					}
//
//					bytecode2 += numberDependencies << 6;
//
//
//				}
//				else if (op == R){
//					//Instrucion is R-Type
//					if (funct == SLL || funct == SRL || funct == SRA){
//
//						bytecode1 += (0x8<<28); //stage=2 type = 0
//						bytecode1 += alloc << 27;
//						bytecode1 += functBinding[funct] << 19;
//						bytecode1 += 2<<17; // i=1 br=0
//						bytecode1 += (shamt & 0x7ff);
//
//						bytecode2 += pred1 << 23;
//						bytecode2 += destination << 14;
//						bytecode2 += numberDependencies<<6;
//					}
//					else if (funct == MFHI || funct == MFLO){
//
//
//						bytecode1 += 0x8<<28;
//						bytecode1 += alloc << 27;
//						bytecode1 += 0x41 << 19;
//
//						bytecode1 += pred1;
//
//						bytecode2 += destination << 14;
//						bytecode2 += numberDependencies << 6;
//
//					}
//					else if(funct == JALR){
//						numberSuccessors = 1;
//						successor1 = basicBlockNumber + 1;
//
//						bytecode1 += (0x2<<28); //stage=0 type = 2
//						bytecode1 += 0;
//						bytecode1 += functBinding[funct] << 19;
//
//						bytecode2 += pred1 << 14;
//						bytecode2 += numberDependencies<<6;
//
//						haveJump = 1;
//						jumpID = indexInCurrentBlock;
//					}
//					else if (funct == JR){
//						if (pred1_reg == 31){
//							numberSuccessors = 0;
//							successor1 = basicBlockNumber + 1;
//
//							bytecode1 += (0x2<<28); //stage=0 type = 2
//							bytecode1 += 0;
//							bytecode1 += functBinding[funct] << 19;
//
//							bytecode2 += pred1 << 14;
//							bytecode2 += numberDependencies<<6;
//
//							haveJump = 1;
//							jumpID = indexInCurrentBlock;
//						}
//						else{
//							printf("Funct not handled %x (JR)\n", funct);
//							exit(-1);
//						}
//					}
//					else if (funct == MULT || funct == MULTU){
//
//						bytecode1 += 0xc << 28;
//						bytecode1 += 0x1 << 27;
//						bytecode1 += functBinding[MFLO] << 19;
//						bytecode1 += pred1;
//
//						bytecode2 += pred2 << 23;
//
//						bytecode2 += indexInCurrentBlock << 14;
//						bytecode2 += numberDependencies<< 6;
//
//
//						reglo = indexInCurrentBlock;
//						reghi = indexInCurrentBlock+1;
//
//						rename_outsideNext_bytecode1 += 0xc << 28; //stage=2 type = 0
//						rename_outsideNext_bytecode1 += 1 << 27;
//						rename_outsideNext_bytecode1 += functBinding[MFHI]<<19;
//
//
//						rename_outsideNext_isImm = 0;
//						rename_outsideNext_isLongImm = 0;
//						rename_outsideNext_imm = 0;
//						rename_outsideNext_pred1_ena = 1;
//						rename_outsideNext_pred1_solved = 0;
//						rename_outsideNext_pred1_reg = pred1_reg;
//						rename_outsideNext_pred2_ena = 1;
//						rename_outsideNext_pred2_reg = pred2_reg;
//						rename_outsideNext_dest_ena = 0;
//						rename_outsideNext_dest_reg = indexInCurrentBlock+1;
//						rename_outsideNext_dest_alloc = 1;
//
//						rename_isOutsideNext = 1;
//
//					}
//					else {
//
//						if (functBinding[funct] == -1 || functBinding[funct] == -2){
//							printf("Funct not handled %x in %x\n", funct, oneInstruction);
//							exit(-1);
//						}
//
//						bytecode1 += (0x8<<28); //stage=2 type = 0
//						bytecode1 += alloc << 27;
//						bytecode1 += functBinding[funct] << 19;
//						bytecode1 += 0; // i=0 br=0
//						bytecode1 += pred1;
//
//						bytecode2 += pred2 << 23;
//						bytecode2 += destination << 14;
//						bytecode2 += numberDependencies<<6;
//					}
//				}
//				else if (op == LUI){
//
//					//Instruction is fisrt translated as a movi to the destination register
//					bytecode1 += (0xa<<28); //stage=0 type = 2
//					bytecode1 += alloc << 27;
//					bytecode1 += 0x58 << 19; //opcode of movi
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += ((address>>9) & 0x3ff);
//
//					bytecode2 += (address & 0x1ff) << 23;
//					bytecode2 += destination<<14;
//					bytecode2 += numberDependencies<<6;
//
//
//
//					//Then next instruction is shl, from destination register to destination register, of 16bits
//					rename_outsideNext_bytecode1 += (0x8<<28); //stage=2 type = 0
//					rename_outsideNext_bytecode1 += alloc << 27;
//					rename_outsideNext_bytecode1 += 0x4f << 19;
//					rename_outsideNext_bytecode1 += 2<<17; // i=1 br=0
//
//					rename_outsideNext_isImm = 1;
//					rename_outsideNext_isLongImm = 0;
//					rename_outsideNext_imm = 16;
//					rename_outsideNext_pred1_ena = 1;
//					rename_outsideNext_pred1_solved = 0;
//
//					rename_outsideNext_pred1_reg = dest_reg;
//					rename_outsideNext_pred2_ena = 0;
//					rename_outsideNext_dest_ena = 1;
//					rename_outsideNext_dest_reg = dest_reg;
//					rename_outsideNext_dest_alloc = 0;
//
//					rename_isOutsideNext = 1;
//
//				}
//				else if (op == SB || op == SH || op == SW){
//					/****************************/
//					/* We update lastWriterOnMemory and add required dependencies to keep memory coherence */
//
//					for (int oneReader = 0; oneReader < lastReaderOnMemoryCounter; oneReader++){
//						writeSuccessor(bytecode, lastReaderOnMemory[oneReader], indexInCurrentBlock, *placeCode);
//						numberDependencies++;
//					}
//
//					lastReaderOnMemoryCounter = 0;
//					lastWriterOnMemory = indexInCurrentBlock;
//
//					bytecode1 += (0x10<<26); //stage=1, type=0=alloc=allocbr
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += (address & 0x7ff);
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += pred2 << 14;
//					bytecode2 += numberDependencies<<6;
//
//				}
//				else if (op == LB || op == LH || op == LW || op == LBU || op == LHU){
//					/****************************/
//					/* We update lastReaderOneMemory and add required dependencies to keep memory coherence */
//					if (lastReaderOnMemoryCounter < 4){
//						lastReaderOnMemoryCounter++;
//						if (lastWriterOnMemory != -1){
//							writeSuccessor(bytecode, lastWriterOnMemory, indexInCurrentBlock, *placeCode);
//							numberDependencies++;
//						}
//					}
//					else{
//						int readerToEvince = lastReaderOnMemory[lastReaderOnMemoryPlaceToWrite];
//						writeSuccessor(bytecode, readerToEvince, indexInCurrentBlock, *placeCode);
//						numberDependencies++;
//					}
//					lastReaderOnMemory[lastReaderOnMemoryPlaceToWrite] = indexInCurrentBlock;
//					lastReaderOnMemoryPlaceToWrite = (lastReaderOnMemoryPlaceToWrite + 1) & 0x3;
//
//					lastReaderOnMemoryCounter = 0;
//					lastWriterOnMemory = indexInCurrentBlock;
//
//					bytecode1 += (0x4<<28); //stage=1 type = 0
//					bytecode1 += alloc << 27;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += (address & 0x7ff);
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += destination << 14;
//					bytecode2 += numberDependencies<<6;
//
//				}
//				else if (op == J){
//					numberSuccessors = 1;
//					successor1 = blocksBoundaries[correctedTgtadr];
//
//					bytecode1 += (0x2<<28); //stage=0 type = 2
//					bytecode1 += 0;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//
//					bytecode2 += numberDependencies<<6;
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock;
//				}
//				else if (op == BEQ || op == BNE){
//					numberSuccessors = 2;
//					if (address >= 32768)
//						address = address - 65536;
//					successor1 = blocksBoundaries[(address+oneInstructionIndex) + 1];
//					successor2 = blocksBoundaries[oneInstructionIndex + 2];
//
//					bytecode1 += (0x8<<28); //stage=2
//					bytecode1 += 1 << 27;
//					bytecode1 += opcodeBinding[op] << 19;//opcode (here the binding will be done with cmpeq/cmpne
//					bytecode1 += 0<<17; // i=1 br=0
//					bytecode1 += pred1;
//
//					bytecode2 += pred2 << 23;
//					bytecode2 += oneInstructionIndex << 14;
//					bytecode2 += numberDependencies<<6;
//
//					rename_isOutsideNext = 1;
//					rename_outsideNext_bytecode1 = (0x2<<28) + (0x25<<19) + (2<<17);
//					rename_outsideNext_isLongImm = 1;
//					rename_outsideNext_imm = 0;
//					rename_outsideNext_pred1_ena = 1;
//					rename_outsideNext_pred1_solved = 1;
//
//					rename_outsideNext_pred2_ena = 0;
//					rename_outsideNext_dest_ena = 0;
//
//					rename_outsideNext_pred1_reg = 32;
//					rename_outsideNext_pred1 = indexInCurrentBlock; //TODO marche pas !!!
//					rename_outsideNext_dest_alloc = 0;
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock +1;
//
//
//				}
//				else if (op == BLEZ || op == BGTZ || op == REGIMM){
//					numberSuccessors = 2;
//					if (address >= 32768)
//						address = address - 65536;
//					successor1 = blocksBoundaries[(address+oneInstructionIndex) + 1];
//					successor2 = blocksBoundaries[oneInstructionIndex + 2];
//
//					bytecode1 += (0x8<<28); //stage=2
//					bytecode1 += 1 << 27;
//					bytecode1 += ((op==REGIMM) ? regimmBindings[rt] : opcodeBinding[op]) << 19;//opcode (here the binding will be done with cmpeq/cmpne
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += 0;
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += oneInstructionIndex << 14;
//					bytecode2 += numberDependencies<<6;
//
//					rename_isOutsideNext = 1;
//					rename_outsideNext_bytecode1 = (0x2<<28) + (0x25<<19) + (2<<17);
//					rename_outsideNext_isLongImm = 1;
//					rename_outsideNext_imm = address;
//					rename_outsideNext_pred1_ena = 1;
//					rename_outsideNext_pred2_ena = 1;
//					rename_outsideNext_dest_ena = 1;
//					rename_outsideNext_pred1_solved = 1;
//					rename_outsideNext_dest_alloc = 0;
//					rename_outsideNext_pred1_reg = 32;
//					rename_outsideNext_pred1 = indexInCurrentBlock; //TODO marche pas !!!
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock+1;
//
//				}
//				else if (op == JAL){
//					isCallBlock = 1;
//					numberSuccessors = 1;
//					//successor1 = basicBlockNumber + 1;
//					successor1 = proceduresBoundaries[correctedTgtadr];
//
//					bytecode1 += (0x2<<28); //stage=0 type = 2
//					bytecode1 += 0;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//
//					bytecode2 += numberDependencies<<6;
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock;
//				}
//				else{ //For all other instructions
//
//					//These instruction should not be immediate
//					if (opcodeBinding[op] == -1 || opcodeBinding[op] == -2){
//						printf("Opcode not handled at %x :  %x (%x)\n",(oneInstructionIndex>>2) + addressStart, op, oneInstruction);
//						exit(-1);
//					}
//
//					bytecode1 += (0x8<<28); //stage=2 (mult are handled elsewhere as they need to be modified) type = 0
//					bytecode1 += alloc << 27;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += (address & 0x7ff);
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += destination << 14;
//					bytecode2 += numberDependencies<<6;
//
//				}
//
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock, bytecode1);
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock+4, bytecode2);
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock+8, bytecode3);
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock+12, bytecode4);
//
//				if (debugLevel >= 1)
//					printf("Generated bytecode is %x %x %x %x\n", bytecode1, bytecode2, bytecode3, bytecode4);
//
//
//				if (!rename_droppedInstruction)
//					indexInCurrentBlock++;
//
//				if (wasOutside)
//					rename_isOutsideNext = 0;
//				else
//					oneInstructionIndex++;
//
//			}
//			while ((blocksBoundaries[oneInstructionIndex] & 0x1) == 0 && oneInstructionIndex < procedureEnd);
//
//
//			//*****************************************************************
//			//  Second loop to add dependencies to jump instruction
//
//			if (haveJump)
//				for (int oneInstructionFromBlock = 0; oneInstructionFromBlock < indexInCurrentBlock; oneInstructionFromBlock++){
//					int offset[4] = {3, 1, -1, -3};
//
//					unsigned char wordWithNbDataSucc = ((unsigned int *)bytecode)[*placeCode/4 + 4*oneInstructionFromBlock + 1]; //FIXME endianness
//					unsigned char nbSucc = wordWithNbDataSucc & 0x7;
//
//					if (nbSucc == 0){
//
//						unsigned int index = *placeCode + oneInstructionFromBlock + 16*oneInstructionFromBlock + 8 + 1 + (6 - nbSucc) + offset[(7-nbSucc) & 0x3]; //FIXME endianness
//
//						bytecode[index] = jumpID;
//						nbSucc++;
//
//						wordWithNbDataSucc = (wordWithNbDataSucc & 0xf8) + nbSucc;
//						bytecode[*placeCode/4 + 4*oneInstructionFromBlock + 1] = wordWithNbDataSucc;//FIXME endianness
//					}
//				}
//
//
//
//			successor1 >>= 2;
//			successor2 >>= 2;
//
//			//*****************************************************************
//			/* We write information on current block in the bytecode */
//
//
//
//			char typeOfFirstSucc = 0;
//			if (numberSuccessors == 0 && !haveJump){
//				typeOfFirstSucc = 254;
//				numberSuccessors++;
//				successor1 = basicBlockNumber + 1;
//			}
//
//			//First we write the block header
//
//			blockHeaders[basicBlockNumber].nbrInstr = indexInCurrentBlock;
//			blockHeaders[basicBlockNumber].numberSucc = numberSuccessors;
//			blockHeaders[basicBlockNumber].placeInstr = *placeCode;
//			blockHeaders[basicBlockNumber].placeOfSucc = *placeCode + 16*indexInCurrentBlock;
//
//			int placeSucc = *placeCode + 16*indexInCurrentBlock;
//
//
//			if (isCallBlock){
//				bytecode[placeSucc] = indexInCurrentBlock-1; //The place of the call
//				bytecode[placeSucc + 1] = 255;
//				bytecode[placeSucc + 4] = (successor1>>24) & 0xff;
//				bytecode[placeSucc + 5] = (successor1>>16) & 0xff;
//				bytecode[placeSucc + 6] = (successor1>>8) & 0xff;
//				bytecode[placeSucc + 7] = (successor1>>0) & 0xff;
//
//				placeSucc += 8;
//
//			}
//			else{
//				if (numberSuccessors >= 1){
//					bytecode[placeSucc] = jumpID; //The place of the jump
//					bytecode[placeSucc + 1] = typeOfFirstSucc;
//					bytecode[placeSucc + 2] = (successor1>>8) & 0xff;
//					bytecode[placeSucc + 3] = (successor1>>0) & 0xff;
//				}
//				if (numberSuccessors >= 2){
//					bytecode[placeSucc +8] = jumpID; //The place of the jump
//					bytecode[placeSucc + 9] = 0;
//					bytecode[placeSucc + 10] = (successor2>>8) & 0xff;
//					bytecode[placeSucc + 11] = (successor2>>0) & 0xff;
//				}
//
//			}
//
//			//We update place of code for the next BB
//			*placeCode = *placeCode +(16*indexInCurrentBlock) + numberSuccessors*8;
//
//			registersUsage[basicBlockNumber] = currentRegistresUsageWord;
//			//*****************************************************************
//
//
//			basicBlockNumber++;
//			start = oneInstructionIndex;
//			placeInBlockTable = placeInBlockTable + sizeof(struct blockHeader);
//
//		}
//
//		/* We complete information on current procedure */
//		funtionHeaders[oneProcedure].nbrBlock = basicBlockNumber;
//
//
//		globalVariableCounter = registerAllocation(bytecode, basicBlockNumber, blockHeaders, registersUsage, globalVariables);
//
//		*placeCode = savedPlaceCode;
//		placeInBlockTable = savedPlaceBlock;
//
//
//		oneInstructionIndex = procedureStarts[oneProcedure];
//
//
//		reg1_mul = 0;
//		reg2_mul = 0;
//		imm_mul = 0;
//		is_imm_mul = 0;
//
//
//		while (oneInstructionIndex < procedureEnd){
//
//			unsigned long long currentRegistresUsageWord = 0;
//
//
//			/* Basic Block metadata */
//			int numberSuccessors = 0;
//			int successor1;
//			int successor2;
//			unsigned char indexInCurrentBlock = 0;
//
//			/* Datastructures for dag construction*/
//			int registers[64] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
//
//			/* Generated code */
//			unsigned char numbersSuccessor[256];
//			unsigned char numbersDataSuccessor[256];
//			unsigned char successors[256][30];
//
//			unsigned char numbersPredecessor[256];
//			int predecessors[256][8];
//
//			/* Datastructure for RAW dependencies on global registers */
//			int lastWriterOnGlobal[32] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
//			int lastReaderOnGlobalCounter[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//			int lastReaderOnGlobalPlaceToWrite[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//			int lastReaderOnGlobal[32][4];
//
//			/* Datastructure for control dependencies on memories */
//			int lastWriterOnMemory = -1;
//			int lastReaderOnMemoryCounter = 0;
//			int lastReaderOnMemoryPlaceToWrite = 0;
//			int lastReaderOnMemory[4];
//
//			int isCallBlock = 0;
//			basicBlockNumber = blocksBoundaries[oneInstructionIndex]>>2;
//
//			char haveJump = 0;
//			char jumpID = 0;
//
//			short reglo, reghi = 0;
//
//
//
//
//			do {
//
//				char insertMove_ena = 0;
//				short insertMove_src;
//
//				rename_droppedInstruction = 0;
//
//				int wasOutside = 0;
//
//				unsigned int oneInstruction = 0;
//				if (rename_isOutsideNext){
//					wasOutside = 1;
//					rename_isOutsideNext = 0;
//				}
//				else{
//					oneInstruction = (code[4*oneInstructionIndex] << 24)
//							+ (code[4*oneInstructionIndex+1] << 16)
//							+ (code[4*oneInstructionIndex+2] << 8)
//							+ (code[4*oneInstructionIndex+3]);
//				}
//
//				if (debugLevel >= 1)
//					printf("Source instruction is %x\n", oneInstruction);
//
//
//				char op = (oneInstruction >> 26);
//
//				char funct = (oneInstruction & 0x3f);
//				int shamt = ((oneInstruction >> 6) & 0x1f);
//				char rd = ((oneInstruction >> 11) & 0x1f);
//				char rt = ((oneInstruction >> 16) & 0x1f);
//				char rs = ((oneInstruction >> 21) & 0x1f);
//				int regimm = rt;
//				int address = (oneInstruction & 0xffff);
//
//				int tgtadr = (oneInstruction & 0x3ffffff);
//
//				int correctedTgtadr = tgtadr - (addressStart>>2);
//
//
//				numbersSuccessor[indexInCurrentBlock] = 0;
//				numbersPredecessor[indexInCurrentBlock] = 0;
//
//				/* Local value for numberDependencies */
//				char numberDependencies = 0;
//
//				char pred1_ena = 0, pred2_ena = 0, dest_ena = 0;
//				short pred1_reg = rs, pred2_reg = rt, dest_reg;
//
//
//
//				//We determine whether to use pred1
//				if (wasOutside){
//					pred1_ena = rename_outsideNext_pred1_ena;
//					pred1_reg = rename_outsideNext_pred1_reg;
//				}
//				else if (op == R && ((funct == SLL && rd != 0) || funct == SRA || funct == SRL)){
//					pred1_ena = 1;
//					pred1_reg = rt;
//				}
//
//				else if (!(op == J || op == JAL ) &&  !(op == 0 && funct == 0 && rd == 0)){
//					pred1_ena = 1;
//				}
//
//				//We determine whether to use pred2
//				if (wasOutside){
//					pred2_ena = rename_outsideNext_pred2_ena;
//					pred2_reg = rename_outsideNext_pred2_reg;
//				}
//				else if (op == R && (funct == MFHI || funct == MFLO)){
//					pred2_ena = 0;
//				}
//				else if (op == BEQ || op == BNE || op == BLEZ || op == BGTZ || op == SB || op == SH || op == SW ||
//						(op == R && !(funct == SLL || funct == SRL || funct == SRA || funct == 0)))
//					pred2_ena = 1;
//
//				//We determine whether to use dest
//				if (wasOutside){
//					dest_ena = rename_outsideNext_dest_ena;
//					dest_reg = rename_outsideNext_dest_reg;
//				}
//				else if (op == R && (funct == MULT || funct == MULTU)){
//					dest_ena = 0;
//				}
//				else if (op == R){
//
//					if (oneInstruction == 0){
//						rename_droppedInstruction = 1;
//					}
//					else if (funct != JALR && funct != JR){
//						dest_ena = 1;
//						dest_reg = rd;
//					}
//
//				}
//				else if (! (op == SB || op == SH || op == SW || op == J || op == JAL || op == BEQ || op == BNE || op == BLEZ || op == BGTZ )){
//					dest_ena = 1;
//					dest_reg = rt;
//				}
//
//
//				//******************************************
//				//We find predecessors and correct its value
//
//				int temp_pred1 = registers[pred1_reg];
//				if ((wasOutside && rename_outsideNext_pred1_solved))
//					temp_pred1 = rename_outsideNext_pred1;
//				if (op == 0 && funct == MFHI)
//					temp_pred1 = reghi;
//				if (op == 0 && funct == MFLO)
//					temp_pred1 = reglo;
//
//
//				int pred1;
//				if (!pred1_ena & temp_pred1 == -1) //If value comes from global register
//					temp_pred1 = globalVariables[basicBlockNumber & 0x1f][pred1_reg];
//
//
//				if (pred1_ena){
//
//					//To gather succ information on different path
//					char succ_ena = 0;
//					char succ_src;
//					char succ_isData = 0;
//
//					if (temp_pred1 == -1){ //If value comes from global register
//						if (globalVariables[basicBlockNumber & 0x1f][pred1_reg] == -1){ //If value is not assigned yet, we allocate the value from globalVariableCounter
//							temp_pred1 = globalVariableCounter;
//							globalVariables[basicBlockNumber & 0x1f][pred1_reg] = globalVariableCounter++;
//						}
//						else{ //Otherwise we use the already allocated value
//							temp_pred1 = globalVariables[basicBlockNumber & 0x1f][pred1_reg];
//						}
//
//						//** We also mark this node as a reader of the global value. Should this value be modified
//						//** in the block, we will add dependencies
//						if (lastReaderOnGlobalCounter[pred1_reg] < 3){
//							lastReaderOnGlobalCounter[pred1_reg]++;
//							//** We handle successors: if the value in a global register has been written in the same
//							//** basic block, we add a dependency from the writer node to this instruction.
//							if (lastWriterOnGlobal[pred1_reg] != -1){
//								succ_ena = 1;
//								succ_src = lastWriterOnGlobal[pred1_reg];
//								numberDependencies++;
//							}
//						}
//						else{
//							int readerToEvince = lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite[pred1_reg]];
//							succ_ena = 1;
//							succ_src = readerToEvince;
//							numberDependencies++;
//						}
//						lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite[pred1_reg]] = indexInCurrentBlock;
//						lastReaderOnGlobalPlaceToWrite[pred1_reg] = (lastReaderOnGlobalPlaceToWrite[pred1_reg] + 1) % 3;
//
//					}
//					else{ //We add a successor to the predecessor
//						succ_ena = 1;
//						succ_src = temp_pred1;
//						succ_isData = 1;
//						numberDependencies++;
//					}
//
//					pred1 = temp_pred1;
//					if (succ_ena)
//						if (succ_isData){
//							char nbSucc = writeDataSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//							if (nbSucc == 7){
//								insertMove_ena = 1;
//								insertMove_src = pred1;
//								dest_reg = pred1_reg;
//								dest_ena = 1;
//							}
//						}
//						else
//							writeSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//
//
//
//
//
//					predecessors[indexInCurrentBlock][numbersPredecessor[indexInCurrentBlock]++] = pred1;
//
//				}
//
//
//				int temp_pred2 = registers[pred2_reg];
//				int pred2;
//				if (!pred2_ena & temp_pred2 == -1) //If value comes from global register
//					temp_pred2 = globalVariables[basicBlockNumber & 0x1f][pred2_reg];
//
//				if (pred2_ena){
//
//					//To gather succ information on different path
//					char succ_ena = 0;
//					char succ_src;
//					char succ_isData = 0;
//
//					if (temp_pred2 == -1){ //If value comes from global register
//						if (globalVariables[basicBlockNumber & 0x1f][pred2_reg] == -1){
//							temp_pred2 = globalVariableCounter;
//							globalVariables[basicBlockNumber & 0x1f][pred2_reg] = globalVariableCounter++;
//						}
//						else
//							temp_pred2 = globalVariables[basicBlockNumber & 0x1f][pred2_reg];
//
//						//If the global register has been used in the current block, we add a control dependency
//						if (lastReaderOnGlobalCounter[pred2_reg] < 3){
//							lastReaderOnGlobalCounter[pred2_reg]++;
//							if (lastWriterOnGlobal[pred2_reg] != -1){
//								succ_ena = 1;
//								succ_src = lastWriterOnGlobal[pred2_reg];
//								numberDependencies++;
//							}
//						}
//						else{
//							int readerToEvince = lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite[pred2_reg]];
//							succ_ena = 1;
//							succ_src = readerToEvince;
//							numberDependencies++;
//						}
//						lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite[pred2_reg]] = indexInCurrentBlock;
//						lastReaderOnGlobalPlaceToWrite[pred2_reg] = (lastReaderOnGlobalPlaceToWrite[pred2_reg] + 1) % 3;
//					}
//					else{ //We add a successor to the predecessor
//						succ_ena = 1;
//						succ_src = temp_pred2;
//						succ_isData = 1;
//						numberDependencies++;
//
//					}
//
//					pred2 = temp_pred2;
//
//					if (succ_ena)
//						if (succ_isData){
//							char nbSucc = writeDataSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//							if (nbSucc == 7){
//								insertMove_ena = 1;
//								insertMove_src = pred2;
//								dest_reg = pred2_reg;
//								dest_ena = 1;
//							}
//						}
//						else
//							writeSuccessor(bytecode, succ_src, indexInCurrentBlock, *placeCode);
//
//					predecessors[indexInCurrentBlock][numbersPredecessor[indexInCurrentBlock]++] = pred2;
//				}
//
//
//				//******************************************
//				//We set the destination
//
//				int temp_destination = indexInCurrentBlock;
//				int destination;
//				if (!dest_ena & temp_destination == -1) //If value comes from global register
//					temp_destination = globalVariables[basicBlockNumber & 0x1f][dest_reg];
//
//
//				char alloc = 1;
//
//				if (dest_ena) {
//					if (globalVariables[basicBlockNumber & 0x1f][dest_reg] == -1 || rename_outsideNext_dest_alloc || insertMove_ena){
//						registers[dest_reg] = indexInCurrentBlock;
//
//						//We mark the value as a write which is potentially not read
//						currentRegistresUsageWord |= 1<<dest_reg;
//					}
//					else{
//						alloc = 0;
//						temp_destination = globalVariables[basicBlockNumber & 0x1f][dest_reg];
//						lastWriterOnGlobal[dest_reg] = indexInCurrentBlock;
//						for (int oneReader = 0; oneReader < lastReaderOnGlobalCounter[dest_reg]; oneReader++)
//							if (lastReaderOnGlobal[dest_reg][oneReader] != indexInCurrentBlock){
//								writeSuccessor(bytecode, lastReaderOnGlobal[dest_reg][oneReader], indexInCurrentBlock, *placeCode);
//								numberDependencies++;
//							}
//						lastReaderOnGlobalCounter[dest_reg] = 0;
//
//					}
//				}
//				destination = temp_destination;
//
//			//	printf("DEBUG : for instr %d : op = %x funct = %x pred1_reg = %d pred1_ena = %d pred2_reg = %d, pred2_ena = %d dest_reg = %d dest_ena = %d\n", indexInCurrentBlock, op, funct, pred1_reg, pred1_ena, pred2_reg, pred2_ena, dest_reg,dest_ena);
//
//				/***************************************************************/
//				/*  We generate the instruction  */
//				unsigned int bytecode1=0, bytecode2=0, bytecode3=0, bytecode4=0;
//
//				if (insertMove_ena){
//					bytecode1 += 0x8<<28;
//					bytecode1 += alloc << 27;
//					bytecode1 += 0x41 << 19;
//
//					bytecode1 += insertMove_src;
//
//					bytecode2 += destination << 14;
//					bytecode2 += numberDependencies << 6;
//
//					rename_isOutsideNext = wasOutside;
//					wasOutside = 1;
//				}
//				else if (wasOutside){
//					bytecode1 = rename_outsideNext_bytecode1;
//					if (rename_outsideNext_isImm){
//						bytecode1 += (rename_outsideNext_imm & 0x7ff);
//						bytecode2 += pred1 << 23;
//						bytecode2 += destination << 14;
//					}
//					else if (rename_outsideNext_isLongImm){
//						bytecode1 += ((rename_outsideNext_imm>>9) & 0x3ff);
//
//						bytecode2 += (rename_outsideNext_imm & 0x1ff) << 23;
//
//						if (pred1_ena)
//							bytecode2 += pred1 << 14;
//						else
//							bytecode2 += destination << 14;
//
//					}
//					else{
//						bytecode1 += pred1;
//
//						bytecode2 += pred2 << 23;
//						bytecode2 += destination << 14;
//					}
//
//					bytecode2 += numberDependencies << 6;
//
//
//				}
//				else if (op == R){
//					//Instrucion is R-Type
//					if (funct == SLL || funct == SRL || funct == SRA){
//
//						bytecode1 += (0x8<<28); //stage=2 type = 0
//						bytecode1 += alloc << 27;
//						bytecode1 += functBinding[funct] << 19;
//						bytecode1 += 2<<17; // i=1 br=0
//						bytecode1 += (shamt & 0x7ff);
//
//						bytecode2 += pred1 << 23;
//						bytecode2 += destination << 14;
//						bytecode2 += numberDependencies<<6;
//					}
//					else if (funct == MFHI || funct == MFLO){
//
//
//						bytecode1 += 0x8<<28;
//						bytecode1 += alloc << 27;
//						bytecode1 += 0x41 << 19;
//
//						bytecode1 += pred1;
//
//						bytecode2 += destination << 14;
//						bytecode2 += numberDependencies << 6;
//
//					}
//					else if(funct == JALR){
//						numberSuccessors = 1;
//						successor1 = basicBlockNumber + 1;
//
//						bytecode1 += (0x2<<28); //stage=0 type = 2
//						bytecode1 += 0;
//						bytecode1 += functBinding[funct] << 19;
//
//						bytecode2 += pred1 << 14;
//						bytecode2 += numberDependencies<<6;
//
//						haveJump = 1;
//						jumpID = indexInCurrentBlock;
//					}
//					else if (funct == JR){
//						if (pred1_reg == 31){
//							numberSuccessors = 0;
//							successor1 = basicBlockNumber + 1;
//
//							bytecode1 += (0x2<<28); //stage=0 type = 2
//							bytecode1 += 0;
//							bytecode1 += functBinding[funct] << 19;
//
//							bytecode2 += pred1 << 14;
//							bytecode2 += numberDependencies<<6;
//
//							haveJump = 1;
//							jumpID = indexInCurrentBlock;
//						}
//						else{
//							printf("Funct not handled %x (JR)\n", funct);
//							exit(-1);
//						}
//					}
//					else if (funct == MULT || funct == MULTU){
//
//						bytecode1 += 0xc << 28;
//						bytecode1 += 0x1 << 27;
//						bytecode1 += functBinding[MFLO] << 19;
//						bytecode1 += pred1;
//
//						bytecode2 += pred2 << 23;
//
//						bytecode2 += indexInCurrentBlock << 14;
//						bytecode2 += numberDependencies<< 6;
//
//
//						reglo = indexInCurrentBlock;
//						reghi = indexInCurrentBlock+1;
//
//						rename_outsideNext_bytecode1 += 0xc << 28; //stage=2 type = 0
//						rename_outsideNext_bytecode1 += 1 << 27;
//						rename_outsideNext_bytecode1 += functBinding[MFHI]<<19;
//
//
//						rename_outsideNext_isImm = 0;
//						rename_outsideNext_isLongImm = 0;
//						rename_outsideNext_imm = 0;
//						rename_outsideNext_pred1_ena = 1;
//						rename_outsideNext_pred1_solved = 0;
//						rename_outsideNext_pred1_reg = pred1_reg;
//						rename_outsideNext_pred2_ena = 1;
//						rename_outsideNext_pred2_reg = pred2_reg;
//						rename_outsideNext_dest_ena = 0;
//						rename_outsideNext_dest_reg = indexInCurrentBlock+1;
//						rename_outsideNext_dest_alloc = 1;
//
//						rename_isOutsideNext = 1;
//
//					}
//					else {
//
//						if (functBinding[funct] == -1 || functBinding[funct] == -2){
//							printf("Funct not handled %x in %x\n", funct, oneInstruction);
//							exit(-1);
//						}
//
//						bytecode1 += (0x8<<28); //stage=2 type = 0
//						bytecode1 += alloc << 27;
//						bytecode1 += functBinding[funct] << 19;
//						bytecode1 += 0; // i=0 br=0
//						bytecode1 += pred1;
//
//						bytecode2 += pred2 << 23;
//						bytecode2 += destination << 14;
//						bytecode2 += numberDependencies<<6;
//					}
//				}
//				else if (op == LUI){
//
//					//Instruction is fisrt translated as a movi to the destination register
//					bytecode1 += (0xa<<28); //stage=0 type = 2
//					bytecode1 += alloc << 27;
//					bytecode1 += 0x58 << 19; //opcode of movi
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += ((address>>9) & 0x3ff);
//
//					bytecode2 += (address & 0x1ff) << 23;
//					bytecode2 += destination<<14;
//					bytecode2 += numberDependencies<<6;
//
//
//
//					//Then next instruction is shl, from destination register to destination register, of 16bits
//					rename_outsideNext_bytecode1 += (0x8<<28); //stage=2 type = 0
//					rename_outsideNext_bytecode1 += alloc << 27;
//					rename_outsideNext_bytecode1 += 0x4f << 19;
//					rename_outsideNext_bytecode1 += 2<<17; // i=1 br=0
//
//					rename_outsideNext_isImm = 1;
//					rename_outsideNext_isLongImm = 0;
//					rename_outsideNext_imm = 16;
//					rename_outsideNext_pred1_ena = 1;
//					rename_outsideNext_pred1_solved = 0;
//
//					rename_outsideNext_pred1_reg = dest_reg;
//					rename_outsideNext_pred2_ena = 0;
//					rename_outsideNext_dest_ena = 1;
//					rename_outsideNext_dest_reg = dest_reg;
//					rename_outsideNext_dest_alloc = 0;
//
//					rename_isOutsideNext = 1;
//
//				}
//				else if (op == SB || op == SH || op == SW){
//					/****************************/
//					/* We update lastWriterOnMemory and add required dependencies to keep memory coherence */
//
//					for (int oneReader = 0; oneReader < lastReaderOnMemoryCounter; oneReader++){
//						writeSuccessor(bytecode, lastReaderOnMemory[oneReader], indexInCurrentBlock, *placeCode);
//						numberDependencies++;
//					}
//
//					lastReaderOnMemoryCounter = 0;
//					lastWriterOnMemory = indexInCurrentBlock;
//
//					bytecode1 += (0x10<<26); //stage=1, type=0=alloc=allocbr
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += (address & 0x7ff);
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += pred2 << 14;
//					bytecode2 += numberDependencies<<6;
//
//				}
//				else if (op == LB || op == LH || op == LW || op == LBU || op == LHU){
//					/****************************/
//					/* We update lastReaderOneMemory and add required dependencies to keep memory coherence */
//					if (lastReaderOnMemoryCounter < 4){
//						lastReaderOnMemoryCounter++;
//						if (lastWriterOnMemory != -1){
//							writeSuccessor(bytecode, lastWriterOnMemory, indexInCurrentBlock, *placeCode);
//							numberDependencies++;
//						}
//					}
//					else{
//						int readerToEvince = lastReaderOnMemory[lastReaderOnMemoryPlaceToWrite];
//						writeSuccessor(bytecode, readerToEvince, indexInCurrentBlock, *placeCode);
//						numberDependencies++;
//					}
//					lastReaderOnMemory[lastReaderOnMemoryPlaceToWrite] = indexInCurrentBlock;
//					lastReaderOnMemoryPlaceToWrite = (lastReaderOnMemoryPlaceToWrite + 1) & 0x3;
//
//					lastReaderOnMemoryCounter = 0;
//					lastWriterOnMemory = indexInCurrentBlock;
//
//					bytecode1 += (0x4<<28); //stage=1 type = 0
//					bytecode1 += alloc << 27;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += (address & 0x7ff);
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += destination << 14;
//					bytecode2 += numberDependencies<<6;
//
//				}
//				else if (op == J){
//					numberSuccessors = 1;
//					successor1 = blocksBoundaries[correctedTgtadr];
//
//					bytecode1 += (0x2<<28); //stage=0 type = 2
//					bytecode1 += 0;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//
//					bytecode2 += numberDependencies<<6;
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock;
//				}
//				else if (op == BEQ || op == BNE){
//					numberSuccessors = 2;
//					if (address >= 32768)
//						address = address - 65536;
//					successor1 = blocksBoundaries[(address+oneInstructionIndex) + 1];
//					successor2 = blocksBoundaries[oneInstructionIndex + 2];
//
//					bytecode1 += (0x8<<28); //stage=2
//					bytecode1 += 1 << 27;
//					bytecode1 += opcodeBinding[op] << 19;//opcode (here the binding will be done with cmpeq/cmpne
//					bytecode1 += 0<<17; // i=1 br=0
//					bytecode1 += pred1;
//
//					bytecode2 += pred2 << 23;
//					bytecode2 += oneInstructionIndex << 14;
//					bytecode2 += numberDependencies<<6;
//
//					rename_isOutsideNext = 1;
//					rename_outsideNext_bytecode1 = (0x2<<28) + (0x25<<19) + (2<<17);
//					rename_outsideNext_isLongImm = 1;
//					rename_outsideNext_imm = 0;
//					rename_outsideNext_pred1_ena = 1;
//					rename_outsideNext_pred1_solved = 1;
//
//					rename_outsideNext_pred2_ena = 0;
//					rename_outsideNext_dest_ena = 0;
//
//					rename_outsideNext_pred1_reg = 32;
//					rename_outsideNext_pred1 = indexInCurrentBlock; //TODO marche pas !!!
//					rename_outsideNext_dest_alloc = 0;
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock +1;
//
//
//				}
//				else if (op == BLEZ || op == BGTZ || op == REGIMM){
//					numberSuccessors = 2;
//					if (address >= 32768)
//						address = address - 65536;
//					successor1 = blocksBoundaries[(address+oneInstructionIndex) + 1];
//					successor2 = blocksBoundaries[oneInstructionIndex + 2];
//
//					bytecode1 += (0x8<<28); //stage=2
//					bytecode1 += 1 << 27;
//					bytecode1 += ((op==REGIMM) ? regimmBindings[rt] : opcodeBinding[op]) << 19;//opcode (here the binding will be done with cmpeq/cmpne
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += 0;
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += oneInstructionIndex << 14;
//					bytecode2 += numberDependencies<<6;
//
//					rename_isOutsideNext = 1;
//					rename_outsideNext_bytecode1 = (0x2<<28) + (0x25<<19) + (2<<17);
//					rename_outsideNext_isLongImm = 1;
//					rename_outsideNext_imm = address;
//					rename_outsideNext_pred1_ena = 1;
//					rename_outsideNext_pred2_ena = 1;
//					rename_outsideNext_dest_ena = 1;
//					rename_outsideNext_pred1_solved = 1;
//					rename_outsideNext_dest_alloc = 0;
//					rename_outsideNext_pred1_reg = 32;
//					rename_outsideNext_pred1 = indexInCurrentBlock; //TODO marche pas !!!
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock+1;
//
//				}
//				else if (op == JAL){
//					isCallBlock = 1;
//					numberSuccessors = 1;
//					//successor1 = basicBlockNumber + 1;
//					successor1 = proceduresBoundaries[correctedTgtadr];
//
//					bytecode1 += (0x2<<28); //stage=0 type = 2
//					bytecode1 += 0;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//
//					bytecode2 += numberDependencies<<6;
//
//					haveJump = 1;
//					jumpID = indexInCurrentBlock;
//				}
//				else{ //For all other instructions
//
//					//These instruction should not be immediate
//					if (opcodeBinding[op] == -1 || opcodeBinding[op] == -2){
//						printf("Opcode not handled at %x :  %x (%x)\n",(oneInstructionIndex>>2) + addressStart, op, oneInstruction);
//						exit(-1);
//					}
//
//					bytecode1 += (0x8<<28); //stage=2 (mult are handled elsewhere as they need to be modified) type = 0
//					bytecode1 += alloc << 27;
//					bytecode1 += opcodeBinding[op] << 19;
//					bytecode1 += 2<<17; // i=1 br=0
//					bytecode1 += (address & 0x7ff);
//
//					bytecode2 += pred1 << 23;
//					bytecode2 += destination << 14;
//					bytecode2 += numberDependencies<<6;
//
//				}
//
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock, bytecode1);
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock+4, bytecode2);
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock+8, bytecode3);
//				writeInt(bytecode, *placeCode + 16*indexInCurrentBlock+12, bytecode4);
//
//				if (debugLevel >= 1)
//					printf("Generated bytecode is %x %x %x %x\n", bytecode1, bytecode2, bytecode3, bytecode4);
//
//
//				if (!rename_droppedInstruction)
//					indexInCurrentBlock++;
//
//				if (wasOutside)
//					rename_isOutsideNext = 0;
//				else
//					oneInstructionIndex++;
//
//			}
//			while ((blocksBoundaries[oneInstructionIndex] & 0x1) == 0 && oneInstructionIndex < procedureEnd);
//
//
//			//*****************************************************************
//			//  Second loop to add dependencies to jump instruction
//
//			if (haveJump)
//				for (int oneInstructionFromBlock = 0; oneInstructionFromBlock < indexInCurrentBlock; oneInstructionFromBlock++){
//					int offset[4] = {3, 1, -1, -3};
//
//					unsigned char wordWithNbDataSucc = ((unsigned int *)bytecode)[*placeCode/4 + 4*oneInstructionFromBlock + 1]; //FIXME endianness
//					unsigned char nbSucc = wordWithNbDataSucc & 0x7;
//
//					if (nbSucc == 0){
//
//						unsigned int index = *placeCode + oneInstructionFromBlock + 16*oneInstructionFromBlock + 8 + 1 + (6 - nbSucc) + offset[(7-nbSucc) & 0x3]; //FIXME endianness
//
//						bytecode[index] = jumpID;
//						nbSucc++;
//
//						wordWithNbDataSucc = (wordWithNbDataSucc & 0xf8) + nbSucc;
//						bytecode[*placeCode/4 + 4*oneInstructionFromBlock + 1] = wordWithNbDataSucc;//FIXME endianness
//					}
//				}
//
//
//
//			successor1 >>= 2;
//			successor2 >>= 2;
//
//			//*****************************************************************
//			/* We write information on current block in the bytecode */
//
//
//
//			char typeOfFirstSucc = 0;
//			if (numberSuccessors == 0 && !haveJump){
//				typeOfFirstSucc = 254;
//				numberSuccessors++;
//				successor1 = basicBlockNumber + 1;
//			}
//
//			//First we write the block header
//
//			blockHeaders[basicBlockNumber].nbrInstr = indexInCurrentBlock;
//			blockHeaders[basicBlockNumber].numberSucc = numberSuccessors;
//			blockHeaders[basicBlockNumber].placeInstr = *placeCode;
//			blockHeaders[basicBlockNumber].placeOfSucc = *placeCode + 16*indexInCurrentBlock;
//
//			int placeSucc = *placeCode + 16*indexInCurrentBlock;
//
//
//			if (isCallBlock){
//				bytecode[placeSucc] = indexInCurrentBlock-1; //The place of the call
//				bytecode[placeSucc + 1] = 255;
//				bytecode[placeSucc + 4] = (successor1>>24) & 0xff;
//				bytecode[placeSucc + 5] = (successor1>>16) & 0xff;
//				bytecode[placeSucc + 6] = (successor1>>8) & 0xff;
//				bytecode[placeSucc + 7] = (successor1>>0) & 0xff;
//
//				placeSucc += 8;
//
//			}
//			else{
//				if (numberSuccessors >= 1){
//					bytecode[placeSucc] = jumpID; //The place of the jump
//					bytecode[placeSucc + 1] = typeOfFirstSucc;
//					bytecode[placeSucc + 2] = (successor1>>8) & 0xff;
//					bytecode[placeSucc + 3] = (successor1>>0) & 0xff;
//				}
//				if (numberSuccessors >= 2){
//					bytecode[placeSucc +8] = jumpID; //The place of the jump
//					bytecode[placeSucc + 9] = 0;
//					bytecode[placeSucc + 10] = (successor2>>8) & 0xff;
//					bytecode[placeSucc + 11] = (successor2>>0) & 0xff;
//				}
//
//			}
//
//			//We update place of code for the next BB
//			*placeCode = *placeCode +(16*indexInCurrentBlock) + numberSuccessors*8;
//
//			registersUsage[basicBlockNumber] = currentRegistresUsageWord;
//			//*****************************************************************
//
//
//			basicBlockNumber++;
//			start = oneInstructionIndex;
//			placeInBlockTable = placeInBlockTable + sizeof(struct blockHeader);
//
//		}
//
//		/* We complete information on current procedure */
//		funtionHeaders[oneProcedure].nbrBlock = basicBlockNumber;
//
//
//	}
//}
//#endif
//
