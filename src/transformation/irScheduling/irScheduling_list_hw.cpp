/*
 * irScheduling_list_hw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#include <transformation/irScheduler.h>

#ifndef __HW
#ifndef __SW

#ifndef __CATAPULT
#include <lib/log.h>
#endif

char countToFail;

int latencies[4] = {4,4,4,4};   //The latencies of the different pipeline stages
unsigned int mask[4] = {0xff000000, 0xff0000, 0xff00, 0xff};

ac_int<6, false> stages[8] = {0,3,1,2,4,5,6,7};

//Memory units of the architecture
ac_int<32, false> instructions[256];
static bool dummy1 = ac::init_array<AC_VAL_DC>(instructions, 256);
ac_int<18, false> instructionsEnd[256];
static bool dummy2 = ac::init_array<AC_VAL_DC>(instructionsEnd, 256);
ac_int<8, false> priorities[256];
static bool dummy3 = ac::init_array<AC_VAL_DC>(priorities, 256);
ac_int<8, false> numbersOfDependencies[256];
static bool dummy4 = ac::init_array<AC_VAL_DC>(numbersOfDependencies, 256);

ac_int<3, false> numbersOfRegisterDependencies[256];
ac_int<5, false> writeFreeRegister = 0;
ac_int<5, false> readFreeRegister = 0;



unsigned char globalOptLevel;

unsigned char firstAdd;
unsigned char firstMult;
unsigned char firstMem;
unsigned char firstBr;

ac_int<9, false> readyList[64]; //Correspond to the chained list : [Enable(1), InsNumber(8)]
ac_int<16, false> readyListNext[64]; // Correspond to the end of the chained list : [next(8), priorNext(8)]


ac_int<6, false> writePlace[64] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
ac_int<6, false> writePlaceRead =0;
ac_int<6, false> writePlaceWrite = 0;

ac_int<6, false> first[4];
static bool dummy7 = ac::init_array<AC_VAL_DC>(first, 4);
ac_int<6, false> last[4];
static bool dummy8 = ac::init_array<AC_VAL_DC>(last, 4);
ac_int<8, false> firstPriorities[4] = {0,0,0,0};

ac_int<6, false> readyNumber[4] = {0,0,0,0};


ac_int<8, false> returnGetFirstNumber;
ac_int<1,false> returnGetFirstEnable;


unsigned char stall =0;
unsigned char aliveInstructions=0;

ac_int<8, false> reservationTableNum[4][MAX_ISSUE_WIDTH];
ac_int<1,false> reservationTableEnable[4][MAX_ISSUE_WIDTH] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};


ac_int<8, false> fifoInsertReadyInstruction[64];
static bool dummy13 = ac::init_array<AC_VAL_DC>(fifoInsertReadyInstruction, 64);
ac_int<6, false> fifoPlaceToWrite = 0;
ac_int<6, false> fifoPlaceToRead = 0;
ac_int<6, false> fifoNumberElement = 0;

ac_int<8, false> scheduledInstructions = 0;

//We declare array for used registers and used successors
ac_int<8*7, false> successors[4][2][MAX_ISSUE_WIDTH];
ac_int<3, false> successorNumbers[4][2][MAX_ISSUE_WIDTH];
ac_int<3, false> successorStageNumber[4][2] = {{0,0},{0,0},{0,0},{0,0}};

ac_int<8, false> totalNumberOfSuccessors[4];
ac_int<8, false> totalNumberOfDataSuccessors[4];


ac_int<9, false> usedRegister[4][3*MAX_ISSUE_WIDTH];
ac_int<5, false> numbersUsedRegister[4];



int getType(unsigned int ins){

	return (ins >> 30);
}

void insertReadyInstruction(ac_int<8, false> instructionNumber){
	unsigned int instruction = instructions[instructionNumber];
	ac_int<8, false> instructionPriority = priorities[instructionNumber];
		ac_int<2, false> type = getType(instruction);
		ac_int<6, false> place;

		if ((writePlaceRead + 1) == writePlaceWrite){
						stall = 1;
					}
				else{
			place = writePlace[writePlaceRead];
			writePlaceRead++;

			ac_int<9, false> tempReadyElement;

			tempReadyElement[8] = 1;
			tempReadyElement.set_slc(0, instructionNumber);
			readyList[place] = tempReadyElement;





			if (readyNumber[type] == 0){
					first[type] = place;
					last[type] = place;
				}
				else	if (instructionPriority[7]){
						// We insert on the top of the list
						ac_int<16, false> tempNextElement;
						tempNextElement.set_slc(8, first[type]);
						readyListNext[place] = tempNextElement;
						first[type] = place;

					}
				else {
						// we insert at the end of the list
						ac_int<16, false> tempNextElement;
						tempNextElement.set_slc(8, place);
						readyListNext[last[type]] = tempNextElement;
						last[type] = place;

					}

				readyNumber[type]++;
					}
}

void insertReadyInstructionOpt(ac_int<8, false> instructionNumber){

	unsigned int instruction = instructions[instructionNumber];
	ac_int<8, false> instructionPriority = priorities[instructionNumber];
		ac_int<2, false> type = getType(instruction);
		ac_int<6, false> place;

		if ((writePlaceRead + 1) == writePlaceWrite){
						stall = 1;
					}
				else{
			place = writePlace[writePlaceRead];
			writePlaceRead++;

			ac_int<9, false> tempReadyElement;
			ac_int<16, false> tempNextElement;

			tempReadyElement[8] = 1;
			tempReadyElement.set_slc(0, instructionNumber);
			readyList[place] = tempReadyElement;


			if (instructionPriority >= firstPriorities[type] || readyNumber[type] == 0){

									tempNextElement.set_slc(8, first[type]);
								tempNextElement.set_slc(0, firstPriorities[type]);
								first[type] = place;
									firstPriorities[type] = instructionPriority;
								}
							else {
									ac_int<6, false> currentPlace = first[type];
									ac_int<8, false> currentPriority = 255;

									while (readyList[readyListNext[currentPlace].slc<8>(8)][8] && readyListNext[currentPlace].slc<8>(0) > instructionPriority && /*readyListNext[currentPlace].slc<8>(0) < currentPriority &&*/ currentPlace != readyListNext[currentPlace].slc<8>(8)){
											currentPriority = readyListNext[currentPlace].slc<8>(0);
										currentPlace = readyListNext[currentPlace].slc<8>(8);
										}

									tempNextElement.set_slc(8, readyListNext[currentPlace].slc<8>(8));
									readyListNext[currentPlace].set_slc(8, place);
									tempNextElement.set_slc(0, readyListNext[currentPlace].slc<8>(0));
									readyListNext[currentPlace].set_slc(0, instructionPriority);
								}

							readyListNext[place] = tempNextElement;
					readyNumber[type]++;
					}
}

