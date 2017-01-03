/*
 * BuildControlFlow.cpp
 *
 *  Created on: 29 nov. 2016
 *      Author: Simon Rokicki
 */

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <stdlib.h>
#include <string.h>

#define TEMP_PROCEDURE_STORAGE_SIZE 50
#define TEMP_BLOCK_STORAGE_SIZE 900


int buildBasicControlFlow(DBTPlateform dbtPlateform, int startAddress, int endAddress, IRProcedure** result){

	int sizeNewlyTranslated = endAddress-startAddress;


	//This first step consists of mapping insertions in order to easily say if isntr n is an insertion or not.
	//TODO: This could be done directly by the accelerator...

	char* insertionMap = (char*) malloc(sizeNewlyTranslated);
	for (int oneInstruction = 0; oneInstruction < sizeNewlyTranslated; oneInstruction++)
		insertionMap[oneInstruction] = 0;

	for (int oneInsertion = 1; oneInsertion <= dbtPlateform.insertions[0]; oneInsertion++){
		//We mark the destination as an insertion
		int index = dbtPlateform.insertions[oneInsertion] - startAddress;
		insertionMap[index] = 1;
	}


	//We declare a temporary storage for storing procedures.
	IRProcedure* tempProcedures = (IRProcedure*) malloc(TEMP_PROCEDURE_STORAGE_SIZE * sizeof(IRProcedure));
	IRBlock* tempBlocks = (IRBlock*) malloc(TEMP_BLOCK_STORAGE_SIZE * sizeof(IRBlock));

	int procedureCounter = 0;
	int blockCounter = 0;

	int indexInMipsBinaries = 0;
	int indexInVLIWBinaries = 0;
	int previousProcedureStart = -1;
	int previousBlockStart = -1;
	for (int oneInstruction = 0; oneInstruction < sizeNewlyTranslated; oneInstruction++){
		if (dbtPlateform.blockBoundaries[indexInMipsBinaries]){
			if (previousProcedureStart != -1){

				//We reach the end of a block: we create the block and mark this place as a new start
				tempBlocks[blockCounter] = IRBlock(previousBlockStart+startAddress, indexInVLIWBinaries+startAddress);
				blockCounter++;
				if (blockCounter > TEMP_BLOCK_STORAGE_SIZE){
					fprintf(stderr, "Error while building basic control flow: temporary storage size for blocks is too small and nothing has been implemented to handle this...\n");
					exit(0);
				}
			}

			previousBlockStart = indexInVLIWBinaries;

		}

		if (dbtPlateform.procedureBoundaries[indexInMipsBinaries]){

			if (previousProcedureStart != -1){


				//We reached the end of the procedure: we declare a new array which will hold all block declarations
				IRBlock *procedureBlocks = (IRBlock*) malloc(blockCounter*sizeof(IRBlock));

				//We do a memcopy to have those block descriptions
				memcpy(procedureBlocks, tempBlocks, blockCounter*sizeof(IRBlock));

				//We declare the procedure
				tempProcedures[procedureCounter] = IRProcedure(previousProcedureStart+startAddress, indexInVLIWBinaries+startAddress, procedureBlocks, blockCounter);

				//We store the new start of a procedure and reset the block counter
				procedureCounter++;
				if (procedureCounter > TEMP_PROCEDURE_STORAGE_SIZE){
					fprintf(stderr, "Error while building basic control flow: temporary storage size for procedures is too small and nothing has been implemented to handle this...\n");
					exit(0);
				}
			}

			blockCounter = 0;
			previousProcedureStart = indexInVLIWBinaries;

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


	//We reached the end of the procedure: we declare a new array which will hold all block declarations
	IRBlock *procedureBlocks = (IRBlock*) malloc(blockCounter*sizeof(IRBlock));

	//We do a memcopy to have those block descriptions
	memcpy(procedureBlocks, tempBlocks, blockCounter*sizeof(IRBlock));

	//We declare the procedure
	tempProcedures[procedureCounter] = IRProcedure(previousProcedureStart+startAddress, indexInVLIWBinaries+startAddress, procedureBlocks, blockCounter);

	//We store the new start of a procedure and reset the block counter
	previousProcedureStart = indexInVLIWBinaries;
	procedureCounter++;
	blockCounter = 0;
	if (procedureCounter > TEMP_PROCEDURE_STORAGE_SIZE){
		fprintf(stderr, "Error while building basic control flow: temporary storage size for procedures is too small and nothing has been implemented to handle this...\n");
		exit(0);
	}


	//We finally copy the procedures in a new memory
	*result = (IRProcedure*) malloc(procedureCounter*sizeof(IRProcedure));
	memcpy(*result, tempProcedures, procedureCounter*sizeof(IRProcedure));

	//We free temporary used data
	free(insertionMap);
	free(tempProcedures);
	free(tempBlocks);

	return procedureCounter;

}

