/*
 * BuildControlFlow.cpp
 *
 *  Created on: 29 nov. 2016
 *      Author: Simon Rokicki
 */

#include <stdio.h>

#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>
#include <dbt/profiling.h>

#include <isa/irISA.h>
#include <isa/vexISA.h>

#include <stdlib.h>
#include <string.h>

#include <lib/endianness.h>
#include <types.h>

#include <lib/log.h>
#include <transformation/irGenerator.h>

#define TEMP_PROCEDURE_STORAGE_SIZE 50
#define TEMP_BLOCK_STORAGE_SIZE 4000

void buildBasicControlFlow(DBTPlateform* dbtPlateform, int section, int mipsStartAddress, int sectionStartAddress,
                           int startAddress, int endAddress, IRApplication* application, Profiler* profiler)
{

  char offsetInBinaries   = (dbtPlateform->vliwInitialIssueWidth > 4) ? 2 : 1;
  int sizeNewlyTranslated = (endAddress - startAddress) / offsetInBinaries;
  mipsStartAddress        = mipsStartAddress >> 2;

  // This first step consists of mapping insertions in order to easily say if isntr n is an insertion or not.

  char* insertionMap = (char*)malloc(sizeNewlyTranslated);
  for (int oneInstruction = 0; oneInstruction < sizeNewlyTranslated; oneInstruction++)
    insertionMap[oneInstruction] = 0;

  int** insertions     = (int**)malloc(sizeof(int**));
  int numberInsertions = getInsertionList((sectionStartAddress >> 2) - mipsStartAddress, insertions); // TODO
  for (int oneInsertion = 0; oneInsertion < numberInsertions; oneInsertion++) {
    // We mark the destination as an insertion
    int index           = (*insertions)[oneInsertion] / offsetInBinaries;
    insertionMap[index] = 1;
  }
  free(insertions);

  // We declare a temporary storage for storing procedures.

  int indexInMipsBinaries      = 0;
  int indexInVLIWBinaries      = 0;
  int previousBlockStart       = 0;
  int previousBlockStartSource = 0;

  // Online resolution of jumps
  unsigned int unresolvedJumpIndex = 0;

  for (int oneInstruction = 0; oneInstruction <= sizeNewlyTranslated; oneInstruction++) {
    int offset         = (indexInMipsBinaries + ((sectionStartAddress >> 2) - mipsStartAddress));
    char blockBoundary = 1;
    if (oneInstruction != sizeNewlyTranslated)
      blockBoundary = (dbtPlateform->blockBoundaries[offset]);

    char isInsertion = (oneInstruction == sizeNewlyTranslated) ? 0 : insertionMap[oneInstruction];
    if (blockBoundary && !isInsertion && previousBlockStart < indexInVLIWBinaries) {

      /******************************************************************************************
       ******************************  Creation and insertion of the block
       ******************************************************************************************
       * A new boundary has been reached, we have to create the IRBlock, to give him its values
       * (eg. vliwStartAddress, vliwEndAddress, sourceStartAddress, sourceEndAddress etc...) and
       * we insert it in the IRApplication
       *
       ******************************************************************************************/

      // We reach the end of a block: we create the block and mark this place as a new start
      IRBlock* newBlock = new IRBlock(previousBlockStart + startAddress, indexInVLIWBinaries + startAddress, section);
      newBlock->sourceStartAddress = previousBlockStartSource + (sectionStartAddress >> 2);
      newBlock->sourceEndAddress   = indexInMipsBinaries + (sectionStartAddress >> 2);
      newBlock->sourceDestination  = -1;

      application->addBlock(newBlock);

      /******************************************************************************************
       ******************************  Branch resolution
       ******************************************************************************************
       * If we find that there is an unresolved jump in this block, we find the correct location and
       * solve it if possible (eg. if the destination has already been translated.
       *
       ******************************************************************************************/
      unsigned int oneJumpSource             = dbtPlateform->unresolvedJumps_src[unresolvedJumpIndex];
      unsigned int oneJumpInitialDestination = dbtPlateform->unresolvedJumps[unresolvedJumpIndex];
      unsigned int oneJumpType               = dbtPlateform->unresolvedJumps_type[unresolvedJumpIndex];

      // We build the jumpType to be inserted in the block
      jumpType cleansedOneJumpType;
      if (((oneJumpType & 0x7f) == VEX_BR) || ((oneJumpType & 0x7f) == VEX_BRF) || ((oneJumpType & 0x7f) == VEX_BLT) ||
          ((oneJumpType & 0x7f) == VEX_BGE) || ((oneJumpType & 0x7f) == VEX_BLTU) ||
          ((oneJumpType & 0x7f) == VEX_BGEU)) {
        cleansedOneJumpType = CONDITIONAL_JUMP;
      } else if ((oneJumpType & 0x7f) == VEX_GOTO) {
        cleansedOneJumpType = DIRECT_JUMP;
      } else if ((oneJumpType & 0x7f) == VEX_CALL) {
        cleansedOneJumpType = DIRECT_CALL;
      } else if ((oneJumpType & 0x7f) == VEX_CALLR) {
        cleansedOneJumpType = INDIRECT_CALL;
      } else if ((oneJumpType & 0x7f) == VEX_GOTOR) {
        cleansedOneJumpType = INDIRECT_JUMP;
      } else {
        cleansedOneJumpType = NO_JUMP;
      }

      if (newBlock->vliwEndAddress - 2 * offsetInBinaries == oneJumpSource) {
        // We save the destination and the jump type
        newBlock->sourceDestination = oneJumpInitialDestination + (sectionStartAddress >> 2);
        newBlock->setJump(cleansedOneJumpType, newBlock->sourceDestination, -1, -1);

        unsigned char isAbsolute =
            ((oneJumpType & 0x7f) != VEX_BR) && ((oneJumpType & 0x7f) != VEX_BRF && (oneJumpType & 0x7f) != VEX_BLTU) &&
            ((oneJumpType & 0x7f) != VEX_BGE && (oneJumpType & 0x7f) != VEX_BGEU) && ((oneJumpType & 0x7f) != VEX_BLT);
        int destinationInVLIWFromNewMethod = solveUnresolvedJump(
            dbtPlateform, oneJumpInitialDestination + ((sectionStartAddress >> 2) - mipsStartAddress));

        if (destinationInVLIWFromNewMethod == -1) {
          // In this case, the jump cannot be resolved because the destination block is not translated yet.
          // We store information concerning the destination and it will be resolved later

          int numberUnresolvedJumps = unresolvedJumpsArray[0];
          unresolvedJumpsArray[1 + numberUnresolvedJumps] =
              oneJumpInitialDestination + ((sectionStartAddress >> 2) - mipsStartAddress);
          unresolvedJumpsTypeArray[1 + numberUnresolvedJumps]   = oneJumpType;
          unresolvedJumpsSourceArray[1 + numberUnresolvedJumps] = oneJumpSource;

          unresolvedJumpsArray[0] = numberUnresolvedJumps + 1;
        } else {
          // The jump can be resolved
          int immediateValue =
              (isAbsolute) ? (destinationInVLIWFromNewMethod) : ((destinationInVLIWFromNewMethod - oneJumpSource));
          int mask = (isAbsolute) ? 0x7ffff : 0x1fff;
          writeInt(dbtPlateform->vliwBinaries, 16 * (oneJumpSource), oneJumpType + ((immediateValue & mask) << 7));

          if (destinationInVLIWFromNewMethod != 0) {
            unsigned int instructionBeforePreviousDestination =
                readInt(dbtPlateform->vliwBinaries, 16 * (destinationInVLIWFromNewMethod - 1) + 12);
            if (instructionBeforePreviousDestination != 0)
              writeInt(dbtPlateform->vliwBinaries, 16 * (oneJumpSource + 1 * offsetInBinaries) + 12,
                       instructionBeforePreviousDestination);
          }
        }

        unresolvedJumpIndex++;

        // We check if the boundary was already marked

        int offsetForDestination = (oneJumpInitialDestination + ((sectionStartAddress >> 2) - mipsStartAddress));
        int sectionOfDestination = offsetForDestination >> 10;

        if (sectionOfDestination < section) {

          if (application->getBlock(newBlock->sourceDestination) == NULL) {

            unsigned int containingBlockStart = newBlock->sourceDestination - 1;
            while (application->getBlock(containingBlockStart) == NULL)
              containingBlockStart--;

            IRBlock* blockToSplit = application->getBlock(containingBlockStart);

            IRBlock* splittedBlock =
                new IRBlock(destinationInVLIWFromNewMethod, blockToSplit->vliwEndAddress, sectionOfDestination);
            // We set metainfo for new block
            splittedBlock->sourceStartAddress = newBlock->sourceDestination;
            splittedBlock->sourceEndAddress   = blockToSplit->sourceEndAddress;
            splittedBlock->sourceDestination  = blockToSplit->sourceDestination;
            splittedBlock->jumpIds            = blockToSplit->jumpIds;
            splittedBlock->jumpPlaces         = blockToSplit->jumpPlaces;
            splittedBlock->jumpTypes          = blockToSplit->jumpTypes;
            splittedBlock->successors         = blockToSplit->successors;
            splittedBlock->nbJumps            = blockToSplit->nbJumps;
            splittedBlock->nbSucc             = blockToSplit->nbSucc;

            // We set meta info for old block
            blockToSplit->sourceEndAddress  = newBlock->sourceDestination;
            blockToSplit->sourceDestination = -1;
            blockToSplit->vliwEndAddress    = destinationInVLIWFromNewMethod;
            blockToSplit->setJump(NO_JUMP, splittedBlock->sourceStartAddress, -1, -1);

            application->addBlock(splittedBlock);
          }
        }
      } else {
        // No jump found
        newBlock->setJump(NO_JUMP, -1, -1, -1);
      }

      /******************************************************************************************/
      // We update interLoop values
      previousBlockStart       = indexInVLIWBinaries;
      previousBlockStartSource = indexInMipsBinaries;

    } else if (!isInsertion && previousBlockStart < indexInVLIWBinaries &&
               indexInVLIWBinaries - previousBlockStart >= 250) {

      /******************************************************************************************
       ******************************  Creation and insertion of the block
       ******************************************************************************************
       * A new boundary has been reached, we have to create the IRBlock, to give him its values
       * (eg. vliwStartAddress, vliwEndAddress, sourceStartAddress, sourceEndAddress etc...) and
       * we insert it in the IRApplication
       *
       ******************************************************************************************/

      // We reach the end of a block: we create the block and mark this place as a new start
      IRBlock* newBlock = new IRBlock(previousBlockStart + startAddress, indexInVLIWBinaries + startAddress, section);
      newBlock->sourceStartAddress = previousBlockStartSource + (sectionStartAddress >> 2);
      newBlock->sourceEndAddress   = indexInMipsBinaries + (sectionStartAddress >> 2);
      newBlock->sourceDestination  = -1;
      newBlock->setJump(NO_JUMP, newBlock->sourceEndAddress, -1, -1);

      application->addBlock(newBlock);

      /******************************************************************************************/
      // We update interLoop values
      previousBlockStart       = indexInVLIWBinaries;
      previousBlockStartSource = indexInMipsBinaries;
    }

    // We increase counters: both if we are not in an insertion, only the VLIW if we are
    indexInVLIWBinaries += offsetInBinaries;
    if (!isInsertion && indexInMipsBinaries < 1024)
      indexInMipsBinaries++;
  }

  // We free temporary used data
  free(insertionMap);
}

