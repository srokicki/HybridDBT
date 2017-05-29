#include <types.h>

const int STAGE_NUMBER_L2 = 2;
const int STAGE_NUMBER = 4;
const int WINDOW_SIZE_L2  = 2;
const int WINDOW_SIZE  = 4;

// IR instruction buffer
ac_int<50, false> instructions[256];

// last register read/write access
ac_int<32, false> lastRead[64]  = {0};
ac_int<32, false> lastWrite[64] = {0};

// Stages windows
ac_int<32, true> stageWindow[STAGE_NUMBER][WINDOW_SIZE];

// Stages types
ac_int<3, false> stageType[STAGE_NUMBER] = { 0, 3, 1, 2 };

ac_int<2, false> getType(ac_int<50, false> instruction) {
  return instruction.slc<2>(30+18);
}

ac_int<32, false> scheduling(
  ac_int<1, false> optLevel,
  ac_int<8, false> basicBlockSize,
  ac_int<128, false> bytecode[256],
  ac_int<128, false> binaries[1024],
  ac_int<16, false> addressInBinaries,
  ac_int<6, false> placeOfRegisters[512],
  ac_int<6, false> numberFreeRegister,
  ac_int<6, false> freeRegisters[64],
  ac_int<4, false> issue_width,
  ac_int<MAX_ISSUE_WIDTH * 2, false> way_specialisation,
  ac_int<32, false> placeOfInstr[256]
){

  ac_int<8, false> instructionId = 0;
  while (instructionId < basicBlockSize) {
    ac_int<128, false> bytecode_word = bytecode[instructionId];
    ac_int<32, false> bytecode_word1 = bytecode_word.slc<32>(96);
    ac_int<32, false> bytecode_word2 = bytecode_word.slc<32>(64);    			
    ac_int<32, false> bytecode_word3 = bytecode_word.slc<32>(32);
    ac_int<32, false> bytecode_word4 = bytecode_word.slc<32>(0);

    /*
     * REMINDER: This is how you decompose the IR into an instruction and
     * its dependencies
     */

    ac_int<50, false> instruction = 0;
    instruction.set_slc(18, bytecode_word1);
    instruction.set_slc(0, bytecode_word2.slc<18>(14));

    // Type of Functional Unit needed by this instruction
    ac_int<2, false> unitType = getType(instruction);

    // We split different information from the instruction
    ac_int<2, false> typeCode = instruction.slc<2>(46);
    ac_int<1, false> alloc = instruction[45];
    ac_int<1, false> allocBr = instruction[44];
    ac_int<7, false> opCode = instruction.slc<7>(37);
    ac_int<1, false> isImm = instruction[36];
    ac_int<1, false> isBr = instruction[35];
    ac_int<9, false> virtualRDest = instruction.slc<9>(0);
    ac_int<9, false> virtualRIn2 = instruction.slc<9>(9);
    ac_int<9, false> virtualRIn1_imm9 = instruction.slc<9>(18);
    ac_int<13, false> imm13 = instruction.slc<13>(18); //TODO
    ac_int<19, false> imm19 = instruction.slc<19>(9);
    ac_int<9, false> brCode = instruction.slc<9>(27);

    // Find the earliest place which avoids RAW, WAR, or WAW conflicts
    ac_int<1, false> stageToPlace = false;
    ac_int<32, false> place = lastRead[virtualRDest];
    
    // Depending on the dependency and the pipeline stages,
    // some offset must be added:
    //
    // Pipeline stages : DC RD EX WR
    //                      DC RD EX WR
    //                         DC RD EX WR
    //                            DC RD EX WR
    //

    // WAR
    ac_int<32, false> place = lastRead[virtualRDest];

    // RAW
    place = max(place, lastWrite[virtualRIn2]);
    if (typeCode == 0 && !isImm)
      place = max(place, lastWrite[virtualRIn1_imm9]);

    // WAW
    place = max(place, lastWrite[virtualRDest]-2);

    // Find the earliest place in the current window
    for (ac_int<STAGE_NUMBER_L2, false> stageId = 0; stageId < STAGE_NUMBER; ++stageId) {

      if (unitType & stageType[stageId]) {
        for (ac_int<WINDOW_SIZE_L2, false> windowOffset = 0
            ; windowOffset < WINDOW_SIZE; ++windowOffset) {

          if (stageWindow[stageId][windowOffset] == -1) {
            if (windowPosition >= place - windowOffset) {
              place = windowPosition + windowOffset;
              stageWindow[stageId][windowOffset] = instructionId;
              found = true;
            }
          }
        } // for windowOffset
      }
    } // for stageId

    // Whenever the instruction doesn't fit:
    // - write all instructions in vliw binaries
    // - shift the window
    if (!found) {
      for (ac_int<WINDOW_SIZE_L2, false> windowOffset = 0
          ; windowOffset < WINDOW_SIZE; ++windowOffset) {

          ac_int<256, false> binariesWord;

          for (ac_int<STAGE_NUMBER_L2, false> stageId = 0
              ; stageId < STAGE_NUMBER; ++stageId) {
            //***************************************
            //We generate the instruction
            ac_int<32, false> generatedInstruction;
            generatedInstruction.set_slc(0, opCode);
            generatedInstruction.set_slc(26, virtualRIn2);

            if (typeCode == 0) { //The instruction is R type

              if (isImm) {
                generatedInstruction.set_slc(7, imm13);
                generatedInstruction.set_slc(20, dest);
              }
              else{
                generatedInstruction.set_slc(14, dest);
                generatedInstruction.set_slc(20, virtualRIn1_imm9);
              }
            }
  //					else if (typeCode == 1){ //The instruction is Rext Type
  //						generatedInstruction[7] = isImm;
  //
  //						generatedInstruction.set_slc(8, placeOfRegisters[brCode].slc<3>(0));
  //
  //						if (isImm){
  //							generatedInstruction.set_slc(11, virtualRIn1_imm9);
  //							generatedInstruction.set_slc(20, dest);
  //						}
  //						else{
  //							generatedInstruction.set_slc(14, dest);
  //							generatedInstruction.set_slc(20, placeOfRegisters[virtualRIn1_imm9]);
  //						}
  //					}
            else { //The instruction is I Type
              if (opCode == 0x28)
                generatedInstruction.set_slc(26, dest);
              else{
                generatedInstruction.set_slc(26, virtualRDest);
              }
              generatedInstruction.set_slc(7, imm19);
            }
            binariesWord.set_slc(stageId*32, generatedInstruction);
          } // for stageId

          //after all stages has been filled, we commit the word
          fprintf(stderr, "test %d\n", writeInBinaries);
          binaries[(windowPosition + windowOffset)*2]     = binariesWord.slc<128>(0);
          binaries[(windowPosition + windowOffset)*2 + 1] = binariesWord.slc<128>(128);
          windowOffset += WINDOW_SIZE;
        } // for windowOffset
    } else {
      lastWrite[virtualRDest]     = place + RAW[unitType];
      lastRead [virtualRIn2]      = place + WAR[unitType];
      lastRead [virtualRIn1_imm9] = place + WAR[unitType];
      ++instructionId;
    }
  }

  return windowPosition+windowSize+addressInBinaries;
}
