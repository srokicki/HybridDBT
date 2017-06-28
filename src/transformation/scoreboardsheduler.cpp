// window constants
const int STAGE_NUMBER_L2 = 3;
const int STAGE_NUMBER = 8;
const int WINDOW_SIZE_L2  = 3;
const int WINDOW_SIZE  = 8;

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
ac_int<32, false> lastRead[64];

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
ac_int<32, false> createInstruction(ac_int<50, false> instruction) {

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

	//***************************************
	//We generate the instruction
	ac_int<32, false> generatedInstruction = 0;
	generatedInstruction.set_slc(0, opCode);
	generatedInstruction.set_slc(26, ac_int<6>(virtualRIn2));

	if (typeCode == 0) { //The instruction is R type

		if (isImm) {
			generatedInstruction.set_slc(7, imm13);
			generatedInstruction.set_slc(20, ac_int<6>(virtualRDest));
		}
		else{
			generatedInstruction.set_slc(14, ac_int<6>(virtualRDest));
			generatedInstruction.set_slc(20, ac_int<6>(virtualRIn1_imm9));
		}
	}
	else { //The instruction is I Type
		if (opCode == 0x28) {
			generatedInstruction.set_slc(26, ac_int<6>(virtualRDest));
		}
		else{
			generatedInstruction.set_slc(26, ac_int<6>(virtualRDest));
		}
		generatedInstruction.set_slc(7, imm19);
	}

	return generatedInstruction;
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
ac_int<4, false> issue_width, // TODO: change to 1 boolean per issue
ac_int<MAX_ISSUE_WIDTH * 2, false> way_specialisation,
ac_int<32, false> placeOfInstr[256]
){
	//**************************************************************
	// Setup scheduler state
	//**************************************************************

	haveJump = 0;
	instructionId = 0;
	windowPosition = addressInBinaries;
	windowShift = 0;

	for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
	; windowOffset < WINDOW_SIZE; ++windowOffset) {
			stageWindow[windowOffset] = 0;
			freeSlot[windowOffset] = 0xFF;
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

		//**************************************************************
		// Allocation
		//**************************************************************

		// if allocation requested (for dest)
		if (alloc) {
			// take the first free register + update register dependencies
			if (numberFreeRegister > 0) {
				dest = freeRegisters[--numberFreeRegister];
				registerDependencies[instructionId] = bytecode_word2.slc<8>(6);
			} else {
				// else crash
				return basicBlockSize+1;
			}
		} else {
			dest = placeOfRegisters[virtualRDest];
		}


		ac_int<32, false> earliest_place = 0;

		// here, ternary conditions are for forwarding purpose
		for (ac_int<3, false> i = 0; i < 7; ++i) {
			ac_int<32, false> place = (deps[i] != instructionId-1) ? placeOfInstr[deps[i]] : lastPlaceOfInstr;

			// RAW (gap depends on the type of stage because of pipeline length diffs)
			if (i < nbDataDeps) {
				ac_int<8, false> stg = (deps[i] != instructionId-1) ? instructionsStages[deps[i]] : lastInstructionStage;
				ac_int<2, false> gap = (stg == 0 || stg == 3) ? 2 : 3;
				earliest_place = max(earliest_place, (ac_int<32,false>(place+gap)));
			}
			// WAW
			else if (6-i < nbNonDataDeps) {
				earliest_place = max(earliest_place, (ac_int<32,false>(place+1)));
			}
		}

		// WAR
		earliest_place = max(earliest_place, lastRead[dest]);

		//**************************************************************
		// Placing the instruction
		//**************************************************************

		ac_int<WINDOW_SIZE_L2+1, false> bestWindowOffset = WINDOW_SIZE;
		ac_int<STAGE_NUMBER_L2+1, false> bestStageId;

		// for explicit tree reduction purpose
		ac_int<8, false> bestFound[STAGE_NUMBER] = { false };
		ac_int<WINDOW_SIZE_L2, false> bestOffset[STAGE_NUMBER];
		ac_int<STAGE_NUMBER_L2, false> bestStage[STAGE_NUMBER] = {0,1,2,3,4,5,6,7};

		// Find the 'best' place in the current window for each stage
		for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
		; windowOffset < WINDOW_SIZE; ++windowOffset)
		{
			ac_int<STAGE_NUMBER*32, false> windowWord = stageWindow[offset(windowOffset)];
			for (ac_int<STAGE_NUMBER_L2+1, false> stageId = 0
			; stageId < STAGE_NUMBER; ++stageId)
			{
				ac_int<2, false> stageType;
				stageType = way_specialisation(stageId << 1);
				if (issue_width < stageId && (unitType == stageType || unitType == 2)
				&& freeSlot[offset(windowOffset)][stageId]
				&& windowPosition+windowOffset >= earliest_place
				&& !bestFound[stageId]) {
					bestOffset[stageId] = windowOffset;
					bestFound[stageId] = 1;
				}
			}
		}

		// compute tree reduction on the stages
		for (ac_int<4, false> depth = 1; depth < 8; depth <<= 1) {

			for (ac_int<STAGE_NUMBER_L2+1, false> stageId = 0
			; stageId < STAGE_NUMBER; stageId += (depth << 1)) {
				if (bestFound[stageId]
				&& bestOffset[stageId] < bestOffset[stageId+depth]) {
					bestOffsets[stageId] = bestOffsets[stageId+depth];
					bestStage[stageId]   = bestStage[stageId+depth];
					bestFound[stageId]   = true;
				}
			}
		}

		bestWindowOffset = bestOffset[0];
		bestStageId			 = bestStage[0];

		//**************************************************************
		//  Place found : Schedule the instruction + next
		// !Place found : Write binaries + shift the window + retry scheduling
		//**************************************************************

		if (bestFound[0]) {

			for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
			; windowOffset < 3; ++windowOffset) {
				ac_int<256, false> binariesWord;
				ac_int<WINDOW_SIZE_L2, false> off = offset(windowOffset);
				ac_int<8, false> available = freeSlot[off];

				for (ac_int<STAGE_NUMBER_L2+1, false> stageId = 0
				; stageId < STAGE_NUMBER; ++stageId) {
					binariesWord.set_slc(  stageId*32
					, available[stageId] ? zero32 : stageWindow[off][stageId]);
				}

				freeSlot[off] = 0xFF;

				if (issue_width < 5) {
					//binaries[windowPosition+windowOffset] = stageWindow[offset(windowOffset)].slc<128>(0);
					binaries[windowPosition+windowOffset] = binariesWord.slc<128>(0);
				} else {
					//binaries[2*(windowPosition+windowOffset)] = stageWindow[offset(windowOffset)].slc<128>(0);
					//binaries[2*(windowPosition+windowOffset)+1] = stageWindow[offset(windowOffset)].slc<128>(128);
					binaries[2*(windowPosition+windowOffset)] = binariesWord.slc<128>(0);
					binaries[2*(windowPosition+windowOffset)+1] = binariesWord.slc<128>(128);
				}
			}
		} else {
			lastPlaceOfInstr = windowPosition + bestWindowOffset;
			placeOfInstr[instructionId] = windowPosition + bestWindowOffset;

			lastInstructionStage = bestStageId;
			instructionsStages[instructionId] = bestStageId;

			instruction.set_slc(0, ac_int<9, false>(dest));
			placeOfRegisters[instructionId] = dest;

			ac_int<9, false> rin1 = virtualRIn1_imm9;
			ac_int<9, false> rin2 = virtualRIn2;

			instruction.set_slc(9, placeOfRegisters[virtualRIn2]);

			ac_int<8, false> rin1Dep = registerDependencies[rin1];
			ac_int<8, false> rin2Dep = registerDependencies[rin2];

			if (rin2 < 256 && rin2Dep == 1) {
				freeRegisters[numberFreeRegister++] = rin2;
			}

			if (typeCode == 0 && !isImm) {

					lastRead[placeOfRegisters[virtualRIn1_imm9]] = lastPlaceOfInstr;
					instruction.set_slc(18, rin1);

					if (rin1 < 256 && rin1Dep == 1) {
						freeRegisters[numberFreeRegister++] = rin1;
					}

					if (rin1 < 256 && rin1 == rin2) {
						registerDependencies[rin1] = rin1Dep - 2;
					} else {
						if (rin1 < 256)
							registerDependencies[rin1] = rin1Dep - 1;

						if (rin2 < 256)
							registerDependencies[rin2] = rin2Dep - 1;
					}
			} else if (rin2 < 256) {
				registerDependencies[rin2] = rin2Dep - 1;
			}

			lastRead[placeOfRegisters[virtualRIn2]] = lastPlaceOfInstr;

			stageWindow[offset(bestWindowOffset)].set_slc(bestStageId*32, createInstruction(instruction));

			if (instruction.slc<2>(48) == 0) {
				haveJump = 1;
				jumpPlace = placeOfInstr[instructionId];
			}

			instructionId++;
		}
	}

	//**************************************************************
	// Write remaining binaries + find the last word which contains instructions
	//**************************************************************

	ac_int<32, false> lastGap;
	for (ac_int<WINDOW_SIZE_L2+1, false> windowOffset = 0
	; windowOffset < 3; ++windowOffset) {
		ac_int<256, false> binariesWord;
		ac_int<WINDOW_SIZE_L2, false> off = offset(windowOffset);
		ac_int<8, false> available = freeSlot[off];

		for (ac_int<STAGE_NUMBER_L2+1, false> stageId = 0
		; stageId < STAGE_NUMBER; ++stageId) {
			binariesWord.set_slc(  stageId*32
			, available[stageId] ? zero32 : stageWindow[off][stageId]);
		}

		if (available) {
			lastGap = windowOffset;
		}

		freeSlot[off] = 0xFF;
		if (issue_width < 5) {
			//binaries[windowPosition+windowOffset] = stageWindow[offset(windowOffset)].slc<128>(0);
			binaries[windowPosition+windowOffset] = binariesWord.slc<128>(0);
		} else {
			//binaries[2*(windowPosition+windowOffset)] = stageWindow[offset(windowOffset)].slc<128>(0);
			//binaries[2*(windowPosition+windowOffset)+1] = stageWindow[offset(windowOffset)].slc<128>(128);
			binaries[2*(windowPosition+windowOffset)] = binariesWord.slc<128>(0);
			binaries[2*(windowPosition+windowOffset)+1] = binariesWord.slc<128>(128);
		}
	}

	return (issue_width > 4 ? 2 : 1)*(windowPosition+lastGap+1)-addressInBinaries;
}