int compare_blocks(const void* a, const void* b)
{
  const IRBlock** blocka = (const IRBlock**)a;
  const IRBlock** blockb = (const IRBlock**)b;

  return ((*blocka)->sourceStartAddress > (*blockb)->sourceStartAddress) -
         ((*blocka)->sourceStartAddress < (*blockb)->sourceStartAddress);
}

int buildAdvancedControlFlow(DBTPlateform* platform, IRBlock* startBlock, IRApplication* application)
{

  IRBlock* blocksToStudy[TEMP_BLOCK_STORAGE_SIZE];
  int numberBlockToStudy = 1;
  blocksToStudy[0]       = startBlock;
  IRBlock* entryBlock    = startBlock;

  IRBlock* blockInProcedure[TEMP_BLOCK_STORAGE_SIZE];
  int numberBlockInProcedure = 0;

  while (numberBlockToStudy != 0) {

    if (numberBlockToStudy > TEMP_BLOCK_STORAGE_SIZE) {
      for (int oneBlockInProc = 0; oneBlockInProc < numberBlockInProcedure; oneBlockInProc++)
        blockInProcedure[oneBlockInProc]->blockState = IRBLOCK_ERROR_PROC;

      Log::logScheduleProc << "Leaving because the generated procedure is too large (" << numberBlockInProcedure
                           << ")\n";
      return -1;
    }

    IRBlock* currentBlock = blocksToStudy[numberBlockToStudy - 1];
    numberBlockToStudy--;

    // If the block is marked as IRBLOCK_PROC, we ensure that it is owned by the procedure being built
    if (currentBlock->blockState == IRBLOCK_PROC) {
      bool isValidConstruction = false;
      for (int oneAlreadySeenBlock = 0; oneAlreadySeenBlock < numberBlockInProcedure; oneAlreadySeenBlock++) {
        IRBlock* alreadySeenBlock = blockInProcedure[oneAlreadySeenBlock];
        if (alreadySeenBlock == currentBlock)
          isValidConstruction = true;
      }
      if (!isValidConstruction) {
        // If the block belongs to another procedure, we cancel the analysis
        for (int oneBlockInProc = 0; oneBlockInProc < numberBlockInProcedure; oneBlockInProc++)
          blockInProcedure[oneBlockInProc]->blockState = IRBLOCK_ERROR_PROC;

        Log::logScheduleProc << "Leaving because the generated procedure is malformed\n";
        return -3;
      } else {
        continue;
      }
    }

    if (numberBlockInProcedure >= TEMP_BLOCK_STORAGE_SIZE) {
      // If the procedure has too many blocks, we cancel the analysis
      for (int oneBlockInProc = 0; oneBlockInProc < numberBlockInProcedure; oneBlockInProc++)
        blockInProcedure[oneBlockInProc]->blockState = IRBLOCK_ERROR_PROC;

      Log::logScheduleProc << "Leavign because of too large proc (" << numberBlockInProcedure << ")\n";
      return -5;
    }

    if (currentBlock->blockState == IRBLOCK_ERROR_PROC) {
      // If the procedure has too many blocks, we cancel the analysis
      for (int oneBlockInProc = 0; oneBlockInProc < numberBlockInProcedure; oneBlockInProc++)
        blockInProcedure[oneBlockInProc]->blockState = IRBLOCK_ERROR_PROC;

      Log::logScheduleProc << "Leavign because of too erroneous block (" << numberBlockInProcedure << ")\n";
      return -2;
    }

    blockInProcedure[numberBlockInProcedure] = currentBlock;
    numberBlockInProcedure++;
    currentBlock->blockState = IRBLOCK_PROC;
    /******************************************************************************************
     ******************************  Successor resolution
     ******************************************************************************************
     * In this part we will go through all blocks in order to find the object which represent successors of the current
     *block. This step is currently expensive and may be simplified by creating a map between addresses and blocks or
     *maybe a dichotomy search function (if they are correctly sorted according to their start address).
     *
     ******************************************************************************************/

    for (int oneSucc = 0; oneSucc < currentBlock->nbSucc; oneSucc++) {
      blocksToStudy[numberBlockToStudy] = application->getBlock(currentBlock->successors[oneSucc]);
      numberBlockToStudy++;

      if (application->getBlock(currentBlock->successors[oneSucc]) == NULL) {
        Log::logScheduleProc
            << "In build advanced control flow, a successor has not been found. Cancelling procedure construction !\n";
        for (int oneBlockInProc = 0; oneBlockInProc < numberBlockInProcedure; oneBlockInProc++)
          blockInProcedure[oneBlockInProc]->blockState = IRBLOCK_ERROR_PROC;

        return -4;
      }
    }

    // We search for blowk which may jump to this one
    for (auto& block : *application) {
      for (int oneSucc = 0; oneSucc < block.nbSucc; oneSucc++) {
        if (block.successors[oneSucc] == currentBlock->sourceStartAddress) {
          blocksToStudy[numberBlockToStudy] = &block;
          numberBlockToStudy++;
        }
      }
    }

    // We actualize if needed the entryBlock TODO:check this
    if (entryBlock->vliwStartAddress > currentBlock->vliwStartAddress) {
      entryBlock = currentBlock;
    }
  }

  // We instanciate the procedure
  IRProcedure* procedure           = new IRProcedure(entryBlock, numberBlockInProcedure);
  procedure->nbBlock               = numberBlockInProcedure;
  procedure->blocks                = (IRBlock**)malloc(numberBlockInProcedure * sizeof(IRBlock*));
  procedure->configuration         = platform->vliwInitialConfiguration;
  procedure->previousConfiguration = procedure->configuration;

  memcpy(procedure->blocks, blockInProcedure, numberBlockInProcedure * sizeof(IRBlock*));
  qsort(procedure->blocks, numberBlockInProcedure, sizeof(IRBlock*), compare_blocks);

  // TODO code a better sort function
  /*	int previousIndex = 0;
          for (int oneBlock = 0; oneBlock<numberBlockInProcedure; oneBlock++){
                  int minBlock = 0x1000000;
                  int minBlockIndex = 0;
                  for (int oneOtherBlock = 0; oneOtherBlock<numberBlockInProcedure; oneOtherBlock++){
                          if (blockInProcedure[oneOtherBlock]->sourceStartAddress > previousIndex &&
     blockInProcedure[oneOtherBlock]->sourceStartAddress < minBlock){ minBlock =
     blockInProcedure[oneOtherBlock]->sourceStartAddress; minBlockIndex = oneOtherBlock;
                          }

                  }
                  procedure->blocks[oneBlock] = blockInProcedure[minBlockIndex];
                  previousIndex = minBlock;
          }*/

  procedure->entryBlock = procedure->blocks[0];
  application->addProcedure(procedure);

  // We create IR for all blocks annd modify the location of their unique reference
  for (unsigned int oneBasicBlock = 0; oneBasicBlock < procedure->nbBlock; oneBasicBlock++) {
    IRBlock* block = procedure->blocks[oneBasicBlock];

    block->procedureSourceStartAddress = procedure->entryBlock->sourceStartAddress;
    block->reference                   = &(procedure->blocks[oneBasicBlock]);

    if (block->nbInstr == 0) {

      int globalVariableCounter = 288;

      for (int oneGlobalVariable = 0; oneGlobalVariable < 128; oneGlobalVariable++)
        platform->globalVariables[oneGlobalVariable] = 256 + oneGlobalVariable;

      int originalScheduleSize = block->vliwEndAddress - block->vliwStartAddress - 1;

      unsigned int irGeneratorResult =
          irGenerator(platform, block->vliwStartAddress, originalScheduleSize, globalVariableCounter);
      int blockSize = irGeneratorResult & 0xffff;

      block->instructions = (unsigned int*)malloc(blockSize * 4 * sizeof(unsigned int));
      for (int oneBytecodeInstr = 0; oneBytecodeInstr < blockSize; oneBytecodeInstr++) {
        block->instructions[4 * oneBytecodeInstr + 0] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 0);
        block->instructions[4 * oneBytecodeInstr + 1] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 4);
        block->instructions[4 * oneBytecodeInstr + 2] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 8);
        block->instructions[4 * oneBytecodeInstr + 3] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 12);
      }

      block->nbInstr = blockSize;
    }

    // We check if we find a jump as last instruction
    if (block->nbJumps == 1) {
      block->jumpIds[0] = block->nbInstr - 1;
    }

    block->oldVliwStartAddress = block->vliwStartAddress;
    block->blockState          = IRBLOCK_PROC;
  }

  return 0;
}