void getFirstInstruction(int type){
	if (readyNumber[type] > 0){
			returnGetFirstNumber = readyList[first[type]].slc<8>(0);
				returnGetFirstEnable = 1;
				readyList[first[type]][8] = 0;
				writePlace[writePlaceWrite] = first[type];
				firstPriorities[type] = readyListNext[first[type]].slc<8>(0);
				first[type] = readyListNext[first[type]].slc<8>(8);
				readyNumber[type]--;
				writePlaceWrite++;
		}
	else if (readyNumber[2] > 0){
		returnGetFirstNumber = readyList[first[2]].slc<8>(0);
		returnGetFirstEnable = 1;
		readyList[first[2]][8] = 0;
		writePlace[writePlaceWrite] = first[2];
		firstPriorities[2] = readyListNext[first[2]].slc<8>(0);
		first[2] = readyListNext[first[2]].slc<8>(8);
		readyNumber[2]--;
		writePlaceWrite++;
		}
	else {
			returnGetFirstEnable = 0;
		}
}

void getFirstInstructionComplexType(ac_int<4, false> complexeType){

	ac_int<2, false> subType1, subType2;
	subType1 = (complexeType[0]) ? 0 : ((complexeType[1]) ? 1 : ((complexeType[3]) ? 3 : 2));
	subType2 = (complexeType[1] && complexeType[3]) ? 3 : 2;

	if (complexeType == 0){
		returnGetFirstEnable = 0;
	}
	else if (readyNumber[subType1] > 0){
			returnGetFirstNumber = readyList[first[subType1]].slc<8>(0);
				returnGetFirstEnable = 1;
				readyList[first[subType1]][8] = 0;
				writePlace[writePlaceWrite] = first[subType1];
				firstPriorities[subType1] = readyListNext[first[subType1]].slc<8>(0);
				first[subType1] = readyListNext[first[subType1]].slc<8>(8);
				readyNumber[subType1]--;
				writePlaceWrite++;

		}
	else if (readyNumber[subType2] > 0){
		returnGetFirstNumber = readyList[first[subType2]].slc<8>(0);
		returnGetFirstEnable = 1;
		readyList[first[subType2]][8] = 0;
		writePlace[writePlaceWrite] = first[subType2];
		firstPriorities[subType2] = readyListNext[first[subType2]].slc<8>(0);
		first[subType2] = readyListNext[first[subType2]].slc<8>(8);
		readyNumber[subType2]--;
		writePlaceWrite++;
		}
	else if (readyNumber[2] > 0){
		returnGetFirstNumber = readyList[first[2]].slc<8>(0);
		returnGetFirstEnable = 1;
		readyList[first[2]][8] = 0;
		writePlace[writePlaceWrite] = first[2];
		firstPriorities[2] = readyListNext[first[2]].slc<8>(0);
		first[2] = readyListNext[first[2]].slc<8>(8);
		readyNumber[2]--;
		writePlaceWrite++;
		}
	else {
			returnGetFirstEnable = 0;
		}
}

