/*
 * irScheduling_scoreboard_hw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#ifndef __HW
#ifndef __SW

#include <types.h>
#include <isa/irISA.h>
#include <isa/vexISA.h>
#include <transformation/irScheduler.h>

template<int N> struct l2 { enum { value = 1 + l2<N/2>::value }; };
template<> struct l2<1> { enum { value = 1 }; };

// window constants
const int STAGE_NUMBER = 8;
const int STAGE_NUMBER_L2 = l2<STAGE_NUMBER>::value;
const int WINDOW_SIZE  = 16;
const int WINDOW_SIZE_L2  = l2<WINDOW_SIZE>::value;

// useful constants
const ac_int<9, false> cst1ff = 0x1ff;
const ac_int<32, false> zero32 = 0;

// for replacing the jump instruction at a basic block end
ac_int<1, false> haveJump;
ac_int<32, false> jumpPlace;

// computed instructino ID
ac_int<9, false> instructionId;

// window position in the VEX binaries
ac_int<16, false> windowPosition;

// used to virtually shift the window (instead of copying the values backward)
ac_int<4, false> windowShift;

// for knowing when to enable a register for allocation
ac_int<8, false> registerDependencies[256];

// for WAR dependencies
ac_int<32, true> lastRead[128];

// for RAW dependencies
ac_int<32, true> lastWrite[128];

// Scheduled instructions stages
ac_int<8, false> instructionsStages[256];

// explicit forwarding for catapult (lowering II)
ac_int<8, false> lastInstructionStage;
ac_int<32, false> lastPlaceOfInstr;

ac_int<32, false> window[WINDOW_SIZE][STAGE_NUMBER];
ac_int<8, false> freeSlot[WINDOW_SIZE];

ac_int<32, false> max(ac_int<32, false> a, ac_int<32, false> b) {
	return a > b ? a : b;
}

/**
 * @brief computes the instruction type in order to know which stage
 *				it can go on
 * @param the instruction
 * @return its type
 */
ac_int<2, false> getType(ac_int<50, false> instruction) {
	return instruction.slc<2>(30+18);
}

/**
 * @brief computes the real window offset for circular buffer representation
 * @param the virtual offset
 * @return the real offset
 */
ac_int<WINDOW_SIZE_L2+1, false> offset(ac_int<WINDOW_SIZE_L2+1, false> off) {
	return (off + windowShift) % WINDOW_SIZE;
}

/**
 * @brief Creates a VEX instruction from a bytecode instruction with real
 *				registers (virtual registers represent real ones)
 * @param the bytecode instruction
 * @return the VEX instruction
 */
ac_int<32, false> createInstruction(ac_int<50, false> instruction, ac_int<6, false> operand1, ac_int<6, false> operand2, ac_int<6, false> dest) {

	// Type of Functional Unit needed by this instruction
	ac_int<2, false> unitType = getType(instruction);

	// We split different information from the instruction
	ac_int<2, false> typeCode = instruction.slc<2>(46);
	ac_int<1, false> alloc = instruction[45];
	ac_int<1, false> allocBr = instruction[44];
	ac_int<7, false> opCode = instruction.slc<7>(37);

	ac_int<1, false> isImm = instruction[36];
	ac_int<7, false> funct = instruction.slc<5>(31);

	ac_int<1, false> isBr = instruction[35];
	ac_int<9, false> virtualRDest = instruction.slc<9>(0);
	ac_int<9, false> virtualRIn2 = instruction.slc<9>(9);
	ac_int<9, false> virtualRIn1_imm9 = instruction.slc<9>(18);
	ac_int<13, false> imm13 = instruction.slc<13>(18); //TODO
	ac_int<19, false> imm19 = instruction.slc<19>(9);
	ac_int<9, false> brCode = instruction.slc<9>(27);

	//***************************************
	//We generate the instruction
	ac_int<32, false> generatedInstruction = 0;
	generatedInstruction.set_slc(0, opCode);

	if (opCode == VEX_CGRA)
	{
		generatedInstruction.set_slc(7, imm19);
	}
	else
	{
		generatedInstruction.set_slc(26, ac_int<6>(operand2));
		if (typeCode == 0) { //The instruction is R type


			if (opCode == VEX_FP){
				generatedInstruction.set_slc(7, funct);

			}


			if (isImm) {
				generatedInstruction.set_slc(7, imm13);

				if ((opCode >> 3) == (VEX_STW>>3) || opCode == VEX_FSW){
					generatedInstruction.set_slc(20, operand1);
				}
				else {
					generatedInstruction.set_slc(20, dest);
				}
			}
			else{
				generatedInstruction.set_slc(14, dest);
				generatedInstruction.set_slc(20, operand1);
			}
		}
		else { //The instruction is I Type
			if (opCode == VEX_MOVI || opCode == VEX_CALL) {
				generatedInstruction.set_slc(7, imm19);
				generatedInstruction.set_slc(26, dest);
			}
			else if (opCode == VEX_BR || opCode == VEX_BRF || opCode == VEX_BGE || opCode == VEX_BLT || opCode == VEX_BGEU || opCode == VEX_BLTU){
				generatedInstruction.set_slc(26, operand1);
				generatedInstruction.set_slc(20, operand2);

			}
			else{
				generatedInstruction.set_slc(7, imm19);
				generatedInstruction.set_slc(26, operand1);
			}
		}
	}

	return generatedInstruction;
}

