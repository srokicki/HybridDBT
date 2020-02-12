/*
 * debugFunctions.cpp
 *
 *  Created on: 7 déc. 2016
 *      Author: Simon Rokicki
 */

#include <dbt/dbtPlateform.h>
#include <isa/riscvISA.h>
#include <isa/vexISA.h>

void debugFirstPassResult(DBTPlateform platform, int start, int end, int sourceBinariesStartAddress)
{

  /* This function is made to debug the first pass translator. It will print
   * the desassembled code of each source and generated instruction and align
   * them correctly (by considering information on insertions)
   *
   * WARNING: if this function is not called write after firstPassTranslator, it may not work properly...
   */

  int sizeNewlyTranslated = end - start;

  char* insertionMap = (char*)malloc(sizeNewlyTranslated);
  for (int oneInstruction = 0; oneInstruction < sizeNewlyTranslated; oneInstruction++)
    insertionMap[oneInstruction] = 0;

  for (int oneInsertion = 1; oneInsertion <= platform.insertions[0]; oneInsertion++) {
    // We mark the destination as an insertion
    int index           = platform.insertions[oneInsertion] - start;
    insertionMap[index] = 1;
  }

  int indexSourceBinaries = 0;
  for (int oneVLIWInstruction = 0; oneVLIWInstruction < sizeNewlyTranslated; oneVLIWInstruction++) {

    fprintf(stderr, "|| %d | ", oneVLIWInstruction + start, insertionMap[oneVLIWInstruction]);

    if (!insertionMap[oneVLIWInstruction]) {
      fprintf(stderr, "%x | ", sourceBinariesStartAddress + (indexSourceBinaries << 2));
      std::cerr << printDecodedInstrRISCV(platform.mipsBinaries[indexSourceBinaries]);
      indexSourceBinaries++;
    } else
      fprintf(stderr, "      |                     ");
    fprintf(stderr, "\t | ");
    ac_int<128, false> vliwInstr = platform.vliwBinaries[oneVLIWInstruction + start];
    std::cerr << printDecodedInstr(vliwInstr.slc<32>(0));
    fprintf(stderr, " ");
    std::cerr << printDecodedInstr(vliwInstr.slc<32>(32));
    fprintf(stderr, " ");
    std::cerr << printDecodedInstr(vliwInstr.slc<32>(64));
    fprintf(stderr, " ");
    std::cerr << printDecodedInstr(vliwInstr.slc<32>(96));
    fprintf(stderr, " ");
    fprintf(stderr, "||\n");
  }
}
