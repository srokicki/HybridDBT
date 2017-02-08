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

#include <types.h>
#include <lib/endianness.h>

#define TEMP_PROCEDURE_STORAGE_SIZE 50
#define TEMP_BLOCK_STORAGE_SIZE 900


void buildBasicControlFlow(DBTPlateform dbtPlateform, int section, int mipsStartAddress, int startAddress, int endAddress, IRApplication *application){

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

	//We declare a temporary storage for storing procedures.
	application->blocksInSections[section] = (IRBlock**) malloc(TEMP_BLOCK_STORAGE_SIZE * sizeof(IRBlock**));
	application->numbersAllocatedBlockInSections[section] = TEMP_BLOCK_STORAGE_SIZE;

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
			IRBlock *newBlock = new IRBlock(previousBlockStart+startAddress, indexInVLIWBinaries+startAddress, section);
			application->addBlock(newBlock, section);

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
	IRBlock *newBlock = new IRBlock(previousBlockStart+startAddress, indexInVLIWBinaries+startAddress, section);
	application->addBlock(newBlock, section);
	previousBlockStart = indexInVLIWBinaries;

	blockCounter++;
	if (blockCounter > TEMP_BLOCK_STORAGE_SIZE){
		fprintf(stderr, "Error while building basic control flow: temporary storage size for blocks is too small and nothing has been implemented to handle this...\n");
		exit(0);
	}

	//We free temporary used data
	free(insertionMap);


}

