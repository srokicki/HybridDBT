/*
 * irScheduling_scoreboard_sw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */


/*
 * irScheduling_scoreboard_hw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */


#include <types.h>
#include <isa/irISA.h>
#include <isa/vexISA.h>
#include <lib/endianness.h>

template<int N> struct l2 { enum { value = 1 + l2<N/2>::value }; };
template<> struct l2<1> { enum { value = 1 }; };

// window constants
const int STAGE_NUMBER = 8;
const int STAGE_NUMBER_L2 = l2<STAGE_NUMBER>::value;
const int WINDOW_SIZE  = 16;
const int WINDOW_SIZE_L2  = l2<WINDOW_SIZE>::value;

// useful constants
const short cst1ff = 0x1ff;
const unsigned int zero32 = 0;

// for replacing the jump instruction at a basic block end
bool haveJump_sw;
unsigned int jumpPlace_sw;

// computed instructino ID
unsigned short instructionId_sw;

// window position in the VEX binaries
unsigned short windowPosition_sw;

// used to virtually shift the window (instead of copying the values backward)
unsigned char windowShift_sw;

// for knowing when to enable a register for allocation
unsigned char registerDependencies_sw[256];

// for WAR dependencies
unsigned int lastRead_sw[128];

// for RAW dependencies
unsigned int lastWrite_sw[128];

// Scheduled instructions stages
unsigned char instructionsStages_sw[256];

// explicit forwarding for catapult (lowering II)
unsigned char lastInstructionStage_sw;
unsigned int lastPlaceOfInstr_sw;

unsigned int window_sw[WINDOW_SIZE][STAGE_NUMBER];
bool freeSlot_sw[WINDOW_SIZE][8];

unsigned int max(unsigned int a, unsigned int b) {
	return a > b ? a : b;
}

/**
 * @brief computes the real window offset for circular buffer representation
 * @param the virtual offset
 * @return the real offset
 */
int offset(int off) {
	return (off + windowShift_sw) % WINDOW_SIZE;
}

/**
 * @brief Creates a VEX instruction from a bytecode instruction with real
 *				registers (virtual registers represent real ones)
 * @param the bytecode instruction
 * @return the VEX instruction
 */
unsigned int createInstruction(unsigned int irInstr96, unsigned int irInstr64, unsigned short rIn1, unsigned short rIn2, unsigned short rDest) {
	// Type of Functional Unit needed by this instruction
	char unitType = irInstr96>>30;

	// We split different information from the instruction
	char typeCode = ((irInstr96 >> 28) & 0x3);
	unsigned char alloc = ((irInstr96 >> 27) & 0x1);
	unsigned char allocBr = ((irInstr96 >> 26) & 0x1);
	unsigned char opCode = ((irInstr96 >> 19) & 0x7f);

	unsigned char  isImm = ((irInstr96 >> 18) & 0x1);
	unsigned char funct = ((irInstr96 >> 13) & 0x1f);

	unsigned short virtualRDest = (irInstr64 >> 14) & 0x1ff;
	unsigned short virtualRIn2 = (irInstr64 >> 23) & 0x1ff;
	unsigned short virtualRIn1_imm9 = (irInstr96 >> 0) & 0x1ff;
	unsigned short imm13 = (irInstr96 >> 0) & 0x1fff;
	unsigned int imm19 = (((irInstr96 >> 0) & 0x3ff)<<9) + ((irInstr64 >> 23) & 0x1ff);

	//***************************************
	//We generate the instruction

	unsigned int generatedInstruction = 0;
	generatedInstruction |= opCode;

	if (typeCode == 0) { //The instruction is R type
		generatedInstruction |= ((rIn2 & 0x3f)<<26);

		if (opCode == VEX_FP){
			generatedInstruction |= ((funct & 0x1f)<<7);
		}

		if (isImm) {
			generatedInstruction |= ((imm13 & 0x1fff)<<7);
			generatedInstruction |= ((rDest & 0x3f)<<20);
		}
		else{
			generatedInstruction |= ((rDest & 0x3f)<<14);
			generatedInstruction |= ((rIn1 & 0x3f)<<20);
		}
	}
	else { //The instruction is I Type
		generatedInstruction |= ((rDest & 0x3f)<<26);

		generatedInstruction |= ((imm19 & 0x7ffff)<<7);
	}

	return generatedInstruction;
}


