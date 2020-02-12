#include <lib/tools.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>

#define SWAP_2(x) ((((x)&0xff) << 8) | ((unsigned short)(x) >> 8))
#define SWAP_4(x) (((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | ((x) >> 24))
#define FIX_SHORT(x) (x) = SWAP_2(x)
#define FIX_INT(x) (x) = SWAP_4(x)

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

int main(int argc, char* argv[])
{
  printf("/**********************************************/ \n");
  printf("/*  Extraction of code from %s \n", argv[1]);
  printf("/**********************************************/ \n");

  void* bytecode = openReadFile(argv[1]);
  struct bytecodeHeader* bytecodeHeader =
      (struct bytecodeHeader*)readFile((void*)bytecode, 0, sizeof(struct bytecodeHeader), 1);

  int bbcnt = 0;

  printf("Bytecode contains %d procedure(s).\nProcedure table starts at offset %x in binary file.\n\n",
         bytecodeHeader->functionTableSize, bytecodeHeader->functionTablePtr);

  unsigned char* bytecodeTest = (unsigned char*)bytecode;

  for (int procedureNumber = 0; procedureNumber < bytecodeHeader->functionTableSize; procedureNumber++) {
    printf("/****************************************/ \n");
    printf("/*  Disassembling procedure %d \n", procedureNumber);
    printf("/****************************************/ \n");
    // Reading the function header
    struct functionHeader* functionHeader = (struct functionHeader*)readFile(
        (void*)bytecode, bytecodeHeader->functionTablePtr + procedureNumber * sizeof(struct functionHeader),
        sizeof(struct functionHeader), 1);

    // Reading block headers
    struct blockHeader* basicBlockHeaders = (struct blockHeader*)readFile(
        (void*)bytecode, functionHeader->blockTablePtr, sizeof(struct blockHeader), functionHeader->nbrBlock);

    // Allocating a table for block successors (TODO is it needed in nios ?)
    struct successor** blocksSuccessors =
        (struct successor**)malloc(functionHeader->nbrBlock * sizeof(struct successor*));

    //****************************************************************************************************************
    // Then we compile every trace, ignoring the control flow

    printf("Procedure contains %d basic blocks : \n", functionHeader->nbrBlock);

    for (int basicBlockNumber = 0; basicBlockNumber < functionHeader->nbrBlock; basicBlockNumber++) {

      struct blockHeader blockHeader = basicBlockHeaders[basicBlockNumber];

      printf("\nBlock %d\n", basicBlockNumber);

      int* code = (int*)readFile(bytecode, blockHeader.placeInstr, sizeof(int), blockHeader.nbrInstr * 4);

      for (int oneInstruction = 0; oneInstruction < blockHeader.nbrInstr; oneInstruction++) {

        uint32 instructionPart1 = code[4 * oneInstruction + 0];
        uint32 instructionPart2 = code[4 * oneInstruction + 1];
        uint32 instructionPart3 = code[4 * oneInstruction + 2];
        uint32 instructionPart4 = code[4 * oneInstruction + 3];

        printBytecodeInstruction(oneInstruction, instructionPart1, instructionPart2, instructionPart3,
                                 instructionPart4);
      }

      printf("Successors are blocks ");

      int placeOfSucc = blockHeader.placeOfSucc;
      //			placeOfSucc = FIX_INT(placeOfSucc);

      unsigned char realNumberOfSuccessor = blockHeader.numberSucc;
      if (realNumberOfSuccessor == 0xff)
        realNumberOfSuccessor = 1;

      printf("Place of succ is %x\n", placeOfSucc);
      struct successor* successors = (struct successor*)readFile((void*)bytecode, blockHeader.placeOfSucc,
                                                                 sizeof(struct successor), realNumberOfSuccessor);

      int succ;
      for (succ = 0; succ < realNumberOfSuccessor; succ++) {

        printf("state %d : ", successors[succ].state);
        if (successors[succ].state == 255) {
          // We are solving the case of a call block
          int dest = successors[succ].callDest;
          dest     = FIX_INT(dest);
          printf("procedure %x ", dest);

        } else if (successors[succ].state == 254) {
          short dest = successors[succ].dest;
          dest       = FIX_SHORT(dest);
          printf(" implicit %x (state = %d) ", dest, successors[succ].state);
        } else {
          short dest = successors[succ].dest;
          dest       = FIX_SHORT(dest);
          printf("%x (state %d) ", dest, successors[succ].state);
        }
      }
      printf(" \n");
    }
  }
}
