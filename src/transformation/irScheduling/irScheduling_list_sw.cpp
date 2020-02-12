/*
 * irScheduling_list_sw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#include <transformation/irScheduler.h>

#ifndef __USE_AC

const int numberOfFUs = 4;       // Correspond to the number of concurent instructions possible
int latencies[4] = {4, 4, 4, 4}; // The latencies of the different pipeline stages
const int maxLatency = 4;
unsigned int mask[4] = {0xff000000, 0xff0000, 0xff00, 0xff};

// Memory units of the architecture
unsigned int instructions[256];
unsigned int instructionsEnd[256];
unsigned char priorities[256];
unsigned char numbersOfDependencies[256];

unsigned char numbersOfRegisterDependencies[256];
unsigned char writeFreeRegister = 0;
unsigned char readFreeRegister  = 0;

unsigned char globalOptLevel;

unsigned char firstAdd;
unsigned char firstMult;
unsigned char firstMem;
unsigned char firstBr;

unsigned char readyList[64];
unsigned char readyListEna[64];
unsigned char readyListNext[64];      // Correspond to the chained list : [Enable(1), InsNumber(8)]
unsigned char readyListPriorNext[64]; // Correspond to the end of the chained list : [next(8), priorNext(8)]

unsigned int writePlace[64]  = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                               22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
                               44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
unsigned int writePlaceRead  = 0;
unsigned int writePlaceWrite = 0;

unsigned int first[4];

unsigned int last[4];

unsigned int firstPriorities[4] = {0, 0, 0, 0};

unsigned int readyNumber[4] = {0, 0, 0, 0};

unsigned char returnGetFirstNumber;
unsigned char returnGetFirstEnable;

unsigned char stall             = 0;
unsigned char aliveInstructions = 0;

unsigned char reservationTableNum[4][MAX_ISSUE_WIDTH];
unsigned char reservationTableEnable[4][MAX_ISSUE_WIDTH];

unsigned char fifoInsertReadyInstruction[64];
unsigned char fifoPlaceToWrite  = 0;
unsigned char fifoPlaceToRead   = 0;
unsigned char fifoNumberElement = 0;

unsigned char scheduledInstructions = 0;

unsigned int usedRegister[3][3 * MAX_ISSUE_WIDTH];
unsigned char numbersUsedRegister[3];

unsigned char isBrRegister[512];

#pragma hls_design
int getType(unsigned int ins)
{
  return (ins >> 30);
}

void insertReadyInstruction(unsigned char instructionNumber)
{
  unsigned int instruction          = instructions[instructionNumber];
  unsigned char instructionPriority = priorities[instructionNumber];
  unsigned char type                = getType(instruction);
  unsigned char place;

  if ((writePlaceRead + 1) == writePlaceWrite) {
    stall = 1;
  } else {
    place          = writePlace[writePlaceRead];
    writePlaceRead = (writePlaceRead + 1) & 0xbf;

    readyListEna[place] = 1;
    readyList[place]    = instructionNumber;

    if (readyNumber[type] == 0) {
      first[type] = place;
      last[type]  = place;
    } else if ((instructionPriority >> 7) & 0x1) {
      // We insert on the top of the list
      readyListNext[place] = first[type];
      first[type]          = place;

    } else {
      // we insert at the end of the list
      readyListNext[last[type]] = place;
      last[type]                = place;
    }

    readyNumber[type]++;
  }
}

void insertReadyInstructionOpt(unsigned char instructionNumber)
{
  // printf("inserting %d \n", instructionNumber);
  unsigned int instruction          = instructions[instructionNumber];
  unsigned char instructionPriority = priorities[instructionNumber];
  unsigned char type                = getType(instruction);
  unsigned char place;

  int writePlaceReadIncr = writePlaceRead + 1 - writePlaceWrite;
  if (writePlaceReadIncr == 0) {
    stall = 1;
  } else {
    place          = writePlace[writePlaceRead];
    writePlaceRead = (writePlaceRead + 1) & 0xbf;

    readyListEna[place] = 1;
    readyList[place]    = instructionNumber;

    if (instructionPriority >= firstPriorities[type] || readyNumber[type] == 0) {

      readyListNext[place]      = first[type];
      readyListPriorNext[place] = firstPriorities[type];
      first[type]               = place;
      firstPriorities[type]     = instructionPriority;
    } else {
      unsigned char currentPlace    = first[type];
      unsigned char currentPriority = instructionPriority;

      while (readyListEna[readyListNext[currentPlace]] && readyListPriorNext[currentPlace] > instructionPriority &&
             readyListPriorNext[currentPlace] < currentPriority && currentPlace != readyListNext[currentPlace]) {
        currentPriority = readyListPriorNext[currentPlace];
        currentPlace    = readyListNext[currentPlace];
      }

      readyListNext[place]        = readyListNext[currentPlace];
      readyListPriorNext[place]   = readyListPriorNext[currentPlace];
      readyListNext[currentPlace] = place;

      readyListPriorNext[currentPlace] = instructionPriority;
    }
    readyNumber[type]++;
  }
}

void getFirstInstruction(int type)
{
  if (readyNumber[type] > 0) {
    returnGetFirstNumber        = readyList[first[type]];
    returnGetFirstEnable        = 1;
    readyListEna[first[type]]   = 0;
    writePlace[writePlaceWrite] = first[type];
    firstPriorities[type]       = readyListPriorNext[first[type]];
    first[type]                 = readyListNext[first[type]];

    readyNumber[type]--;
    writePlaceWrite = (writePlaceWrite + 1) & 0xbf;
  } else if (readyNumber[2] > 0) {
    returnGetFirstNumber        = readyList[first[2]];
    returnGetFirstEnable        = 1;
    readyListEna[first[2]]      = 0;
    writePlace[writePlaceWrite] = first[2];
    firstPriorities[2]          = readyListPriorNext[first[2]];
    first[2]                    = readyListNext[first[2]];
    readyNumber[2]--;
    writePlaceWrite = (writePlaceWrite + 1) & 0xbf;
  } else {
    returnGetFirstEnable = 0;
  }
}

// The argument optLevel is here to set the difficulty of the scheduling : 0 mean that there is just a binary priority
// and 1 mean that we'll consider the entire priority value
unsigned int irScheduler_list_sw(unsigned char optLevel, unsigned char basicBlockSize, unsigned int bytecode[1024],
                                 unsigned int binaries[1024], unsigned char placeOfRegisters[512],
                                 unsigned char numberFreeRegister, unsigned char freeRegisters[64],
                                 unsigned char issue_width, uintIW way_specialisation, unsigned int placeOfInstr[256])
{

  unsigned int cycleNumber     = 0; // This is the current cycle
  int lineNumber               = 0;
  unsigned int writeInBinaries = 0;
  writeFreeRegister            = numberFreeRegister;
  int i;

  for (i = 0; i < 64; i++) {
    writePlace[i] = i;
  }
  writePlaceRead  = 0;
  writePlaceWrite = 0;

  readyNumber[0] = 0;
  readyNumber[1] = 0;
  readyNumber[2] = 0;
  readyNumber[3] = 0;

  for (i = 0; i < MAX_ISSUE_WIDTH; i++) {
    reservationTableEnable[0][i] = 0;
    reservationTableEnable[1][i] = 0;
    reservationTableEnable[2][i] = 0;
    reservationTableEnable[3][i] = 0;
  }

  stall             = 0;
  aliveInstructions = 0;

  fifoPlaceToWrite  = 0;
  fifoPlaceToRead   = 0;
  fifoNumberElement = 0;

  scheduledInstructions = 0;

  globalOptLevel = optLevel;

  for (i = 0; i < basicBlockSize; i++) {
    instructions[i]                  = bytecode[i * 4];
    instructionsEnd[i]               = (bytecode[i * 4 + 1] >> 14) & 0x3ffff;
    numbersOfDependencies[i]         = (bytecode[i * 4 + 1] >> 6) % 256;
    priorities[i]                    = (bytecode[i * 4 + 2] >> 24) % 256;
    numbersOfRegisterDependencies[i] = (bytecode[i * 4 + 1] >> 3) % 8;

    if (numbersOfDependencies[i] == 0) {
      if (fifoNumberElement == 64)
        stall = 1;
      else {
        fifoInsertReadyInstruction[fifoPlaceToWrite] = i;
        fifoPlaceToWrite                             = (fifoPlaceToWrite + 1) % 64;
        fifoNumberElement++;
        numbersOfDependencies[i]--;
      }
    }
  }

  while (1) {

    // If the fifo buffer is full, then we need to check once more every instruction in case some are ready but not
    // inserted on the ready list
    if (stall) {
      stall = 0;
      for (i = 0; i < basicBlockSize; i++) {

        if (numbersOfDependencies[i] == 0) {
          if (fifoNumberElement == 64) {
            stall = 1;
            break;
          } else {
            fifoInsertReadyInstruction[fifoPlaceToWrite] = i;
            fifoPlaceToWrite                             = (fifoPlaceToWrite + 1) % 64;
            fifoNumberElement++;
            numbersOfDependencies[i]--;
          }
        }
      }
    }

    // For every element in the ready list, we insert the instruction in the sorted ready list. Two insertion rules
    // exist, depending on the optimization level chosen.
    if (optLevel)
      for (i = 0; i < fifoNumberElement; i++) {
        insertReadyInstructionOpt(fifoInsertReadyInstruction[fifoPlaceToRead]);
        fifoPlaceToRead = (fifoPlaceToRead + 1) % 64;
      }
    else
      for (i = 0; i < fifoNumberElement; i++) {
        insertReadyInstructionOpt(fifoInsertReadyInstruction[fifoPlaceToRead]);
        fifoPlaceToRead = (fifoPlaceToRead + 1) % 64;
      }

    fifoNumberElement = 0;

    // For each stage, we schedule the first instruction (if possible) and generate the binary code

    int stage;
    for (stage = 0; stage < issue_width; stage++) {
      unsigned char type = (way_specialisation >> (stage << 1)) % 4;
      getFirstInstruction(type);

      char nextLineNumber     = (lineNumber + 1) & 0x1;
      char lineNumberForStage = (type == 3) ? nextLineNumber : lineNumber;

      if (returnGetFirstEnable) {
        reservationTableNum[lineNumberForStage][stage]    = returnGetFirstNumber;
        reservationTableEnable[lineNumberForStage][stage] = 1;
        unsigned int instruction                          = 0;
        unsigned int instructionEnd                       = 0;
        instruction                                       = instructions[returnGetFirstNumber];
        instructionEnd                                    = instructionsEnd[returnGetFirstNumber];

        // We split different information from the instruction:
        unsigned int typeCode             = (instruction >> 28) % 4;
        unsigned int alloc                = (instruction >> 27) % 2;
        unsigned int allocBr              = (instruction >> 26) % 2;
        unsigned int opCode               = (instruction >> 19) % 128;
        unsigned int isImm                = (instruction >> 18) % 2;
        unsigned int isBr                 = (instruction >> 17) % 2;
        unsigned int virtualRDest         = instructionEnd % 512;
        unsigned int virtualRIn2          = (instructionEnd >> 9) % 512;
        unsigned int virtualRIn1_imm9     = instruction % 512;
        unsigned int imm11                = instruction % 2048;
        unsigned int imm19                = ((instructionEnd >> 9) & 0x1ff) + (instruction & 0x3ff);
        unsigned int brCode               = (instruction >> 9) % 512;
        unsigned int generatedInstruction = 0;

        unsigned int dest;

        // If the instruction allocate a new register (if alloc is equal to one), we choose a new register in the list
        // of free registers
        if (alloc == 1) {
          if (numberFreeRegister != 0) {
            dest                                   = freeRegisters[readFreeRegister];
            placeOfRegisters[returnGetFirstNumber] = dest;
            isBrRegister[returnGetFirstNumber]     = 0;
            readFreeRegister                       = (readFreeRegister) + 1 & 0x3f;
            numberFreeRegister--;
          } else {
            // There is no free registers, we need to cancel the current instruction being scheduled
            fifoInsertReadyInstruction[fifoPlaceToWrite] = returnGetFirstNumber;
            fifoPlaceToWrite                             = (fifoPlaceToWrite + 1) % 64;
            fifoNumberElement++;
            reservationTableEnable[lineNumberForStage][stage] = 0;
          }

        }
        // test !
        else {
          dest                                   = placeOfRegisters[virtualRDest];
          placeOfRegisters[returnGetFirstNumber] = dest;
        }

        if (reservationTableEnable[lineNumberForStage][stage]) {
          scheduledInstructions++;
          if (typeCode == 0) {
            usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn2;
            numbersUsedRegister[lineNumberForStage]++;
            if (!isImm) {
              usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn1_imm9;
              numbersUsedRegister[lineNumberForStage]++;
            }

          } else if (typeCode == 1) {
            usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn2;
            numbersUsedRegister[lineNumberForStage]++;
            usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = brCode;
            numbersUsedRegister[lineNumberForStage]++;
            if (!isImm) {
              usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn1_imm9;
              numbersUsedRegister[lineNumberForStage]++;
            }
          }

          // Instruction generation
          generatedInstruction += opCode;
          generatedInstruction += placeOfRegisters[virtualRIn2] << 26;

          if (typeCode == 0) { // The instruction is R type
            generatedInstruction += isImm << 7;
            generatedInstruction += isBr << 8;

            if (isImm) {
              generatedInstruction += imm11 << 9;
              generatedInstruction += dest << 20; // placeOfRegisters[virtualRDest];
            } else {
              generatedInstruction += dest << 14; // placeOfRegisters[virtualRDest];
              generatedInstruction += placeOfRegisters[virtualRIn1_imm9] << 20;
            }
          } else if (typeCode == 1) { // The instruction is Rext Type
            generatedInstruction += isImm << 7;

            generatedInstruction += placeOfRegisters[brCode] << 8;

            if (isImm) {
              generatedInstruction += virtualRIn1_imm9 << 11;
              generatedInstruction += dest << 20; // placeOfRegisters[virtualRDest];
            } else {
              generatedInstruction += dest << 14; // placeOfRegisters[virtualRDest];
              generatedInstruction += placeOfRegisters[virtualRIn1_imm9] << 20;
            }
          } else { // The instruction is I Type
            generatedInstruction += dest << 26;
            placeOfRegisters[virtualRDest];
            generatedInstruction += imm19 << 7;
          }

          binaries[writeInBinaries] = generatedInstruction;
          writeInBinaries++;

        } else {
          reservationTableEnable[lineNumberForStage][stage] = 0;
          binaries[writeInBinaries]                         = 0;
          writeInBinaries++;
        }
      } else {
        reservationTableEnable[lineNumberForStage][stage] = 0;
        binaries[writeInBinaries]                         = 0;
        writeInBinaries++;
      }

      if (reservationTableEnable[lineNumberForStage][stage])
        placeOfInstr[returnGetFirstNumber] = writeInBinaries - 1;
    }

    // We check if the scheduling is over
    if (scheduledInstructions >= basicBlockSize)
      break;

    // Next cycle
    cycleNumber = cycleNumber + 1;

    char oldLineNumber = lineNumber;
    lineNumber         = (lineNumber + 1) & 0x1;

    // The last step is the commit of the executed instructions: for every instruction scheduled xx cycles earlier, we
    // reduce the register
    // dependencies of data predecessors and the dependencies of successors
    unsigned char successors[MAX_ISSUE_WIDTH * 7];
    unsigned char totalNumberOfSuccessors = 0;
    for (stage = 0; stage < issue_width; stage++) {

      unsigned char instructionFinishedNum = reservationTableNum[lineNumber][stage];
      char instructionFinishedEnable       = reservationTableEnable[lineNumber][stage];
      char numberOfSuccessor               = bytecode[(instructionFinishedNum << 2) + 1] % 8;
      if (instructionFinishedEnable) {
        int successorNumber = 0;
        for (successorNumber = 0; successorNumber < numberOfSuccessor; successorNumber++) {
          if (successorNumber < 4) {
            successors[totalNumberOfSuccessors] =
                ((bytecode[(instructionFinishedNum << 2) + 3] >> (successorNumber << 3)) % 256);
          } else
            successors[totalNumberOfSuccessors] =
                ((bytecode[(instructionFinishedNum << 2) + 2] >> ((successorNumber - 4) << 3)) % 256);

          totalNumberOfSuccessors++;
        }
      }
    }

    for (i = 0; i < numbersUsedRegister[lineNumber]; i++) {
      if ((usedRegister[lineNumber][i] < 256)) {
        numbersOfRegisterDependencies[usedRegister[lineNumber][i]]--;

        if (numbersOfRegisterDependencies[usedRegister[lineNumber][i]] == 0) {
          freeRegisters[writeFreeRegister] = placeOfRegisters[usedRegister[lineNumber][i]];
          writeFreeRegister                = (writeFreeRegister + 1) & 0x3f;
          numberFreeRegister++;
        }
      }
    }
    numbersUsedRegister[lineNumber] = 0;

    for (i = 0; i < totalNumberOfSuccessors; i++) {

      unsigned char successorName          = successors[i];
      numbersOfDependencies[successorName] = numbersOfDependencies[successorName] - 1;
      // If the number of dependencies of the successor is now to 0, we add the instruction to the list
      // of ready instructions:
      if (numbersOfDependencies[successorName] == 0) {
        if (fifoNumberElement == 64)
          stall = 1;
        else {
          fifoInsertReadyInstruction[fifoPlaceToWrite] = successorName;
          fifoPlaceToWrite                             = (fifoPlaceToWrite + 1) % 64;
          fifoNumberElement++;
          numbersOfDependencies[successorName]--;
        }
      }
    }
  }
  return cycleNumber;
}

#endif