int priority_sw[MAX_ISSUE_WIDTH] = {7,6,0,3,1,4,5,2};
void sort_ways(unsigned int ways)
{
  static int sorted = 0;
  if (!sorted)
  {
    int ns[16] = { 0 };
    int classeur[16][MAX_ISSUE_WIDTH];
    sorted = 1;

    for (int i = 0; i < MAX_ISSUE_WIDTH; ++i)
    {
      int id = (ways>>(i*4)) & 0xf;
      classeur[id][ns[id]++] = i;
    }

    int * it = priority_sw;
    for (int i = 0; i < 16; ++i)
    {
      for (int j = 0; j < ns[i]; ++j)
      {
        *(it++) = classeur[i][j];
      }
    }
  }
}

#pragma hls_design top
unsigned int irScheduler_scoreboard_sw(
		bool optLevel,
		unsigned char basicBlockSize,
		unsigned int bytecode[256*4],
		unsigned int binaries[1024*4],
		unsigned int addressInBinaries,
		unsigned char placeOfRegisters[512],
		unsigned char numberFreeRegister,
		unsigned char freeRegisters[64],
		unsigned char issue_width, // TODO: change to 1 boolean per issue
		unsigned int way_specialisation,
		unsigned int placeOfInstr[256]){

	//**************************************************************
	// Setup scheduler state
	//**************************************************************

//  sort_ways(way_specialisation);


	haveJump_sw = 0;
	instructionId_sw = 0;
	windowPosition_sw = 0;//addressInBinaries;
	windowShift_sw = 0;

	char incrementInBinaries = (issue_width>4) ? 2 : 1;

	for (int windowOffset = 0; windowOffset < WINDOW_SIZE; ++windowOffset)
		for (int oneStage = 0; oneStage<MAX_ISSUE_WIDTH; oneStage++)
			freeSlot_sw[windowOffset][oneStage] = true;


	for (int i = 0; i < 128; ++i) {
		lastRead_sw[i] = 0;
		lastWrite_sw[i] = 0;
	}

	while (instructionId_sw < basicBlockSize) {


		//**************************************************************
		// Fetching / Decoding instruction
		//**************************************************************

		unsigned int irInstr96 = readInt(bytecode, instructionId_sw*16 + 0);
		unsigned int irInstr64 = readInt(bytecode, instructionId_sw*16 + 4);
		unsigned int irInstr32 = readInt(bytecode, instructionId_sw*16 + 8);
		unsigned int irInstr0 = readInt(bytecode, instructionId_sw*16 + 12);

		// Type of Functional Unit needed by this instruction
		char unitType = irInstr96>>30;

		// We split different information from the instruction
		char typeCode = ((irInstr96 >> 28) & 0x3);
		unsigned char alloc = ((irInstr96 >> 27) & 0x1);
		unsigned char allocBr = ((irInstr96 >> 26) & 0x1);
		unsigned char opCode = ((irInstr96 >> 19) & 0x7f);

		unsigned char  isImm = ((irInstr96 >> 18) & 0x1);
		unsigned char funct = ((irInstr96 >> 13) & 0x1f);

		unsigned short virtualRDest = (irInstr64 >> 14) & 0x1ff;
		unsigned short virtualRIn2 = (irInstr64 >> 23) & 0x1ff;
		unsigned short virtualRIn1_imm9 = (irInstr96 >> 0) & 0x1ff;
		unsigned short imm13 = (irInstr96 >> 0) & 0x1fff;
		unsigned int imm19 = ((irInstr96 >> 0) & 0x3ff) + ((irInstr64 >> 23) & 0x1ff);

		// real dest register (after alloc/dereferencing)
		unsigned char dest;

		// for RAW/WAW dependencies
		unsigned char nbDataDeps    = (irInstr64 >> 3) & 0x7;
		unsigned char nbNonDataDeps = (irInstr64 & 0x7) - nbDataDeps;
		unsigned char deps[8];

		deps[0] = irInstr32 >>16;
		deps[1] = irInstr32>>8;
		deps[2] = irInstr32>>0;
		deps[3] = irInstr0>>24;
		deps[4] = irInstr0>>16;
		deps[5] = irInstr0>>8;
		deps[6] = irInstr0>>0;


		//For alloc
		unsigned char accessPlaceOfReg = placeOfRegisters[virtualRDest];
		unsigned char accessFreeReg = freeRegisters[numberFreeRegister-1];

		//**************************************************************
		// Allocation
		//**************************************************************

		// if allocation requested (for dest)
		if (alloc) {
			// take the first free register + update register dependencies
			if (numberFreeRegister > 0) {
				numberFreeRegister--;
				dest = accessFreeReg;
				registerDependencies_sw[instructionId_sw] = ((irInstr64 >> 6) & 0xff);
			} else {
				// else crash
				exit(-1);
				//return basicBlockSize+1;
			}
		} else {
			registerDependencies_sw[instructionId_sw] = 0xff;
			dest = accessPlaceOfReg;
		}

		//**************************************************************
		// Computing dependencies
		//**************************************************************

		unsigned int earliest_place = 0;
		// here, ternary conditions are for forwarding purpose

		for (int i = 0; i < 7; ++i) {
			int place = placeOfInstr[deps[i]];

			// RAW (gap depends on the type of stage because of pipeline length diffs)
			if (i < nbDataDeps) {
				char stg = instructionsStages_sw[deps[i]];

				char spec = (way_specialisation>>(stg*4)) & 0xf;
				char gap = ((spec & 0x2) || (spec & 0x8)) ? 2 : 1;

				earliest_place = max(earliest_place, place+gap);

			}
			// WAW
			else if (6-i < nbNonDataDeps) {
				if (unitType == 0){

					//Special case for handling jump instructions a bit better
					char typeOfPred = readInt(bytecode, 16*deps[i]) >> 30;

					char stg = instructionsStages_sw[deps[i]];
					char spec = (way_specialisation>>(stg*4)) & 0xf;
					char gap = ((spec & 0x2) || (spec & 0x8)) ? 0 : -1;
					unsigned int test = place+gap;
					test = !((spec & 0x2) || (spec & 0x8)) && place == 0 ? windowPosition_sw : test;
					test = (typeOfPred == 0) ? place+2 : test+0;
					earliest_place = max(earliest_place, test);

				}
				else{
					//Normal case for control dep
					char typeOfPred = readInt(bytecode, 16*deps[i]) >> 30;
					unsigned int test = (typeOfPred == 0) ? place+2 : place+1;
					earliest_place = max(earliest_place, test);

//					ac_int<2, false> typeOfPred = bytecode[deps[i]].slc<2>(126);
//					ac_int<32, true> test = (typeOfPred == 0) ? (ac_int<32,false>(place+2)) : (ac_int<32,false>(place+1));
//					earliest_place = max(earliest_place, test);

				}
			}
		}

		// WAR
		earliest_place = max(earliest_place, lastRead_sw[dest]);

		unsigned char shiftedOpcode = opCode>>4;

		char isNop = (opCode == 0);
		char isArith2 = (shiftedOpcode == 4 || shiftedOpcode == 5 || shiftedOpcode == 0);
		char isLoad = (opCode>>3) == 0x2;
		char isStore = (opCode>>3) == 0x3;
		char isArith1 = (shiftedOpcode == 6 || shiftedOpcode == 7);
		char isBranchWithReg = (opCode == VEX_CALLR) ||(opCode == VEX_GOTOR);
		char isFPOneReg = (opCode == VEX_FP && (funct == VEX_FP_FCVTSW || funct == VEX_FP_FCVTSWU || funct == VEX_FP_FCVTWS
				|| funct == VEX_FP_FCVTWUS || funct == VEX_FP_FMVWX || funct == VEX_FP_FMVXW || funct == VEX_FP_FCLASS));
		char isFPTwoReg = opCode == VEX_FP && !isFPOneReg;
		char isBranchWithTwoRegs = (opCode == VEX_BR) || (opCode == VEX_BRF) || (opCode == VEX_BGE) || (opCode == VEX_BLT) || (opCode == VEX_BGEU) || (opCode == VEX_BLTU);

		if (isArith2 || isFPTwoReg){
			earliest_place = max(earliest_place, lastWrite_sw[virtualRIn1_imm9]);
			earliest_place = max(earliest_place, lastWrite_sw[virtualRIn2]);
		}
		else if (isStore || isBranchWithTwoRegs){
			earliest_place = max(earliest_place, lastWrite_sw[virtualRIn2]);
			earliest_place = max(earliest_place, lastWrite_sw[virtualRDest]);
		}
		else if (isArith1 || isLoad || isFPOneReg)
			earliest_place = max(earliest_place, lastWrite_sw[virtualRIn2]);
		else if (isBranchWithReg)
			earliest_place = max(earliest_place, lastWrite_sw[virtualRDest]);


		//**************************************************************
		// Placing the instruction
		//**************************************************************

		int bestWindowOffset = WINDOW_SIZE;
		int bestStageId;

		int offsetStart = earliest_place < windowPosition_sw ? 0 : earliest_place - windowPosition_sw;

		// available places search
		bool found = false;

		for (int windowOffset = offsetStart; windowOffset < WINDOW_SIZE; ++windowOffset){
			for (int stageId_ = 0; stageId_ < STAGE_NUMBER; stageId_ += 1){
				int stageId = priority_sw[stageId_];
				char stageType = (way_specialisation>>(stageId*4)) & 0xf;
				//If type is compatible
				if (stageType && ((stageType >> unitType) & 0x1) && freeSlot_sw[offset(windowOffset)][stageId]) {
					bestStageId = stageId;
					bestWindowOffset = windowOffset;
					found = true;
					break;

				}

			}
			if (found)
				break;

		}


		bool possible = found;

		//**************************************************************
		// Generation + pre-placement of instruction
		//**************************************************************

		placeOfRegisters[instructionId_sw] = dest;

		short rin1 = virtualRIn1_imm9;
		short rin2 = typeCode == 2 ? virtualRDest : virtualRIn2;

		short modifiedRin2 = rin2, modifiedRin1 = rin1, modifiedRdest = dest;

		char placeOfRin1 = placeOfRegisters[rin1];

		char placeOfRin2 = placeOfRegisters[rin2];

		unsigned char rin1Dep = registerDependencies_sw[rin1 & 0xff];
		unsigned char rin2Dep = registerDependencies_sw[rin2 & 0xff];

		bool useRin1 = (typeCode == 0 && !isImm) && (opCode != VEX_FP || (opCode == VEX_FP && funct != VEX_FP_FCVTSW && funct != VEX_FP_FCVTSWU && funct != VEX_FP_FCVTWS && funct != VEX_FP_FCVTWUS && funct != VEX_FP_FMVWX && funct != VEX_FP_FMVXW && funct != VEX_FP_FCLASS));
		bool useRin2 = typeCode == 0 || (typeCode == 2 && opCode != VEX_MOVI && opCode != VEX_GOTO && opCode != VEX_CALL);
		if (useRin2) {
			if (typeCode != 2)
				modifiedRin2 = placeOfRin2;
			else
				modifiedRdest = placeOfRin2;
		}

		if (useRin1) {
			modifiedRin1 = placeOfRin1;
			if (rin1<256 && rin1 == rin2) {
				 rin1Dep -= 2;
			} else {
				if (rin1<256)
					rin1Dep--;

				if (rin2<256)
					rin2Dep--;
			}
		} else if (rin2<256) {
			rin2Dep--;
		}

		registerDependencies_sw[rin1 & 0xff] = rin1Dep;
		registerDependencies_sw[rin2 & 0xff] = rin2Dep;

		if (useRin1 && rin1<256 && rin1Dep == 0)
			freeRegisters[numberFreeRegister++] = placeOfRin1;
		if (useRin2 && rin2<256 && rin2Dep == 0)
			freeRegisters[numberFreeRegister++] = placeOfRin2;

		//***********************************************************************
		// !Place found : Write binaries + shift the window + correct placement
		//***********************************************************************
		if (!possible) {
			unsigned int advance = (earliest_place > windowPosition_sw+WINDOW_SIZE)	? earliest_place-windowPosition_sw-WINDOW_SIZE+1 : 1;

			for (int windowOffset = 0; windowOffset < 3; ++windowOffset) {
				char off = offset(windowOffset);

				binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+3] = freeSlot_sw[off][0] ? 0 : window_sw[off][0];
				binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+2] = freeSlot_sw[off][1] ? 0 : window_sw[off][1];
				binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+1] = freeSlot_sw[off][2] ? 0 : window_sw[off][2];
				binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+0] = freeSlot_sw[off][3] ? 0 : window_sw[off][3];
				if (issue_width>4) {
					binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+7] = freeSlot_sw[off][4] ? 0 : window_sw[off][4];
					binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+6] = freeSlot_sw[off][5] ? 0 : window_sw[off][5];
					binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+5] = freeSlot_sw[off][6] ? 0 : window_sw[off][6];
					binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+4] = freeSlot_sw[off][7] ? 0 : window_sw[off][7];
				}

				if (windowOffset < advance)
					for (int oneStage = 0; oneStage<MAX_ISSUE_WIDTH; oneStage++)
						freeSlot_sw[off][oneStage] = true;
			}
			windowPosition_sw += advance;
			windowShift_sw = (windowShift_sw+(advance))%WINDOW_SIZE;

			for (int stageId_ = 0; stageId_ < STAGE_NUMBER; ++stageId_){
				int stageId = priority_sw[stageId_];
				char stageType = (way_specialisation>>(stageId*4)) & 0xf;
				if (stageType && ((stageType >> unitType) & 0x1)) {
					bestStageId = stageId;
					break;
				}
			}

			bestWindowOffset = WINDOW_SIZE-1;
		}
		//****************************************************************
		// Writing instruction into the window_sw buffer
		//****************************************************************

		lastPlaceOfInstr_sw = windowPosition_sw + bestWindowOffset;
		placeOfInstr[instructionId_sw] = windowPosition_sw + bestWindowOffset;

		instructionsStages_sw[instructionId_sw] = bestStageId;

		if (useRin2)
			lastRead_sw[placeOfRin2] = lastPlaceOfInstr_sw;
		if (useRin1)
			lastRead_sw[placeOfRin1] = lastPlaceOfInstr_sw;

		//We update last write
		if (isArith2 || isFPTwoReg || isArith1 || isLoad || isFPOneReg){
			lastWrite_sw[dest] = lastPlaceOfInstr_sw;
		}

		window_sw[offset(bestWindowOffset)][bestStageId] = createInstruction(irInstr96, irInstr64, modifiedRin1, modifiedRin2, modifiedRdest);
		freeSlot_sw[offset(bestWindowOffset)][bestStageId] = false;

		if (unitType == 0) {
			haveJump_sw = 1;
			jumpPlace_sw = placeOfInstr[instructionId_sw];
		}

		instructionId_sw++;


	}

	//**************************************************************
	// Write remaining binaries + find the last word which contains instructions
	//**************************************************************

	unsigned int lastAddress = 0;
	unsigned int lastGap = 0;
	for (int windowOffset = 0; windowOffset < WINDOW_SIZE; ++windowOffset) {
		int off = offset(windowOffset);

		binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+3] = freeSlot_sw[off][0] ? 0 : window_sw[off][0];
		binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+2] = freeSlot_sw[off][1] ? 0 : window_sw[off][1];
		binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+1] = freeSlot_sw[off][2] ? 0 : window_sw[off][2];
		binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+0] = freeSlot_sw[off][3] ? 0 : window_sw[off][3];
		if (issue_width>4) {
			binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+7] = freeSlot_sw[off][4] ? 0 : window_sw[off][4];
			binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+6] = freeSlot_sw[off][5] ? 0 : window_sw[off][5];
			binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+5] = freeSlot_sw[off][6] ? 0 : window_sw[off][6];
			binaries[(addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4+4] = freeSlot_sw[off][7] ? 0 : window_sw[off][7];
		}

		for (int oneStage = 0; oneStage<MAX_ISSUE_WIDTH; oneStage++){
			if (!freeSlot_sw[off][oneStage]){
				lastGap = windowOffset;
				lastAddress = (addressInBinaries+incrementInBinaries*(windowPosition_sw+windowOffset))*4;
			}
			freeSlot_sw[off][oneStage] = true;
		}
	}


	unsigned int newSize = (windowPosition_sw+lastGap + 1);
	for (int stageId = 0; stageId<issue_width; stageId++){

		char stageOffset = 4*(stageId/4) + (3-(stageId%4));
		unsigned int instr = binaries[lastAddress + stageOffset];

		unsigned char opcode = instr & 0x7f;
        char spec = (way_specialisation>>(stageId*4)) & 0xf;

		if (opcode != 0 && (((spec & 0x2) && (opcode > 0x1f || opcode < VEX_STB))
				|| (spec & 0x8)
				|| ((spec & 0x1) && (opcode == VEX_BR || opcode == VEX_BRF || opcode == VEX_BGE || opcode == VEX_BLT || opcode == VEX_BGEU || opcode == VEX_BLTU || opcode == VEX_CALL || opcode == VEX_CALLR || opcode == VEX_GOTO || opcode == VEX_GOTOR )))){

			newSize++;

			binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+3] = 0;
			binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+2] = 0;
			binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+1] = 0;
			binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+0] = 0;

			if (issue_width>4) {
				binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+7] = 0;
				binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+6] = 0;
				binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+5] = 0;
				binaries[(addressInBinaries+incrementInBinaries*(newSize-1))*4+4] = 0;
			}
			break;
		}
	}

	newSize = newSize * incrementInBinaries;

	unsigned int newEnd = addressInBinaries+newSize;


	return newSize;
}