void buildAdvancedControlFlow(DBTPlateform *platform, IRBlock *startBlock, IRApplication *application){

	IRBlock *blocksToStudy[20];
	int numberBlockToStudy = 1;
	blocksToStudy[0] = startBlock;
	IRBlock *entryBlock = startBlock;

	IRBlock *blockInProcedure[TEMP_PROCEDURE_STORAGE_SIZE];
	int numberBlockInProcedure = 0;



	while (numberBlockToStudy != 0){

		IRBlock *currentBlock = blocksToStudy[numberBlockToStudy-1];
		numberBlockToStudy--;

		unsigned int endAddress = currentBlock->vliwEndAddress;
		unsigned int jumpInstruction = readInt(platform->vliwBinaries, (endAddress-2)*16);

		fprintf(stderr, "Working on block %x from %d to %d. (nbsucc %d) (jumpInstr = %x)\n", (long int) currentBlock, currentBlock->vliwStartAddress, currentBlock->vliwEndAddress, currentBlock->nbSucc, jumpInstruction);


		if (currentBlock->nbSucc != -1)
			continue;

		if (numberBlockInProcedure>TEMP_PROCEDURE_STORAGE_SIZE){
			fprintf(stderr, "Error while building advanced control flow: temporary storage size for blocks is too small and nothing has been implemented to handle this...\n");
			exit(0);
		}

		blockInProcedure[numberBlockInProcedure] = currentBlock;
		numberBlockInProcedure++;

		//We only consider successors if they are after a branch or a goto instruction.
		//If we meet a CALL or a GOTOR (return) instruction we consider it to be the end of the 'procedure'
		// and thus we end the analysis.

		if (((jumpInstruction & 0x3f) == VEX_BR) || ((jumpInstruction & 0x3f) == VEX_BRF)){
			//In this case we have to find one block with its start address, the other one is the next block

			//We compute the destination(s)
			int offset = (jumpInstruction >> 7) & 0x7ffff;
			if ((offset & 0x40000) != 0)
				offset = offset - 0x80000;
			int successor1Start = endAddress-2 + (offset>>2);
			int successor2Start = endAddress;

			//We find the corresponding blocks
			for (int oneSection = 0; oneSection<application->numberOfSections; oneSection++){
				for (int oneBlock = 0; oneBlock < application->numbersBlockInSections[oneSection]; oneBlock++){
					if (application->blocksInSections[oneSection][oneBlock]->vliwStartAddress == successor1Start)
						currentBlock->successor1 = application->blocksInSections[oneSection][oneBlock];
					else if (application->blocksInSections[oneSection][oneBlock]->vliwStartAddress == successor2Start)
						currentBlock->successor2 = application->blocksInSections[oneSection][oneBlock];
				}
			}
			fprintf(stderr, "FOund block  as a succ 2 (%d and %d)\n ",currentBlock->successor1->vliwStartAddress, currentBlock->successor2->vliwStartAddress);

			//We store the result
			currentBlock->nbSucc = 2;
			blocksToStudy[numberBlockToStudy] = currentBlock->successor1;
			blocksToStudy[numberBlockToStudy+1] = currentBlock->successor2;
			numberBlockToStudy += 2;


			//We actualize if needed the entryBlock
			if (entryBlock == currentBlock->successor1 || entryBlock == currentBlock->successor2)
				entryBlock = currentBlock;


		}
		else if ((jumpInstruction & 0x3f) == VEX_GOTO){
			//In this case there is only one successor which is the destination of the GOTO

			//We compute the destination(s)
			int destination = (jumpInstruction >> 7) & 0x7ffff;
			if ((destination & 0x40000) != 0)
				destination = destination - 0x80000;
			int successor1Start = destination>>2;

			//We find the corresponding block
			for (int oneSection = 0; oneSection<application->numberOfSections; oneSection++){
				for (int oneBlock = 0; oneBlock < application->numbersBlockInSections[oneSection]; oneBlock++){
					IRBlock *block = application->blocksInSections[oneSection][oneBlock];

					if (block->vliwStartAddress == successor1Start){
						currentBlock->successor1 = block;
						break;
					}
				}
			}

			//We store the result
			currentBlock->nbSucc = 1;
			blocksToStudy[numberBlockToStudy] = currentBlock->successor1;
			numberBlockToStudy++;

			fprintf(stderr, "found 1 successor %d\n", currentBlock->successor1->vliwStartAddress);


			//We actualize if needed the entryBlock
			if (entryBlock == currentBlock->successor1)
				entryBlock = currentBlock;

		}
		else if (((jumpInstruction & 0x3f) != VEX_CALL) && ((jumpInstruction & 0x3f) != VEX_CALLR) && ((jumpInstruction & 0x3f) != VEX_GOTOR) && ((jumpInstruction & 0x3f) != VEX_STOP)){
			//If there is no jump instruction at the end of the block then the successor is the next block

			//We compute the destination(s)
			int successor1Start = endAddress;

			//We find the corresponding block
			for (int oneSection = 0; oneSection<application->numberOfSections; oneSection++){
				for (int oneBlock = 0; oneBlock < application->numbersBlockInSections[oneSection]; oneBlock++){
					IRBlock *block = application->blocksInSections[oneSection][oneBlock];

					if (block->vliwStartAddress == successor1Start){
						currentBlock->successor1 = block;
						break;
					}
				}
			}

			//We store the result
			currentBlock->nbSucc = 1;
			blocksToStudy[numberBlockToStudy] = currentBlock->successor1;
			numberBlockToStudy++;

			fprintf(stderr, "found 1 successor %d\n", currentBlock->successor1->vliwStartAddress);


			//We actualize if needed the entryBlock
			if (entryBlock == currentBlock->successor1)
				entryBlock = currentBlock;

		}
		else{
			currentBlock->nbSucc = 0;
			fprintf(stderr, "There is no successor (jump instr was%x)\n");

		}
	}

	//We instanciate the procedure
	IRProcedure *procedure = new IRProcedure(entryBlock, numberBlockInProcedure);
	procedure->blocks = (IRBlock**) malloc(numberBlockInProcedure * sizeof(IRBlock*));

	memcpy(procedure->blocks, blockInProcedure, numberBlockInProcedure * sizeof(IRBlock*));

	application->addProcedure(procedure);

	/* Debugging : we print the dot graph of the procedure we created
	 *
	 * This will be commented in the final version to prevent having too much printing
	 */

	fprintf(stderr, "digraph{\n");
	for (int oneBlockInProcedure = 0; oneBlockInProcedure < procedure->nbBlock; oneBlockInProcedure++){
		fprintf(stderr, "node_%d;\n", procedure->blocks[oneBlockInProcedure]->vliwStartAddress);
	}
	for (int oneBlockInProcedure = 0; oneBlockInProcedure < procedure->nbBlock; oneBlockInProcedure++){

		if (blockInProcedure[oneBlockInProcedure]->nbSucc >= 1)
			fprintf(stderr, "node_%d -> node_%d;\n", procedure->blocks[oneBlockInProcedure]->vliwStartAddress, procedure->blocks[oneBlockInProcedure]->successor1->vliwStartAddress);
		if (blockInProcedure[oneBlockInProcedure]->nbSucc >= 2)
			fprintf(stderr, "node_%d -> node_%d;\n", procedure->blocks[oneBlockInProcedure]->vliwStartAddress, procedure->blocks[oneBlockInProcedure]->successor2->vliwStartAddress);

	}
	fprintf(stderr, "}\n");



	fprintf(stderr, "Analysis done ! \n");

}