int priority[MAX_ISSUE_WIDTH] = {7,6,0,3,1,4,5,2};
void sort_ways(ac_int<MAX_ISSUE_WIDTH * 4, false> ways)
{
  static int sorted = 0;
  if (!sorted)
  {
    int ns[16] = { 0 };
    int classeur[16][MAX_ISSUE_WIDTH];
    sorted = 1;

    for (int i = 0; i < MAX_ISSUE_WIDTH; ++i)
    {
      int id = ways.slc<4>(i * 4);
      classeur[id][ns[id]++] = i;
    }

    int * it = priority;
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
ac_int<32, false> irScheduler_scoreboard_hw(
		ac_int<1, false> optLevel,
		ac_int<8, false> basicBlockSize,
		ac_int<128, false> bytecode[256],
		ac_int<128, false> binaries[1024],
		ac_int<32, false> addressInBinaries,
		ac_int<6, false> placeOfRegisters[512],
		ac_int<6, false> numberFreeRegister,
		ac_int<6, false> freeRegisters[64],
		ac_int<8, false> issue_width, // TODO: change to 1 boolean per issue
		ac_int<MAX_ISSUE_WIDTH * 4, false> way_specialisation,
		ac_int<32, false> placeOfInstr[256]){

	//**************************************************************
	// Setup scheduler state
	//**************************************************************

//  sort_ways(way_specialisation);

	#ifndef __CATAPULT
	//Performance simulation
	timeTakenIRScheduler = 0;
	#endif

	haveJump = 0;
	instructionId = 0;
	windowPosition = 0;//addressInBinaries;
	windowShift = 0;

	for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
	; windowOffset < WINDOW_SIZE; ++windowOffset) {
			freeSlot[windowOffset] = 0xFF;
	}

	for (ac_int<l2<64>::value+1, false> i = 0; i < 128; ++i) {
		lastRead[i] = 0;
		lastWrite[i] = -1;

	}

	while (instructionId < basicBlockSize) {


		//**************************************************************
		// Fetching / Decoding instruction
		//**************************************************************

		ac_int<128, false> bytecode_word = bytecode[instructionId];

		ac_int<32, false> bytecode_word1 = bytecode_word.slc<32>(96);
		ac_int<32, false> bytecode_word2 = bytecode_word.slc<32>(64);
		ac_int<32, false> bytecode_word3 = bytecode_word.slc<32>(32);
		ac_int<32, false> bytecode_word4 = bytecode_word.slc<32>(0);

		ac_int<50, false> instruction = 0;
		instruction.set_slc(18, bytecode_word1);
		instruction.set_slc(0, bytecode_word2.slc<18>(14));

		// Type of Functional Unit needed by this instruction
		ac_int<2, false> unitType = getType(instruction);

		ac_int<2, false> typeCode = instruction.slc<2>(46);
		ac_int<1, false> alloc = instruction[45];
		ac_int<1, false> allocBr = instruction[44];
		ac_int<7, false> opCode = instruction.slc<7>(37);
		ac_int<1, false> isImm = instruction[36];
		ac_int<5, false> funct = instruction.slc<5>(31);
		ac_int<1, false> isBr = instruction[35];
		ac_int<9, false> virtualRDest = instruction.slc<9>(0);
		ac_int<9, false> virtualRIn2 = instruction.slc<9>(9);
		ac_int<9, false> virtualRIn1_imm9 = instruction.slc<9>(18);
		ac_int<13, false> imm13 = instruction.slc<13>(18); //TODO
		ac_int<19, false> imm19 = instruction.slc<19>(9);
		ac_int<9, false> brCode = instruction.slc<9>(27);

		// real dest register (after alloc/dereferencing)
		ac_int<6, false> dest;

		// for RAW/WAW dependencies
		ac_int<3, false> nbDataDeps    = bytecode_word2.slc<3>(3);
		ac_int<3, false> nbNonDataDeps = bytecode_word2.slc<3>(0) - nbDataDeps;
		ac_int<8, false> deps[7];
		deps[0] = bytecode_word3.slc<8>(16);
		deps[1] = bytecode_word3.slc<8>(8);
		deps[2] = bytecode_word3.slc<8>(0);
		deps[3] = bytecode_word4.slc<8>(24);
		deps[4] = bytecode_word4.slc<8>(16);
		deps[5] = bytecode_word4.slc<8>(8);
		deps[6] = bytecode_word4.slc<8>(0);

		//For alloc
		ac_int<6, false> accessPlaceOfReg = placeOfRegisters[virtualRDest];
		ac_int<6, false> accessFreeReg = freeRegisters[numberFreeRegister-1];

		//**************************************************************
		// Allocation
		//**************************************************************

		// if allocation requested (for dest)
		if (alloc) {
			// take the first free register + update register dependencies
			if (numberFreeRegister > 0) {
				numberFreeRegister--;
				dest = accessFreeReg;
				registerDependencies[instructionId] = bytecode_word2.slc<8>(6);
			} else {
				// else crash
				exit(-1);
				//return basicBlockSize+1;
			}
		} else {
			registerDependencies[instructionId] = 0xff;
			dest = accessPlaceOfReg;
		}

		//**************************************************************
		// Computing dependencies
		//**************************************************************

		ac_int<32, false> earliest_place = 0;
		// here, ternary conditions are for forwarding purpose
		for (ac_int<3, false> i = 0; i < 7; ++i) {
			ac_int<32, true> place = (deps[i] != instructionId-1)
			? placeOfInstr[deps[i]] : lastPlaceOfInstr;
			// RAW (gap depends on the type of stage because of pipeline length diffs)
			if (i < nbDataDeps) {
				ac_int<8, false> stg = (deps[i] != instructionId-1)
				? instructionsStages[deps[i]] : lastInstructionStage;
        ac_int<4, false> spec = way_specialisation.slc<4>(stg << 2);
				ac_int<2, false> gap = (spec[1] || spec[3]) ? 2 : 1;
				earliest_place = max(earliest_place, (ac_int<32,false>(place+gap)));
			}
			// WAW
			else if (6-i < nbNonDataDeps) {
				if (unitType == 0){
					ac_int<2, false> typeOfPred = bytecode[deps[i]].slc<2>(126);

					ac_int<8, false> stg = (deps[i] != instructionId-1)
					? instructionsStages[deps[i]] : lastInstructionStage;
          ac_int<4, false> spec = way_specialisation.slc<4>(stg << 2);
					ac_int<2, true> gap = (spec[1] || spec[3]) ? 0 : -1;
					ac_int<32, true> test = place+gap;

					test = !(spec[1] || spec[3]) && place == 0 ? ac_int<32, true>(windowPosition) : test;
					test = (typeOfPred == 0) ? place+2 : test+0;

					earliest_place = max(earliest_place, test);
				}
				else{
					ac_int<2, false> typeOfPred = bytecode[deps[i]].slc<2>(126);
					ac_int<32, true> test = (typeOfPred == 0) ? (ac_int<32,false>(place+2)) : (ac_int<32,false>(place+1));
					earliest_place = max(earliest_place, test);

				}
			}
		}

		// WAR
		earliest_place = max(earliest_place, lastRead[dest]);

		unsigned char shiftedOpcode = opCode>>4;

		ac_int<1, false> isNop = (opCode == 0);
		ac_int<1, false> isITypeWithDest = (opCode == VEX_MOVI || opCode == VEX_CALL);
		ac_int<1, false> isArith2 = (shiftedOpcode == 4 || shiftedOpcode == 5 || shiftedOpcode == 0);
		ac_int<1, false> isLoad = (opCode>>3) == 0x2;
		ac_int<1, false> isStore = (opCode>>3) == 0x3;
		ac_int<1, false> isArith1 = (shiftedOpcode == 6 || shiftedOpcode == 7);
		ac_int<1, false> isBranchWithReg = (opCode == VEX_CALLR) ||(opCode == VEX_GOTOR);
		ac_int<1, false> isFPOneReg = (opCode == VEX_FP && (funct == VEX_FP_FCVTSW || funct == VEX_FP_FCVTSWU || funct == VEX_FP_FCVTWS
				|| funct == VEX_FP_FCVTWUS || funct == VEX_FP_FMVWX || funct == VEX_FP_FMVXW || funct == VEX_FP_FCLASS));
		ac_int<1, false> isFPTwoReg = opCode == VEX_FP && !isFPOneReg;
		ac_int<1, false> isBranchWithTwoRegs = (opCode == VEX_BR) || (opCode == VEX_BRF) || (opCode == VEX_BGE) || (opCode == VEX_BLT) || (opCode == VEX_BGEU) || (opCode == VEX_BLTU);

		if (!isNop && !isStore && !isBranchWithReg && ! isBranchWithTwoRegs){
			earliest_place = max(earliest_place, lastWrite[dest]+1);
			earliest_place = max(earliest_place, lastRead[dest]);
		}

		ac_int<9, false> operand1 = virtualRIn1_imm9;
		ac_int<9, false> operand2 = typeCode == 2 ? virtualRDest : virtualRIn2;

		ac_int<1, false> useOperand1 = 0;
		ac_int<1, false> useOperand2 = 0;

		if (isArith2 || isFPTwoReg){
			operand2 = virtualRIn2;

			useOperand1 = 1;
			useOperand2 = 1;
		}
		else if (isStore || isBranchWithTwoRegs){
			operand1 = virtualRDest;
			operand2 = virtualRIn2;
			useOperand1 = 1;
			useOperand2 = 1;
		}
		else if (isArith1 || isLoad || isFPOneReg){
			operand2 = virtualRIn2;
			useOperand2 = 1;
		}
		else if (isBranchWithReg){
			operand1 = virtualRDest;
			useOperand1 = 1;
		}

		ac_int<6, false> placeOfOperand1, placeOfOperand2;

		if (useOperand1)
			placeOfOperand1 = placeOfRegisters[operand1];

		if (useOperand2)
			placeOfOperand2 = placeOfRegisters[operand2];

		//**************************************************************
		// Placing the instruction
		//**************************************************************

		ac_int<WINDOW_SIZE_L2+1, false> bestWindowOffset = WINDOW_SIZE;
		ac_int<STAGE_NUMBER_L2+1, false> bestStageId;

		// for explicit tree reduction purpose
		ac_int<WINDOW_SIZE, false> possible = 0;
		ac_int<STAGE_NUMBER_L2, false> bestStage[WINDOW_SIZE];
		ac_int<WINDOW_SIZE_L2, false> bestOffset[WINDOW_SIZE];

		// available places search
		for (ac_int<STAGE_NUMBER_L2+1, false> stageId_ = 0
		; stageId_ < STAGE_NUMBER; stageId_ += 1)
		{
      int stageId = priority[stageId_];
			// loop unrolled by hand to ease synthesis
			ac_int<4, false> stageType = way_specialisation.slc<4>(stageId << 2);
			if (stageType && stageType[unitType]) {
				for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
				; windowOffset < WINDOW_SIZE; ++windowOffset)
				{
					if (freeSlot[offset(windowOffset)][stageId]
					&& !possible[windowOffset]) {
						bestStage[windowOffset] = stageId;
						bestOffset[windowOffset] = windowOffset;
						possible[windowOffset] = 1;
					}
				}
			}

		}

		// updates possible[] array with the [earliest_place] constraint
		for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
		; windowOffset < WINDOW_SIZE; ++windowOffset) {

			possible[windowOffset] = possible[windowOffset] &&
			windowOffset+windowPosition >= earliest_place;
		}
		// 3 [for] loops for tree reduction over available places
		for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
		; windowOffset < WINDOW_SIZE; windowOffset += 2) {
			if (possible[windowOffset+1] && !possible[windowOffset]) {
				bestOffset[windowOffset] = bestOffset[windowOffset+1];
				bestStage[windowOffset] = bestStage[windowOffset+1];
				possible[windowOffset] = 1;
			}
		}

		for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
		; windowOffset < WINDOW_SIZE; windowOffset += 4) {
			if (possible[windowOffset+2] && !possible[windowOffset]) {
				bestOffset[windowOffset] = bestOffset[windowOffset+2];
				bestStage[windowOffset] = bestStage[windowOffset+2];
				possible[windowOffset] = 1;
			}
		}

		for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
		; windowOffset < WINDOW_SIZE; windowOffset += 8) {
			if (possible[windowOffset+4] && !possible[windowOffset]) {
				bestOffset[windowOffset] = bestOffset[windowOffset+4];
				bestStage[windowOffset] = bestStage[windowOffset+4];
				possible[windowOffset] = 1;
			}
		}
		for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
		; windowOffset < WINDOW_SIZE; windowOffset += 16) {
			if (possible[windowOffset+8] && !possible[windowOffset]) {
				bestOffset[windowOffset] = bestOffset[windowOffset+8];
				bestStage[windowOffset] = bestStage[windowOffset+8];
				possible[windowOffset] = 1;
			}
		}



		bestWindowOffset = bestOffset[0];
		bestStageId = bestStage[0];

		//**************************************************************
		// Generation + pre-placement of instruction
		//**************************************************************

		instruction.set_slc(0, ac_int<9, false>(dest));
		placeOfRegisters[instructionId] = dest;


		ac_int<8, false> operand1Dep = registerDependencies[operand1.slc<8>(0)];
		ac_int<8, false> operand2Dep = registerDependencies[operand2.slc<8>(0)];


		if (useOperand1) {

			if (!operand1[8] && operand1 == operand2) {
				operand1Dep -= 2;
			} else {
				if (!operand1[8])
					operand1Dep--;

				if (!operand2[8])
					operand2Dep--;
			}
		} else if (!operand2[8]) {
			operand2Dep--;
		}

		registerDependencies[operand1.slc<8>(0)] = operand1Dep;
		registerDependencies[operand2.slc<8>(0)] = operand2Dep;

		if (useOperand1 && !operand1[8] && operand1Dep == 0)
			freeRegisters[numberFreeRegister++] = placeOfOperand1;
		if (useOperand2 && !operand2[8] && operand2Dep == 0)
			freeRegisters[numberFreeRegister++] = placeOfOperand2;

		//***********************************************************************
		// !Place found : Write binaries + shift the window + correct placement
		//***********************************************************************
		if (!possible[0]) {
			ac_int<32, false> advance = (earliest_place > windowPosition+WINDOW_SIZE)
			? earliest_place-windowPosition-WINDOW_SIZE+1 : ac_int<35,true>(1);
			for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
			; windowOffset < 3; ++windowOffset) {
				ac_int<256, false> binariesWord;
				ac_int<WINDOW_SIZE_L2, false> off = offset(windowOffset);
				ac_int<8, false> available = freeSlot[off];

				for (ac_int<STAGE_NUMBER_L2+1, false> stageId = 0
				; stageId < STAGE_NUMBER; ++stageId) {
					binariesWord.set_slc(  stageId*32
					, available[stageId] ? zero32 : window[off][stageId]);


				}

				if (windowOffset < advance)
					freeSlot[off] = 0xFF;

				if (issue_width<=4) {
					binaries[addressInBinaries+windowPosition+windowOffset] = binariesWord.slc<128>(0);
				} else {
					binaries[addressInBinaries+(windowPosition+windowOffset)*2] = binariesWord.slc<128>(0);
					binaries[addressInBinaries+(windowPosition+windowOffset)*2+1] = binariesWord.slc<128>(128);
				}
			}


			windowPosition += advance;
			windowShift = (windowShift+(advance))%WINDOW_SIZE;

			for (ac_int<STAGE_NUMBER_L2+1, false> stageId_ = 0
			; stageId_ < STAGE_NUMBER; ++stageId_) {
				int stageId = priority[stageId_];
				ac_int<4, false> stageType = way_specialisation.slc<4>(stageId << 2);
				if (stageType && stageType[unitType]) {
					bestStageId = stageId;
					break;
				}
			}

			bestWindowOffset = WINDOW_SIZE-1;
		}
		//****************************************************************
		// Writing instruction into the window buffer
		//****************************************************************


		lastPlaceOfInstr = windowPosition + bestWindowOffset;
		placeOfInstr[instructionId] = windowPosition + bestWindowOffset;

		lastInstructionStage = bestStageId;
		instructionsStages[instructionId] = bestStageId;

		if (useOperand2){
			lastRead[placeOfOperand2] = max(lastPlaceOfInstr, lastRead[placeOfOperand2]);
		}
		if (useOperand1){
			lastRead[placeOfOperand1] = max(lastPlaceOfInstr,lastRead[placeOfOperand1]);
		}

		if (!isNop && !isStore && !isBranchWithReg && ! isBranchWithTwoRegs)
			lastWrite[dest] = max(lastPlaceOfInstr,lastWrite[dest]);

		window[offset(bestWindowOffset)][bestStageId] = createInstruction(instruction, placeOfOperand1, placeOfOperand2, dest);
		freeSlot[offset(bestWindowOffset)][bestStageId] = 0;



		if (instruction.slc<2>(48) == 0) {
			haveJump = 1;
			jumpPlace = placeOfInstr[instructionId];
		}

		instructionId++;

		#ifndef __CATAPULT
		//Performance simulation
		timeTakenIRScheduler += 4;
		#endif

	}

	//**************************************************************
	// Write remaining binaries + find the last word which contains instructions
	//**************************************************************

	ac_int<256, false> lastWord = 0;
	ac_int<32, false> lastGap = 0;
	for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
	; windowOffset < WINDOW_SIZE; ++windowOffset) {
		ac_int<256, false> binariesWord;
		ac_int<WINDOW_SIZE_L2, false> off = offset(windowOffset);
		ac_int<8, false> available = freeSlot[off];

		for (ac_int<STAGE_NUMBER_L2+1, false> stageId = 0
		; stageId < STAGE_NUMBER; ++stageId) {
			binariesWord.set_slc(  stageId*32
			, available[stageId] ? zero32 : window[off][stageId]);

		}
		if (available != 0xff) {
			lastGap = windowOffset;
			lastWord = binariesWord;
		}

		freeSlot[off] = 0xFF;
		if (issue_width<=4) {
			binaries[addressInBinaries+windowPosition+windowOffset] = binariesWord.slc<128>(0);
		} else {
			binaries[addressInBinaries+(windowPosition+windowOffset)*2] = binariesWord.slc<128>(0);
			binaries[addressInBinaries+(windowPosition+windowOffset)*2+1] = binariesWord.slc<128>(128);
		}
	}


	ac_int<32, false> newSize = (issue_width>4 ? 2 : 1)*(windowPosition+lastGap + 1);

	for (int stageId = 0; stageId<issue_width; stageId++){
		ac_int<7, false> opcode = lastWord.slc<7>(stageId*32);
        ac_int<4, false> spec = way_specialisation.slc<4>(stageId << 2);

		if (opcode != 0 && ((spec[1] && (opcode > 0x1f || opcode < VEX_STB))
				|| spec[3]
				|| (spec[0] && (opcode == VEX_BR || opcode == VEX_BRF || opcode == VEX_BGE || opcode == VEX_BLT || opcode == VEX_BGEU || opcode == VEX_BLTU || opcode == VEX_CALL || opcode == VEX_CALLR || opcode == VEX_GOTO || opcode == VEX_GOTOR)))){
			newSize += (issue_width>4 ? 2 : 1);
			if (issue_width <= 4) {
				binaries[addressInBinaries + newSize-1] = 0;
			} else {
				binaries[addressInBinaries + newSize-1] = 0;
				binaries[addressInBinaries + newSize-2] = 0;
			}
			break;
		}
	}


	ac_int<32, false> newEnd = addressInBinaries+newSize;
	#ifndef __CATAPULT
	//Performance simulation
	timeTakenIRScheduler += 8;
	#endif

	return newSize;
}

#endif
#endif
