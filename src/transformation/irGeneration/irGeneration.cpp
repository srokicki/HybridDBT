/*
 * irGeneration.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#include <cstdio>
#include <cstdlib>
#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>
#include <transformation/irGenerator.h>

#include <isa/irISA.h>
#include <isa/vexISA.h>
#include <lib/dbtProfiling.h>

#include <lib/log.h>

/****************************************************************************
 *  Definition of the procedure irGenerator
 ****************************************************************************
 *
 *  This procedure is the one that is visible from the dbt framework, the one defined
 *  in the corresponding header file.
 *  This procedure will call the hardware accelerator or its cpp implementation.
 ****************************************************************************/

#ifndef __CATAPULT
// Performance simulation
int timeTakenIRGeneration;
#endif

unsigned int irGenerator(DBTPlateform* platform, unsigned int addressInBinaries, unsigned int blockSize,
                         unsigned int globalVariableCounter)
{

  // Modelization of optimization time : here we need 6 cycles to generate IR for one instruction
  if (platform->dbtType == DBT_TYPE_HW) {
    platform->optimizationCycles += blockSize * 6;
    platform->optimizationEnergy += ((int)blockSize) * 6 * 11.47f;
  } else {
    platform->optimizationCycles += blockSize * 889;
    platform->optimizationEnergy += ((int)blockSize) * 889 * 14.48f;
  }

#ifdef __SW_HW_SIM

  /********************************************************************************************
   * First version of sources for _SW_HW_SIM
   *
   * The pass is done in SW and in HW and the outputs are compared. If they are different
   * it returns an error message.
   * This should be the default mode
   ********************************************************************************************/

  ac_int<128, false>* localBytecode      = (ac_int<128, false>*)malloc(256 * sizeof(ac_int<128, false>));
  ac_int<128, false>* localVliwBinaries  = (ac_int<128, false>*)malloc(MEMORY_SIZE * sizeof(ac_int<128, false>));
  ac_int<32, true>* localGlobalVariables = (ac_int<32, true>*)malloc(128 * sizeof(ac_int<32, true>));

  acintMemcpy(localBytecode, platform->bytecode, 256 * 16);
  acintMemcpy(localVliwBinaries, platform->vliwBinaries, MEMORY_SIZE * 16);
  acintMemcpy(localGlobalVariables, platform->globalVariables, 128 * 4);

  ac_int<32, false> localGlobalVariableCounter = globalVariableCounter;

  unsigned int result = irGenerator_hw(localVliwBinaries, addressInBinaries, blockSize, localBytecode,
                                       localGlobalVariables, localGlobalVariableCounter);

  unsigned int result_sw = irGenerator_sw(platform->vliwBinaries, addressInBinaries, blockSize, platform->bytecode,
                                          platform->globalVariables, globalVariableCounter);

  int size = result & 0xffff;
  //	for (int oneIRInstr = 0; oneIRInstr<size+5; oneIRInstr++){
  //		std::cerr << printBytecodeInstruction(oneIRInstr, platform->bytecode[oneIRInstr*4+0],
  //platform->bytecode[oneIRInstr*4+1], platform->bytecode[oneIRInstr*4+2], platform->bytecode[oneIRInstr*4+3]);
  //		std::cerr << printBytecodeInstruction(oneIRInstr, localBytecode[oneIRInstr].slc<32>(96),
  //localBytecode[oneIRInstr].slc<32>(64),  localBytecode[oneIRInstr].slc<32>(32),
  //localBytecode[oneIRInstr].slc<32>(0)); 		std::cerr << "\n";
  //	}

  if (!acintCmp(platform->bytecode, localBytecode, 256 * 16)) {
    fprintf(stderr, "After performing ir generation in HW and in SW, bytecode is different\n");
    exit(-1);
  }

  if (!acintCmp(platform->vliwBinaries, localVliwBinaries, MEMORY_SIZE * 16)) {
    fprintf(stderr, "After performing ir generation in HW and in SW, vliw binaries are different\n");
    exit(-1);
  }

  if (!acintCmp(platform->globalVariables, localGlobalVariables, 128 * 4)) {
    fprintf(stderr, "After performing ir generation in HW and in SW, global variables are different\n");
    exit(-1);
  }

  if (result_sw != result) {
    fprintf(stderr, "result is different: %x and %x\n", result, result_sw);
    exit(-1);
  }

  free(localBytecode);
  free(localGlobalVariables);
  free(localVliwBinaries);

  return result;

#endif

#ifdef __SW

  /********************************************************************************************
   * Second version of sources for __SW
   *
   * The pass is done in SW only and the result is returned
   *
   ********************************************************************************************/
  startProfiler(1);
  int result = irGenerator_sw(platform->vliwBinaries, addressInBinaries, blockSize, platform->bytecode,
                              platform->globalVariables, globalVariableCounter);
  stopProfiler(1);

  return result;
#endif

#ifdef __HW_SIM

  /********************************************************************************************
   * Second version of sources for __SW
   *
   * The pass is done in HW simulation only: all values coming from normal memories (unsigned int
   * and not ac_int types) are copied into newly allocated arrays using the correct type.
   * Then the pass is called in and the result is copied back to the normal memories.
   *
   ********************************************************************************************/

  ac_int<128, false>* localBytecode      = (ac_int<128, false>*)malloc(256 * sizeof(ac_int<128, false>));
  ac_int<128, false>* localVliwBinaries  = (ac_int<128, false>*)malloc(MEMORY_SIZE * sizeof(ac_int<128, false>));
  ac_int<32, true>* localGlobalVariables = (ac_int<32, true>*)malloc(128 * sizeof(ac_int<32, true>));

  acintMemcpy(localBytecode, platform->bytecode, 256 * 16);
  acintMemcpy(localVliwBinaries, platform->vliwBinaries, MEMORY_SIZE * 16);
  acintMemcpy(localGlobalVariables, platform->globalVariables, 128 * 4);

  ac_int<32, false> localGlobalVariableCounter = globalVariableCounter;

  unsigned int result = irGenerator_hw(localVliwBinaries, addressInBinaries, blockSize, localBytecode,
                                       localGlobalVariables, localGlobalVariableCounter);

  int size = result & 0xffff;

  acintMemcpy(platform->bytecode, localBytecode, size * 16);
  acintMemcpy(platform->vliwBinaries, localVliwBinaries, MEMORY_SIZE * 16);
  acintMemcpy(platform->globalVariables, localGlobalVariables, 128 * 4);

  free(localBytecode);
  free(localGlobalVariables);
  free(localVliwBinaries);

  return result;

#endif
}
