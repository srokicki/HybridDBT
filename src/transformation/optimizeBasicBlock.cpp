/*
 * optimizeBasicBlock.cpp
 *
 *  Created on: 16 nov. 2016
 *      Author: Simon Rokicki
 */

#include <cstring>
#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <isa/vexISA.h>
#include <lib/endianness.h>
#include <simulator/vexSimulator.h>
#include <stdio.h>
#include <stdlib.h>
#include <transformation/irGenerator.h>
#include <transformation/irScheduler.h>
#include <transformation/reconfigureVLIW.h>
#include <types.h>

#include <lib/log.h>

void optimizeBasicBlock(IRBlock* block, DBTPlateform* platform, IRApplication* application, unsigned int placeCode)
{

  /*********************************************************************************
   * Function optimizeBasicBlock
   * ********************************************************************************
   *
   * This function will perform basic optimization on the specified basic block.
   * It will use the irBuilder to build the IR and then use the irScheduler to export it into
   * vliw binaries.
   *
   *********************************************************************************/
  unsigned char incrementInBinaries = (platform->vliwInitialIssueWidth > 4) ? 2 : 1;
  unsigned int basicBlockStart      = block->vliwStartAddress;
  unsigned int basicBlockEnd        = block->vliwEndAddress;

  Log::logScheduleBlocks << "Block from " << std::hex << block->sourceStartAddress << " to " << std::hex
                         << block->sourceEndAddress << " is eligible for scheduling (dest " << std::hex
                         << block->sourceDestination << ") \n";

  // We ensure that the VLIW core is not inside the block being modified
  bool isCurrentlyInBlock =
      (platform->vexSimulator->PC >= basicBlockStart * 4) && (platform->vexSimulator->PC < basicBlockEnd * 4);

  if (isCurrentlyInBlock) {
    unsigned int instructionInEnd = readInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16);
    if (instructionInEnd == 0) {
      writeInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16, 0x2f);

      platform->vexSimulator->doStep(1000);
      writeInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16, 0);
    } else if (readInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16 + 4) == 0) {
      writeInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16, 0x2f);
      writeInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16 + 4, instructionInEnd);

      platform->vexSimulator->doStep(1000);
      writeInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16, instructionInEnd);
      writeInt(platform->vliwBinaries, (block->vliwEndAddress - 1) * 16 + 4, 0);

    } else {
      Log::logError << "In optimize basic block, execution is in the middle of a block and programm cannot stop "
                       "it...\n exiting...\n";
      exit(-1);
    }
  }

  /*****************************************************************
   *	Building the IR
   ************
   * In this step we call the IRGenerator to generate the IR of the block we want to schedule
   *
   *****************************************************************/
  unsigned int globalVariableCounter = 288;

  for (unsigned int oneGlobalVariable = 0; oneGlobalVariable < 128; oneGlobalVariable++)
    platform->globalVariables[oneGlobalVariable] = 256 + oneGlobalVariable;

  unsigned int originalScheduleSize = basicBlockEnd - basicBlockStart - 1;

  unsigned int irGeneratorResult = irGenerator(platform, basicBlockStart, originalScheduleSize, globalVariableCounter);

  unsigned int endAddress = irGeneratorResult >> 16;
  unsigned char blockSize = irGeneratorResult & 0xffff;

  if (endAddress < originalScheduleSize) {
    unsigned int oldEndAddress = block->vliwEndAddress;
    block->vliwEndAddress      = block->vliwStartAddress + endAddress;

    IRBlock* splittedBlock = new IRBlock(block->vliwEndAddress, oldEndAddress, block->section);
    splittedBlock->sourceStartAddress =
        block->sourceStartAddress + ((block->sourceEndAddress - block->sourceStartAddress) >> 1);
    splittedBlock->sourceEndAddress  = block->sourceEndAddress;
    splittedBlock->sourceDestination = block->sourceDestination;

    block->sourceEndAddress  = splittedBlock->sourceStartAddress;
    block->sourceDestination = splittedBlock->sourceStartAddress;

    application->addBlock(splittedBlock);
  }

  // We store old jump instruction. Its places is known from the basicBlockEnd value
  unsigned int jumpInstruction =
      readInt(platform->vliwBinaries, (block->vliwEndAddress - 2 * incrementInBinaries) * 16 + 0);
  unsigned char isRelativeJump = (jumpInstruction & 0x7f) == VEX_BR || (jumpInstruction & 0x7f) == VEX_BRF ||
                                 (jumpInstruction & 0x7f) == VEX_BGE || (jumpInstruction & 0x7f) == VEX_BLT ||
                                 (jumpInstruction & 0x7f) == VEX_BGEU || (jumpInstruction & 0x7f) == VEX_BLTU;
  unsigned char isNoJump = (jumpInstruction & 0x70) != (VEX_CALL & 0x70);
  unsigned char isPassthroughJump =
      isRelativeJump || (jumpInstruction & 0x7f) == VEX_CALL || (jumpInstruction & 0x7f) == VEX_CALLR || isNoJump;

  // We store the result in an array cause it can be used later
  block->instructions = (unsigned int*)malloc(blockSize * 4 * sizeof(unsigned int));
  for (unsigned int oneBytecodeInstr = 0; oneBytecodeInstr < blockSize; oneBytecodeInstr++) {
    block->instructions[4 * oneBytecodeInstr + 0] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 0);
    block->instructions[4 * oneBytecodeInstr + 1] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 4);
    block->instructions[4 * oneBytecodeInstr + 2] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 8);
    block->instructions[4 * oneBytecodeInstr + 3] = readInt(platform->bytecode, 16 * oneBytecodeInstr + 12);
  }

  block->nbInstr = blockSize;

  Log::logScheduleBlocks << "*************************************************************************\n";
  Log::logScheduleBlocks << "Previous version of sources:\n";
  Log::logScheduleBlocks << "*****************\n";

  block->printCode(Log::logScheduleBlocks, platform);

  Log::logScheduleBlocks << "*************************************************************************\n";
  Log::logScheduleBlocks << "Bytecode is: \n";
  Log::logScheduleBlocks << "\n*****************\n";

  block->printBytecode(Log::logScheduleBlocks);

  /*****************************************************************
   *	Scheduling the IR
   ************
   * In this step we call the IRScheduler to perform the instruction scheduling on the IR we just generated.
   *
   *****************************************************************/

  // Preparation of required memories
  for (unsigned int oneFreeRegister = 33; oneFreeRegister < 63; oneFreeRegister++)
    platform->freeRegisters[oneFreeRegister - 33] = oneFreeRegister;

  for (unsigned int onePlaceOfRegister = 0; onePlaceOfRegister < 64; onePlaceOfRegister++)
    platform->placeOfRegisters[256 + onePlaceOfRegister] = onePlaceOfRegister;
  // same for FP registers
  for (unsigned int onePlaceOfRegister = 0; onePlaceOfRegister < 64; onePlaceOfRegister++)
    platform->placeOfRegisters[256 + 64 + onePlaceOfRegister] = onePlaceOfRegister;

  // Calling scheduler
  unsigned int binaSize = irScheduler(platform, true, blockSize, placeCode, 29, platform->vliwInitialConfiguration);
  binaSize              = binaSize & 0xffff;

  // Warning: when we reschedule a block and when a value is created in a >0 latency slot at the previous cycle, we have
  // a risk of using a data not ready yet. To solve this issue we move the block start point if there is any instruction
  // in a longer latency slot at previous cycle. Even if it is inefficient it will be solved at next opt level
  // (procedure opt)

  bool needToInsert = false;
  if ((platform->vliwInitialIssueWidth <= 4 && (readInt(platform->vliwBinaries, 16 * basicBlockStart - 12) != 0 ||
                                                readInt(platform->vliwBinaries, 16 * basicBlockStart - 8) != 0))

      ||
      (platform->vliwInitialIssueWidth > 4 && (readInt(platform->vliwBinaries, 16 * basicBlockStart - 16 - 12) != 0 ||
                                               readInt(platform->vliwBinaries, 16 * basicBlockStart - 16 - 8) != 0 ||
                                               readInt(platform->vliwBinaries, 16 * basicBlockStart - 12) != 0 ||
                                               readInt(platform->vliwBinaries, 16 * basicBlockStart - 8) != 0))) {
    basicBlockStart += incrementInBinaries;
    needToInsert = true;
  }

  if (basicBlockStart + binaSize < basicBlockEnd) {

    if (needToInsert) {
      writeInt(platform->vliwBinaries, basicBlockStart * 16 + 0 - 16, 0);
      writeInt(platform->vliwBinaries, basicBlockStart * 16 + 4 - 16, 0);
      writeInt(platform->vliwBinaries, basicBlockStart * 16 + 8 - 16, 0);
      writeInt(platform->vliwBinaries, basicBlockStart * 16 + 12 - 16, 0);
      if (platform->vliwInitialIssueWidth > 4) {
        writeInt(platform->vliwBinaries, basicBlockStart * 16 + 0 - 32, 0);
        writeInt(platform->vliwBinaries, basicBlockStart * 16 + 4 - 32, 0);
        writeInt(platform->vliwBinaries, basicBlockStart * 16 + 8 - 32, 0);
        writeInt(platform->vliwBinaries, basicBlockStart * 16 + 12 - 32, 0);
      }
    }

    memcpy(&platform->vliwBinaries[4 * basicBlockStart], &platform->vliwBinaries[4 * placeCode],
           (binaSize + 1) * 4 * sizeof(unsigned int));

    for (unsigned int i = basicBlockStart + binaSize; i < block->vliwEndAddress; i++) {
      writeInt(platform->vliwBinaries, i * 16 + 0, 0);
      writeInt(platform->vliwBinaries, i * 16 + 4, 0);
      writeInt(platform->vliwBinaries, i * 16 + 8, 0);
      writeInt(platform->vliwBinaries, i * 16 + 12, 0);
    }

    // We gather jump places
    for (unsigned int oneJump = 0; oneJump < block->nbJumps; oneJump++) {
#ifdef IR_SUCC
      block->jumpPlaces[oneJump] = ((int)platform->placeOfInstr[block->jumpIds[oneJump]]) + basicBlockStart;
#else
      block->jumpPlaces[oneJump] =
          incrementInBinaries * ((int)platform->placeOfInstr[block->jumpIds[oneJump]]) + basicBlockStart;
#endif
    }

    /*****************************************************************
     *	Control Flow Correction
     ************
     * In this part we make the necessary work to make jumps correct.
     * There are three different tasks:
     * 	-> If the jump was relative jump then we have to correct the offset
     * 	-> We have to write the (corrected) jump instruction because current scheduler just compute the place but do not
     *place the instruction
     * 	-> If the jump is a passthrough (eg. if the execution can go in the part after the schedule) we need to add a
     *goto instruction to prevent the execution of a large area of nop instruction (which would remove the interest of
     *the schedule
     *
     *****************************************************************/

    // Ofset correction
    if (isRelativeJump) {

      // We read the offset and correct its sign if needed
      unsigned int offset = (jumpInstruction >> 7) & 0x1fff;
      if ((offset & 0x1000) != 0)
        offset = offset - 0x2000;

      // We compute the original destination
      unsigned int destination = block->vliwEndAddress - 2 * incrementInBinaries + (offset);

      // We compute the new offset, considering the new destination
      unsigned int newOffset = destination - (block->jumpPlaces[0]);

      Log::logScheduleBlocks << "Correction of jump at end of block. Original offset was " << offset
                             << "\n From it derivated destination " << destination << " and new offset " << newOffset
                             << "\n";

      jumpInstruction = (jumpInstruction & 0xfff0007f) | ((newOffset & 0x1fff) << 7);
    }

    // Insertion of jump instruction
    if (!isNoJump) {
      for (int oneJump = 0; oneJump < block->nbJumps; oneJump++)
        writeInt(platform->vliwBinaries, block->jumpPlaces[oneJump] * 16 + 0, jumpInstruction);
    }
    // Insertion of the new block with the goto instruction
    if (isPassthroughJump && basicBlockStart + binaSize + 1 * incrementInBinaries < block->vliwEndAddress) {
      // We need to add a jump to correct the shortening of the block.

      unsigned int insertedJump   = VEX_GOTO + (block->vliwEndAddress << 7);
      unsigned int placeOfNewJump = (basicBlockStart + binaSize) * 16;
      writeInt(platform->vliwBinaries, placeOfNewJump, insertedJump);

      // In this case, we also added a block in the design
      // We need to insert it in the set of blocks
      // IRBlock* newBlock = new IRBlock(basicBlockStart + binaSize, basicBlockStart + binaSize + 2, block->section);
      // newBlock->sourceStartAddress = 0;
      // newBlock->sourceEndAddress = 0;
      // application->addBlock(newBlock);

      Log::logScheduleBlocks << "Adding an extra block from " << (basicBlockStart + binaSize) << "to "
                             << (basicBlockStart + binaSize + 2) << "\n";
    }

    for (int i = basicBlockStart; i < basicBlockEnd; i++) {
      platform->vexSimulator->typeInstr[i - 1 + incrementInBinaries] = 1;
    }

    // We modify the stored information concerning the block
    block->vliwEndAddress = basicBlockStart + binaSize;
  } else {
    Log::logScheduleBlocks << "Schedule is dropped (" << binaSize << " cycles)\n";
  }

  /*****************************************************************/
  // This only for debug

  Log::logScheduleBlocks << "*************************************************************************\n";
  block->printCode(Log::logScheduleBlocks, platform);
  Log::logScheduleBlocks << "*************************************************************************\n";
  Log::logScheduleBlocks << "*************************************************************************\n";

  /*****************************************************************/

  if (block->blockState < IRBLOCK_STATE_SCHEDULED)
    block->blockState = IRBLOCK_STATE_SCHEDULED;
}