/*
 * FIXME: current implementation only works for 4-issue VLIW: need to increase size of vliw binaries and of the binaries word
 * used to write in it.
 *
 */

//The argument optLevel is here to set the difficulty of the scheduling : 0 mean that there is just a binary priority and 1 mean that we'll consider the entire priority value
ac_int<32, false> irScheduler_list_hw(ac_int<1, false> optLevel,
		ac_int<8, false> basicBlockSize,
		ac_int<128, false> bytecode[256],
		ac_int<128, false> binaries[65536],
		ac_int<32, false> addressInBinaries,
		ac_int<6, false> placeOfRegisters[512],
		ac_int<6, false> numberFreeRegister,
		ac_int<6, false> freeRegisters[64],
		ac_int<4, false> issue_width,
		ac_int<MAX_ISSUE_WIDTH * 4, false> way_specialisation,
		ac_int<32, false> placeOfInstr[256]){

		ac_int<32, false> cycleNumber = 0; //This is the current cycle
		ac_int<2, false> lineNumber = 0;
		ac_int<32,false> writeInBinaries =addressInBinaries;

		countToFail = 0;
		for (int i=0; i<64; i++){
				writePlace[i] = i;
			}
		writePlaceRead =0;
		writePlaceWrite = 0;

		readFreeRegister = 0;
		writeFreeRegister = numberFreeRegister;

		readyNumber[0] = 0;
		readyNumber[1] = 0;

		readyNumber[2] = 0;
		readyNumber[3] = 0;


		stall =0;
		aliveInstructions=0;

		fifoPlaceToWrite = 0;
		fifoPlaceToRead = 0;
		fifoNumberElement = 0;

		scheduledInstructions = 0;

		globalOptLevel = optLevel;

		ac_int<32, false> jumpPlace = 0;
		ac_int<1, false> haveJump = 0;

	for(int i = 0; i<basicBlockSize;i++){
		instructions[i] = bytecode[i].slc<32>(96);
		instructionsEnd[i] = bytecode[i].slc<18>(14+64);
		numbersOfDependencies[i] = bytecode[i].slc<8>(6+64);
		priorities[i] = bytecode[i].slc<8>(24+32);
		ac_int<3, false> const7 = 7;
		numbersOfRegisterDependencies[i] = (bytecode[i][96+27]) ? bytecode[i].slc<3>(3+64) : const7;

		if (numbersOfDependencies[i] == 0){
				if (fifoNumberElement == 64)
					stall = 1;
				else{
						fifoInsertReadyInstruction[fifoPlaceToWrite] = i;
							fifoPlaceToWrite++;
							fifoNumberElement++;
							numbersOfDependencies[i]--;
					}
		}
	}

	while (1){

				// The scheduling will be done in three steps : the first one will solve
				// dependencies and find ready instructions. The second one will give an
				// instruction to every free functional unit. The final one will commit
				// finished instructions. A loop correspond to a time cycle.
				if (stall){
					stall = 0;
				for (int i = 0; i < basicBlockSize; i++){

				if (numbersOfDependencies[i] == 0){
						if (fifoNumberElement == 64){
									stall = 1;
									break;
								}
							else{
									fifoInsertReadyInstruction[fifoPlaceToWrite] = i;
									fifoPlaceToWrite++;
									fifoNumberElement++;
									numbersOfDependencies[i]--;
								}
				}
					}
				}

			if (optLevel)
					for (int i=0; i<fifoNumberElement; i++){
								insertReadyInstructionOpt(fifoInsertReadyInstruction[fifoPlaceToRead]);
										fifoPlaceToRead++;
							}
				else
					for (int i=0; i<fifoNumberElement; i++){
								insertReadyInstruction(fifoInsertReadyInstruction[fifoPlaceToRead]);
										fifoPlaceToRead++;
							}

				fifoNumberElement = 0;

				ac_int<256, false> binariesWord = 0;

				//For each functional unit, we assign the most prior instruction
				for (ac_int<6, false> stageIndex = 0; stageIndex < issue_width; stageIndex++){

						ac_int<6, false> stage = stages[stageIndex];

					//We read functional unit configuration
					ac_int<4, false> complexType = way_specialisation.slc<4>(stage<<2);
					getFirstInstructionComplexType(complexType);

				//Depending on the stage type we find the lineNumber
				//  -> For normal instructions, this number correspond to current line number ('lineNumber')
				//  -> For multiplication, this number is the next lineNumber (lineNumber + 1 % 2)

				ac_int<2, false> nextLineNumber = lineNumber + 1;
				ac_int<2, false> lineNumberForStage = (complexType[1] || complexType[3]) ?  nextLineNumber : lineNumber; //Note : the modulo is useless but here to remind that it is a 1 bit variable


			ac_int<32, false> generatedInstruction = 0;
			ac_int<1, false> isSchedulable = 0;


			//We check if we found an instruction to insert
			if (returnGetFirstEnable) {

					isSchedulable = 1;

					//We read the bytecode
					ac_int<128, false> bytecode_word = bytecode[returnGetFirstNumber];
					ac_int<32, false> bytecode_word1 = bytecode_word.slc<32>(96);
					ac_int<32, false> bytecode_word2 = bytecode_word.slc<32>(64);
					ac_int<32, false> bytecode_word3 = bytecode_word.slc<32>(32);
					ac_int<32, false> bytecode_word4 = bytecode_word.slc<32>(0);


				ac_int<50, false> instruction = 0;
				instruction.set_slc(18, bytecode_word1);
				instruction.set_slc(0, bytecode_word2.slc<18>(14));

				//We split different information from the instruction:
				ac_int<2, false> typeCode = instruction.slc<2>(46);
				ac_int<1, false> alloc = instruction[45];
				ac_int<7, false> opCode = instruction.slc<7>(37);
				ac_int<1, false> isImm = instruction[36];
				ac_int<9, false> virtualRDest = instruction.slc<9>(0);
				ac_int<9, false> virtualRIn2 = instruction.slc<9>(9);
				ac_int<9, false> virtualRIn1_imm9 = instruction.slc<9>(18);
				ac_int<13, false> imm13 = instruction.slc<13>(18); //TODO
				ac_int<19, false> imm19 = instruction.slc<19>(9);
				ac_int<9, false> brCode = instruction.slc<9>(27);

				ac_int<6, false> dest;

				//If the instruction allocate a new register (if alloc is equal to one), we choose a new register in the list of free registers
				if (alloc){
					if (numberFreeRegister != 0){
						dest = freeRegisters[readFreeRegister];
						placeOfRegisters[returnGetFirstNumber] = dest;
						readFreeRegister++;
						numberFreeRegister--;
						countToFail=0;
					}
					else {
						countToFail++;
						if (countToFail == 7){
							#ifndef __CATAPULT
							Log::logError << "Failed at allocating registers...\n";
							exit(-1);
							#endif
						}
						//There is no free registers, we need to cancel the current instruction being scheduled
						fifoInsertReadyInstruction[fifoPlaceToWrite] = returnGetFirstNumber;
						fifoPlaceToWrite++;
						fifoNumberElement++;
						isSchedulable = 0;
					}

				}
				else if (typeCode != 2 || opCode == 0x28){ //If we are not a I Type different than movi
					dest = placeOfRegisters[virtualRDest];
					placeOfRegisters[returnGetFirstNumber] = dest;

				}


				//If the allocation of the instruction is validated
				if (isSchedulable){

					scheduledInstructions++;

					if (instruction.slc<2>(48) == 0){
						//This is a jump
						haveJump = 1;
						jumpPlace = writeInBinaries;
					}

					//***************************************
					//We list used registers
					if (typeCode == 0){
						usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn2;
						numbersUsedRegister[lineNumberForStage]++;
						if (!isImm || opCode.slc<4>(3) == 3){
							usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = (opCode.slc<4>(3) == 3) ? virtualRDest : virtualRIn1_imm9;
							numbersUsedRegister[lineNumberForStage]++;

						}
					}
					else if (typeCode == 1){
						usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn2;
						numbersUsedRegister[lineNumberForStage]++;
						usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = brCode;
						numbersUsedRegister[lineNumberForStage]++;
						if (!isImm){
							usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = virtualRIn1_imm9;
							numbersUsedRegister[lineNumberForStage]++;
						}
					}
					else if (typeCode == 2 && opCode != 0x28){
						usedRegister[lineNumberForStage][numbersUsedRegister[lineNumberForStage]] = dest;
						numbersUsedRegister[lineNumberForStage]++;
					}

					//***************************************
					//We list all successors
					ac_int<56, false> successorNames;
					ac_int<56, false> dataSuccessorNames;

					ac_int<3, false> numberOfSuccessor, numberOfDataSuccessor;


					numberOfDataSuccessor = bytecode_word2.slc<3>(3);
					numberOfSuccessor = bytecode_word2.slc<3>(0) - numberOfDataSuccessor;


					successorNames.set_slc(32, bytecode_word3.slc<24>(0));
					successorNames.set_slc(0, bytecode_word4);

					dataSuccessorNames.set_slc(0, bytecode_word3.slc<8>(16));
					dataSuccessorNames.set_slc(8, bytecode_word3.slc<8>(8));
					dataSuccessorNames.set_slc(16, bytecode_word3.slc<8>(0));
					dataSuccessorNames.set_slc(24, bytecode_word4.slc<8>(24));
					dataSuccessorNames.set_slc(32, bytecode_word4.slc<8>(16));
					dataSuccessorNames.set_slc(40, bytecode_word4.slc<8>(8));
					dataSuccessorNames.set_slc(48, bytecode_word4.slc<8>(0));


					if (numberOfSuccessor != 0){
						successorNumbers[lineNumber][0][successorStageNumber[lineNumber][0]] = numberOfSuccessor;
						successors[lineNumber][0][successorStageNumber[lineNumber][0]] = successorNames;
						totalNumberOfSuccessors[lineNumber] += numberOfSuccessor;
						successorStageNumber[lineNumber][0]++;
					}

					if (numberOfDataSuccessor != 0){
						successorNumbers[lineNumberForStage][1][successorStageNumber[lineNumberForStage][1]] = numberOfDataSuccessor;
						successors[lineNumberForStage][1][successorStageNumber[lineNumberForStage][1]] = dataSuccessorNames;
						totalNumberOfDataSuccessors[lineNumberForStage] += numberOfDataSuccessor;
						successorStageNumber[lineNumberForStage][1]++;
					}

					//***************************************
					//We store the place
					placeOfInstr[returnGetFirstNumber] = writeInBinaries-addressInBinaries;

					//***************************************
					//We generate the instruction
					generatedInstruction.set_slc(0, opCode);
					generatedInstruction.set_slc(26, placeOfRegisters[virtualRIn2]);

					if (typeCode == 0){ //The instruction is R type

						if (isImm){
							generatedInstruction.set_slc(7, imm13);
							generatedInstruction.set_slc(20, dest);
						}
						else{
							generatedInstruction.set_slc(14, dest);
							generatedInstruction.set_slc(20, placeOfRegisters[virtualRIn1_imm9]);
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
							generatedInstruction.set_slc(26, placeOfRegisters[virtualRDest]);
						}
						generatedInstruction.set_slc(7, imm19);
					}
				}
							}
						binariesWord.set_slc(stage*32, generatedInstruction);
					}

				//after all stages has been filled, we commit the word
		binaries[writeInBinaries] = binariesWord.slc<128>(0);
		binaries[writeInBinaries + 1] = binariesWord.slc<128>(128);
		writeInBinaries++;

				if (issue_width > 4){
						writeInBinaries++;
					}

				if (scheduledInstructions >= basicBlockSize)
					break;

				cycleNumber++;

				if (issue_width > 4){
						cycleNumber++;
					}

				//Next cycle
				//   cycleNumber = cycleNumber+1;
//        lineNumber = (lineNumber + 1) & 0x1;
//        if (lineNumber == 3)
//        	lineNumber = 0;*
				ac_int<2, false> oldLineNumber = lineNumber;
				lineNumber++; //Note to change this implementation for software version



				ac_int<9, false> previousRegisterName = 511; //this value will never lead to successor reduction
				ac_int<3, false> previousRegNumberOfDep = 0;

				for (int i=0; i<numbersUsedRegister[oldLineNumber]; i++){
						if ((usedRegister[oldLineNumber][i][8] != 1)){
				ac_int<3, false> numberDep = (usedRegister[oldLineNumber][i] == previousRegisterName) ?	previousRegNumberOfDep : numbersOfRegisterDependencies[usedRegister[oldLineNumber][i]];

				numberDep--;
				numbersOfRegisterDependencies[usedRegister[oldLineNumber][i]] = numberDep;

				ac_int<6, false> place = placeOfRegisters[usedRegister[oldLineNumber][i]].slc<6>(0);


				if (numberDep == 0){
						freeRegisters[writeFreeRegister] = place;
						writeFreeRegister++;
						numberFreeRegister++;
				}

				previousRegisterName = usedRegister[oldLineNumber][i];
				previousRegNumberOfDep = numberDep;
						}
					}
				numbersUsedRegister[oldLineNumber] = 0;


				//Loop for successor reduction. Using previous* values as bypass, we should be able to pipeline the loop with
				// an iteration interval of one (by removing memory dependency on 'numberOfDependencies')
				ac_int<8, false> previousSuccessorName = 255;
				ac_int<8, false> previousNumberOfDep = 0;

				ac_int<3, false> stage = 0;
				ac_int<6, false> successorNumberInStage = 0;
				ac_int<1, false> type = (totalNumberOfSuccessors[oldLineNumber] == 0) ? 1 : 0;


				for(int i=0; i<totalNumberOfSuccessors[oldLineNumber] + totalNumberOfDataSuccessors[oldLineNumber]; i++){
						ac_int<8, false> successorName = successors[oldLineNumber][type][stage].slc<8>(successorNumberInStage<<3);
						ac_int<8, false> numberDep = (successorName == previousSuccessorName) ? previousNumberOfDep : numbersOfDependencies[successorName];
					numberDep = numberDep - 1;

					ac_int<1, false> isNull = numberDep == 0;
					ac_int<1, false> isStall = fifoNumberElement == 64;

					stall = isStall;

					previousSuccessorName = successorName;
					previousNumberOfDep = numberDep;

					//If the number of dependencies of the successor is now to 0, we add the instruction to the list
					//of ready instructions:

					ac_int<8, false> constant255 = 255;
					numbersOfDependencies[successorName] = (isNull & !isStall) ? constant255 : numberDep;

							if (isNull & !isStall){
							fifoInsertReadyInstruction[fifoPlaceToWrite] = successorName;
							fifoPlaceToWrite++;
							fifoNumberElement++;

						}

					//We update values to find next successor
					successorNumberInStage++;
					ac_int<1, false> stageOver = successorNumberInStage == successorNumbers[oldLineNumber][type][stage];
					if (stageOver){
							stage++;
						successorNumberInStage = 0;
						if (i+1 >= totalNumberOfSuccessors[oldLineNumber] && type == 0){
								type = 1;
							stage = 0;
							}
						}

					}
				totalNumberOfSuccessors[oldLineNumber] = 0;
				totalNumberOfDataSuccessors[oldLineNumber] = 0;
				successorStageNumber[oldLineNumber][0] = 0;
				successorStageNumber[oldLineNumber][1] = 0;

			}

	if (haveJump){
		if (jumpPlace < writeInBinaries-2){
			if (binaries[writeInBinaries-2].slc<32>(96) != 0 || binaries[writeInBinaries-1].slc<32>(64) != 0 || binaries[writeInBinaries-1].slc<32>(32) != 0){
				//binaries[writeInBinaries].set_slc(96, binaries[jumpPlace].slc<32>(96));
				writeInBinaries++;
				binaries[writeInBinaries] = 0;


				if (issue_width > 4){
					writeInBinaries++;
					binaries[writeInBinaries] = 0;

				}

			}
			binaries[writeInBinaries-2].set_slc(96,binaries[jumpPlace].slc<32>(96));

		}
		else{
			binaries[writeInBinaries] = 0;
			writeInBinaries++;

			if (issue_width > 4){
				binaries[writeInBinaries] = 0;
				writeInBinaries++;
			}
		}
	}

	return writeInBinaries-addressInBinaries;
}
#endif
#endif
