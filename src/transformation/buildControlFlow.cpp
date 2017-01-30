/*
 * BuildControlFlow.cpp
 *
 *  Created on: 29 nov. 2016
 *      Author: Simon Rokicki
 */

#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>

#include <isa/irISA.h>
#include <stdlib.h>
#include <string.h>

#define TEMP_PROCEDURE_STORAGE_SIZE 50
#define TEMP_BLOCK_STORAGE_SIZE 900


int buildBasicControlFlow(DBTPlateform dbtPlateform, int mipsStartAddress, int startAddress, int endAddress, IRBlock** result){

	int sizeNewlyTranslated = endAddress-startAddress;
	mipsStartAddress = mipsStartAddress>>2;

	printf("working from %d to %d\n, equivalent MIPS address is %x\n", startAddress, endAddress, mipsStartAddress);

	//This first step consists of mapping insertions in order to easily say if isntr n is an insertion or not.

	char* insertionMap = (char*) malloc(sizeNewlyTranslated);
	for (int oneInstruction = 0; oneInstruction < sizeNewlyTranslated; oneInstruction++)
		insertionMap[oneInstruction] = 0;

	int** insertions = (int**) malloc (sizeof(int**));
	int numberInsertions = getInsertionList(mipsStartAddress-0x4000, insertions);
	for (int oneInsertion = 0; oneInsertion < numberInsertions; oneInsertion++){
		//We mark the destination as an insertion
		int index = (*insertions)[oneInsertion];
		insertionMap[index] = 1;
	}
	free(insertions);


	//TODO : adapt following code to new encoding of boundaries

	//We declare a temporary storage for storing procedures.
	IRBlock* tempBlocks = (IRBlock*) malloc(TEMP_BLOCK_STORAGE_SIZE * sizeof(IRBlock));

	int blockCounter = 0;

	int indexInMipsBinaries = 0;
	int indexInVLIWBinaries = 0;
	int previousBlockStart = 0;
	for (int oneInstruction = 0; oneInstruction < sizeNewlyTranslated; oneInstruction++){
		int offset = (indexInMipsBinaries + mipsStartAddress) >> 3;
		int bitOffset = (indexInMipsBinaries + mipsStartAddress) & 0x7;
		char blockBoundary = (dbtPlateform.blockBoundaries[offset] >> bitOffset) & 0x1;

		if (blockBoundary && !insertionMap[oneInstruction]){

			//We reach the end of a block: we create the block and mark this place as a new start
			tempBlocks[blockCounter] = IRBlock(previousBlockStart+startAddress, indexInVLIWBinaries+startAddress);
			blockCounter++;
			if (blockCounter > TEMP_BLOCK_STORAGE_SIZE){
				fprintf(stderr, "Error while building basic control flow: temporary storage size for blocks is too small and nothing has been implemented to handle this...\n");
				exit(0);
			}


			previousBlockStart = indexInVLIWBinaries;

		}

		//We increase counters: both if we are not in an insertion, only the VLIW if we are
		indexInVLIWBinaries++;
		if (!insertionMap[oneInstruction])
			indexInMipsBinaries++;
	}

	//We reach the end of a block: we create the block and mark this place as a new start
	tempBlocks[blockCounter] = IRBlock(previousBlockStart+startAddress, indexInVLIWBinaries+startAddress);
	previousBlockStart = indexInVLIWBinaries;

	blockCounter++;
	if (blockCounter > TEMP_BLOCK_STORAGE_SIZE){
		fprintf(stderr, "Error while building basic control flow: temporary storage size for blocks is too small and nothing has been implemented to handle this...\n");
		exit(0);
	}



	//We finally copy the procedures in a new memory
	*result = (IRBlock*) malloc(blockCounter*sizeof(IRBlock));
	memcpy(*result, tempBlocks, blockCounter*sizeof(IRBlock));

	//We free temporary used data
	free(insertionMap);
	free(tempBlocks);

	return blockCounter;

}

