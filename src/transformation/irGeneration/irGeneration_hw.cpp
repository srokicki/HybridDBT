/*
 * irGeneration_hw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#ifndef __SW
#ifndef __HW

#ifndef __CATAPULT
//Includes not required by catapult
#include <cstdio>
#include <cstdlib>

#include <lib/endianness.h>
#include <dbt/dbtPlateform.h>
#endif

//Includes required by catapult
#include <isa/vexISA.h>
#include <isa/irISA.h>
#include <transformation/irGenerator.h>

#define FIRST_RENAME 0
#define LAST_RENAME 0

/* Global values */
ac_int<1, false> isOutsideNext = 0;
ac_int<1, false> droppedInstruction = 0;

ac_int<128, false> outsideNext_bytecode;
ac_int<9, false> outsideNext_pred1_reg;
ac_int<9, false> outsideNext_pred1;
ac_int<9, false> outsideNext_pred2_reg;
ac_int<10, true> outsideNext_dest_reg;
ac_int<16, true> outsideNext_imm;

ac_int<1, false> outsideNext_isImm;
ac_int<1, false> outsideNext_isLongImm;
ac_int<1, false> outsideNext_pred1_ena;
ac_int<1, false> outsideNext_pred1_solved;

ac_int<1, false> outsideNext_pred2_ena;
ac_int<1, false> outsideNext_dest_ena;
ac_int<1, false> outsideNext_dest_alloc;


ac_int<8, false> writeSucc_lastAddr = 255;
ac_int<128, false> writeSucc_lastValue = 0;




inline unsigned int writeSuccessor_ac(ac_int<128, false> bytecode[1024], ac_int<8, false> srcInstr, ac_int<8, false> destInstr, ac_int<1,false> isData, ac_int<128, false> *currentInstruction){


	ac_int<128, false> oneBytecodeInstruction = (writeSucc_lastAddr == srcInstr) ? writeSucc_lastValue : bytecode[srcInstr];
	ac_int<3, false> nbSucc = oneBytecodeInstruction.slc<3>(64);
	ac_int<3, false> nbDSucc = oneBytecodeInstruction.slc<3>(67);

	ac_int<8, false> offsetIsData = 6-nbDSucc;
	ac_int<8, false> offsetNotData = nbSucc - nbDSucc;

	ac_int<8, false> offset = isData ? offsetIsData : offsetNotData;
	offset = offset << 3;
	oneBytecodeInstruction.set_slc(offset, destInstr);

	nbSucc++;
	if (isData)
		nbDSucc++;



	oneBytecodeInstruction.set_slc(64, nbSucc);
	oneBytecodeInstruction.set_slc(67, nbDSucc);


	//We also increment the number of dependencies in the current instruction
	ac_int<8, false> nbDep = currentInstruction->slc<8>(64+6) + 1;

	if (srcInstr != destInstr){

		currentInstruction->set_slc(64+6, nbDep);

		bytecode[srcInstr] = oneBytecodeInstruction;
		writeSucc_lastAddr = srcInstr;
		writeSucc_lastValue = oneBytecodeInstruction;
	}
	return nbSucc;
}

inline unsigned int writePredecessor_ac(ac_int<128, false> bytecode[1024], ac_int<8, false> srcInstr, ac_int<8, false> destInstr, ac_int<1,false> isData, ac_int<128, false> *currentInstruction){
	//We load the bytecode word of the predecessor in order to increment the number of successor
	ac_int<128, false> oneBytecodeInstruction = (writeSucc_lastAddr == srcInstr) ? writeSucc_lastValue : bytecode[srcInstr];
	ac_int<8, false> nbDep = oneBytecodeInstruction.slc<8>(64+6);
	if (isData)
		nbDep++;
	oneBytecodeInstruction.set_slc(64+6, nbDep);

	//Then we get the nbSucc and nbDSucc of the current bytecode word in order to add the new dep
	//TODO: this should be named differently
	ac_int<3, false> nbSucc = currentInstruction->slc<3>(64);
	ac_int<3, false> nbDSucc = currentInstruction->slc<3>(67);

	ac_int<8, false> offsetIsData = 6-nbDSucc;
	ac_int<8, false> offsetNotData = nbSucc - nbDSucc;

	ac_int<8, false> offset = isData ? offsetIsData : offsetNotData;
	offset = offset << 3;


	nbSucc++;
	if (isData)
		nbDSucc++;

	if (srcInstr != destInstr){


		currentInstruction->set_slc(offset, srcInstr);
		currentInstruction->set_slc(64, nbSucc);
		currentInstruction->set_slc(67, nbDSucc);

		bytecode[srcInstr] = oneBytecodeInstruction;
		writeSucc_lastAddr = srcInstr;
		writeSucc_lastValue = oneBytecodeInstruction;
	}
	return nbSucc;

}


inline unsigned int writeDependency_ac(ac_int<128, false> bytecode[1024], ac_int<8, false> srcInstr, ac_int<8, false> destInstr, ac_int<1,false> isData, ac_int<128, false> *currentInstruction){

	//According to the value of IR_SUCC, the correct function will be called.
	//Note: the IR_SUCC should be defined in irISA.h

#ifdef IR_SUCC
	writeSuccessor_ac(bytecode, srcInstr, destInstr, isData, currentInstruction);
#else
	writePredecessor_ac(bytecode, srcInstr, destInstr, isData, currentInstruction);
#endif

}


unsigned int irGenerator_hw(ac_int<128, false> srcBinaries[1024], ac_int<32, false> addressInBinaries, ac_int<32, false> blockSize,
		ac_int<128, false> bytecode[1024], ac_int<32, true> globalVariables[128],
		ac_int<32, false> globalVariableCounter){

	#ifndef __CATAPULT
	//Performance simulation
	timeTakenIRGeneration = 0;
	#endif

	ac_int<1, false> const0 = 0;
	ac_int<1, false> const1 = 1;
	//**************************************************************************
	//Procedure header is placed at 16 + oneProcedure * 8
	//Its size is 8 bytes...
	// 1 byte for the number of global variables
	// 1 blank byte
	// 2 bytes for the number of basic block
	// 4 bytes pointing to the block table. The block table should start at 16 + procedureNumber*8

		writeSucc_lastAddr = 255;

		/* Basic Block metadata */
		int numberSuccessors = 0;
		ac_int<32, false> successor1, successor2;
		unsigned char indexInCurrentBlock = 0;
		ac_int<9, true> registers[128] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
				-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

		/* Datastructures for dag construction*/
		for (int i=0; i<64; i++)
			registers[i] = -1;

		ac_int<64, false> currentRegistresUsageWord = 0;

		/* Generated code */
		unsigned char numbersSuccessor[256];
		unsigned char numbersDataSuccessor[256];
		unsigned char successors[256][30];

		unsigned char numbersPredecessor[256];
		int predecessors[256][8];

		/* Datastructure for RAW dependencies on global registers */
		int lastWriterOnGlobal[128] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
		ac_int<2, false> lastReaderOnGlobalCounter[128] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		ac_int<2, false> lastReaderOnGlobalPlaceToWrite[128] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int lastReaderOnGlobal[128][4] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

		/* Datastructure for control dependencies on memories */
		int lastWriterOnMemory = -1;

		ac_int<1, false> lastWriterOnMemoryRegUnchanged = 0;
		ac_int<5, false> lastWriterOnMemoryReg = 0;
		ac_int<13, false> lastWriterOnMemoryImm = 0;

		ac_int<2, false> lastReaderOnMemoryCounter = 0;
		ac_int<2, false> lastReaderOnMemoryPlaceToWrite = 0;
		int lastReaderOnMemory[4] = {0,0,0,0};

		int isCallBlock = 0;
		ac_int<1, false> haveJump = 0;
		ac_int<8, false> jumpID = 0;
		ac_int<16, false> indexInSourceBinaries = 0;
		ac_int<128, false> previousVLIWSyllabus = 0;
		ac_int<2, false> instructionTranslatedFromPreviousSyllabus = 0;

		do {

			/********************************************************************
			 * First step is to fetch the instruction to translate
			 *
			 * For this, we load a complete 128-bits instruction word from the VLIW
			 * and consider each instruction not empty. If there is more than one, the
			 * second one is kept for next iteration
			 ********************************************************************/
			//FIXME handle the mov insertion
			ac_int<1, false> insertMove_ena = 0;
			droppedInstruction = 0;
			ac_int<9, true> insertMove_src;


			/* Note: How the instruction to translate is selected:
			 * We have possibly 7 sources for the instruction :
			 *
			 * -------------------------------------------------
			 * |instr3		|instr2	 |instr1   | instr0        |	cycle - 1
			 * -------------------------------------------------
			 * |instr3		|instr2	 |instr1   | instr0        |	cycle
			 * -------------------------------------------------
			 *
			 * The program will compute in order c-1/instr0, c-1/instr1, ..., c/instr0, ..., c/instr3
			 *
			 * We first compute the following bits:
			 *
			 * takeCm1_instr1 = instr1 != 0 & alreadyTranslated < 1
			 *
			 * each of these condition will be tested in order:
			 *
			 * c-1/instr3 is taken if takeCm1_instr3 && !takeCm1_instr2 && !takeCm2_instr1
			 *
			 */
			ac_int<128, false> oneVLIWSyllabus = srcBinaries[indexInSourceBinaries+addressInBinaries];
			ac_int<32, false> oneInstruction = 0;

			ac_int<1, false> takeCm1_instr1 = (previousVLIWSyllabus.slc<32>(64) != 0) & (instructionTranslatedFromPreviousSyllabus < 1);
			ac_int<1, false> takeCm1_instr2 = (previousVLIWSyllabus.slc<32>(32) != 0) & (instructionTranslatedFromPreviousSyllabus < 2);
			ac_int<1, false> takeCm1_instr3 = (previousVLIWSyllabus.slc<32>(0) != 0) & (instructionTranslatedFromPreviousSyllabus < 3);
			ac_int<1, false> takeC_instr0 = (oneVLIWSyllabus.slc<32>(96) != 0);
			ac_int<1, false> takeC_instr1 = (oneVLIWSyllabus.slc<32>(64) != 0);
			ac_int<1, false> takeC_instr2 = (oneVLIWSyllabus.slc<32>(32) != 0);
			ac_int<1, false> takeC_instr3 = (oneVLIWSyllabus.slc<32>(0) != 0);


			oneInstruction = takeCm1_instr1 ? previousVLIWSyllabus.slc<32>(64) :
					takeCm1_instr2 ? previousVLIWSyllabus.slc<32>(32) :
					takeCm1_instr3 ? previousVLIWSyllabus.slc<32>(0) :
					takeC_instr0 ? oneVLIWSyllabus.slc<32>(96) :
					takeC_instr1 ? oneVLIWSyllabus.slc<32>(64) :
					takeC_instr2 ? oneVLIWSyllabus.slc<32>(32) :
					oneVLIWSyllabus.slc<32>(0);

			instructionTranslatedFromPreviousSyllabus = takeCm1_instr1 ? 1 :
					takeCm1_instr2 ? 2 :
					takeCm1_instr3 ? 3 :
					takeC_instr0 ? 0 :
					takeC_instr1 ? 1 :
					takeC_instr2 ? 2 :
					3;

			previousVLIWSyllabus = (takeCm1_instr1 || takeCm1_instr2 || takeCm1_instr3) ? previousVLIWSyllabus : oneVLIWSyllabus;
			ac_int<16, false> tempIndex = indexInSourceBinaries + 1;
			indexInSourceBinaries = (takeCm1_instr1 || takeCm1_instr2 || takeCm1_instr3) ? indexInSourceBinaries : tempIndex;

			/********************************************************************
			 * Second step is to identify correct operand according to opcode
			 *
			 * For this we will decode the instruction and consider the opcode to
			 * select registers to read and to write. Instructions are separated into
			 * eight groups:
			 * -> Memory loads
			 * -> Memory writes
			 * -> Direct Branch
			 * -> Indirect/Conditional Branch (which needs to access a register)
			 * -> Movi (which only writes a register)
			 * -> Arith1 (arithmetical instruction with only one operand: sign extention or not)
			 * -> Arith2 (arithmetical instruction with two register operands)
			 * -> ArithImm (arithmetical instruction with one register operand and one immediate value
			 ********************************************************************/

			ac_int<7, false> opcode = oneInstruction.slc<7>(0);
			ac_int<13, true> imm13 = oneInstruction.slc<13>(7);
			ac_int<19, true> imm19 = oneInstruction.slc<19>(7);
			ac_int<6, false> reg8 = oneInstruction.slc<6>(8);
			ac_int<6, false> reg14 = oneInstruction.slc<6>(14);
			ac_int<6, false> reg20 = oneInstruction.slc<6>(20);
			ac_int<6, false> reg26 = oneInstruction.slc<6>(26);

			ac_int<5, false> funct = oneInstruction.slc<5>(7);
			ac_int<1, false> isIType = (opcode.slc<3>(4) == 2);

			ac_int<1, false> isNop = opcode == VEX_NOP | opcode == VEX_RECONFFS;
			ac_int<1, false> isLoadType = opcode == VEX_LDB | opcode == VEX_LDBU | opcode == VEX_LDH
					| opcode == VEX_LDHU | opcode == VEX_LDW | opcode == VEX_LDWU | opcode == VEX_LDD;
			ac_int<1, false> isStoreType = opcode == VEX_STB | opcode == VEX_STH | opcode == VEX_STW | opcode == VEX_STD;
			ac_int<1, false> isSpecMemType = opcode == VEX_SPEC_RST | opcode == VEX_SPEC_INIT;
			ac_int<1, false> isBranchWithNoReg = opcode == VEX_GOTO | opcode == VEX_CALL
					| opcode == VEX_STOP | opcode == VEX_ECALL;
			ac_int<1, false> isBranchWithReg = opcode == VEX_GOTOR | opcode == VEX_CALLR;
			ac_int<1, false> isBranchWithTwoReg = opcode == VEX_BR | opcode == VEX_BRF | opcode == VEX_BGE | opcode == VEX_BLT | opcode == VEX_BGEU | opcode == VEX_BLTU;
			ac_int<1, false> isMovi = opcode == VEX_MOVI;
			ac_int<1, false> isArith1 = opcode == VEX_NOT;
			ac_int<1, false> isArith2 = (opcode.slc<3>(4) == 4 | opcode.slc<3>(4) == 5) & !isArith1;
			ac_int<1, false> isArithImm = opcode.slc<3>(4) == 6 | opcode.slc<3>(4) == 7;
			ac_int<1, false> isMultType = opcode.slc<3>(4) == 0;
			ac_int<1, false> isProfile = opcode == VEX_PROFILE;
			ac_int<1, false> isFMADD = opcode == VEX_FMADD || opcode == VEX_FMSUB ||opcode == VEX_FNMADD ||opcode == VEX_FNMSUB;
			ac_int<1, false> isFSW = opcode == VEX_FSW || opcode == VEX_FSH || opcode == VEX_FSB;
			ac_int<1, false> isFLW = opcode == VEX_FLW || opcode == VEX_FLH || opcode == VEX_FLB;
			ac_int<1, false> isFP = opcode == VEX_FP;

			ac_int<1, false> isFloatRa = isFMADD || (isFP && funct != VEX_FP_FCVTSW && funct != VEX_FP_FCVTSWU && funct != VEX_FP_FMVWX);
			ac_int<1, false> isFloatRb = isFSW || isFMADD || isFP;
			ac_int<1, false> isFloatDest = isFLW || isFMADD || (isFP && funct != VEX_FP_FCVTWS && funct != VEX_FP_FCVTWUS && funct != VEX_FP_FEQ
					&& funct != VEX_FP_FLT && funct != VEX_FP_FLE && funct != VEX_FP_FCLASS && funct != VEX_FP_FMVXW);
			ac_int<1, false> enableRbFloat = isFSW || isFMADD || (isFP && funct != VEX_FP_FSQRT && funct != VEX_FP_FCVTWS && funct != VEX_FP_FCVTWUS
					&& funct != VEX_FP_FMVXW && funct != VEX_FP_FCLASS && funct != VEX_FP_FCVTSW && funct != VEX_FP_FCVTSWU && funct != VEX_FP_FMVWX);


			ac_int<1, false> pred1_ena = 0, pred2_ena = 0, dest_ena = 0;
			ac_int<7, false> pred1_reg = reg26, pred2_reg = reg20, dest_reg=0;

			//Solving accessed register 1
			if (!isBranchWithNoReg && !isMovi && !isProfile && !isSpecMemType)
				pred1_ena = 1;

			//Solving accessed register 2
			if (isStoreType || isArith2 || isMultType || isFSW || enableRbFloat || isBranchWithTwoReg)
				pred2_ena = 1;


			//Solving written register
			if (isArithImm || isArith1 || isLoadType || isFLW){
				dest_ena = 1;
				dest_reg = reg20;
			}
			else if (isMovi || opcode == VEX_CALL || opcode == VEX_CALLR){
				dest_ena = 1;
				dest_reg = reg26;
			}
			else if (isArith2 || isMultType || isFP){
				dest_ena = 1;
				dest_reg = reg14;
			}

			if (isNop){
				dest_ena = 0;
				pred2_ena = 0;
				pred1_ena = 0;
				droppedInstruction = 1;
			}

			if (isFloatRa)
				pred1_reg += 64;

			if (isFloatRb)
				pred2_reg += 64;

			if (isFloatDest)
				dest_reg += 64;



			numbersSuccessor[indexInCurrentBlock] = 0;
			numbersPredecessor[indexInCurrentBlock] = 0;

			/********************************************************************
			 * Third step is to solve/build dependencies in the current block
			 *
			 * This is the delicate step of the generation
			 * TODO detailled description of the process
			 ********************************************************************/


			ac_int<1, false> pred1_succ_ena = 0;
			ac_int<8, false> pred1_succ_src;
			ac_int<1, false> pred1_succ_isData = 0;

			ac_int<10, true> temp_pred1 = registers[pred1_reg];


			//We perform memory accesses for pred1
			ac_int<9, false> pred1;
			ac_int<1, false> pred1_global = 0;
			ac_int<9, false> pred1_global_address = pred1_reg;
			ac_int<10, true> pred1_global_access = globalVariables[pred1_reg];
			ac_int<2, false> lastReaderOnGlobalCounter_access_pred1 = lastReaderOnGlobalCounter[pred1_reg];
			ac_int<2, false> lastReaderOnGlobalPlaceToWrite_access_pred1 = lastReaderOnGlobalPlaceToWrite[pred1_reg];
			ac_int<2, false> lastReaderOnGlobalPlaceToWrite_access_pred1_old = lastReaderOnGlobalPlaceToWrite_access_pred1;
			ac_int<10, true> lastWriterOnGlobal_access_pred1 = lastWriterOnGlobal[pred1_reg];
			ac_int<10, true> lastReaderOnGlobal_value_pred1 = lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite_access_pred1];
			ac_int<9, false> pred1_global_value = 0;

			//We perform memory accesses for pred2
			ac_int<1, false> pred2_global = 0;
			ac_int<9, false> pred2_global_address = pred2_reg;
			ac_int<10, true> pred2_global_access = globalVariables[pred2_reg];
			ac_int<2, false> lastReaderOnGlobalCounter_access_pred2 = lastReaderOnGlobalCounter[pred2_reg];
			ac_int<2, false> lastReaderOnGlobalPlaceToWrite_access_pred2 = lastReaderOnGlobalPlaceToWrite[pred2_reg];
			ac_int<2, false> lastReaderOnGlobalPlaceToWrite_access_pred2_old = lastReaderOnGlobalPlaceToWrite_access_pred2;
			ac_int<10, true> lastWriterOnGlobal_access_pred2 = lastWriterOnGlobal[pred2_reg];
			ac_int<10, true> lastReaderOnGlobal_value_pred2 = lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite_access_pred2];
			ac_int<9, false> pred2_global_value = 0;

			//We perform memory accesses for dest
			ac_int<9, false> dest_global_address = dest_reg;
			ac_int<10, true> dest_global_access = globalVariables[dest_reg];
			ac_int<2, false> lastReaderOnGlobalCounter_access_dest = lastReaderOnGlobalCounter[dest_reg];
			ac_int<10, true> lastWriterOnGlobal_access_dest = lastWriterOnGlobal[dest_reg];
			ac_int<10, true> lastReaderOnGlobal_value_dest_1 = lastReaderOnGlobal[dest_reg][0];
			ac_int<10, true> lastReaderOnGlobal_value_dest_2 = lastReaderOnGlobal[dest_reg][1];
			ac_int<10, true> lastReaderOnGlobal_value_dest_3 = lastReaderOnGlobal[dest_reg][2];
			ac_int<2, false> lastReaderOnGlobalPlaceToWrite_access_dest = lastReaderOnGlobalPlaceToWrite[dest_reg];


			if (pred1_ena){

				if (temp_pred1 < 0){ //If value comes from global register
					temp_pred1 = pred1_global_access;
				}
				else{
					//We are facing a simple data dependency
					pred1_succ_ena = 1;
					pred1_succ_src = temp_pred1;
					pred1_succ_isData = 1;
				}

				pred1 = temp_pred1;
			}



			/************** Pred 2 ****************/
			ac_int<10, true> temp_pred2 = registers[pred2_reg];
			ac_int<9, false> pred2;


			ac_int<1, false> pred2_succ_ena = 0;
			ac_int<8, false> pred2_succ_src = 0;
			ac_int<1, false> pred2_succ_isData = 0;



			pred2_global_access = (pred2_global_address == pred1_global_address) ? pred1_global_access : pred2_global_access;


			if (pred2_ena){

				//To gather succ information on different path
				if (temp_pred2 < 0){ //If value comes from global register
					temp_pred2 = pred2_global_access;
				}
				else{
					//We are facing a simple data dependency
					pred2_succ_ena = 1;
					pred2_succ_src = temp_pred2;
					pred2_succ_isData = 1;
				}

				pred2 = temp_pred2;
			}


			if (isLoadType || isFLW){
				/****************************/
				/* We update lastReaderOneMemory and add required dependencies to keep memory coherence */
				ac_int<16, false> succ_src;
				if (lastReaderOnMemoryCounter < 3){
					lastReaderOnMemoryCounter++;
					if (lastWriterOnMemory != -1 && !(lastWriterOnMemoryRegUnchanged && lastWriterOnMemoryReg == pred1_reg && lastWriterOnMemoryImm != imm13)){
						succ_src = lastWriterOnMemory;
						pred2_succ_ena = 1;
					}
				}
				else{
					int readerToEvince = lastReaderOnMemory[lastReaderOnMemoryPlaceToWrite];
					succ_src = readerToEvince;
					pred2_succ_ena = 1;

				}

				pred2_succ_isData = 0;
				pred2_succ_src = succ_src;

				lastReaderOnMemory[lastReaderOnMemoryPlaceToWrite] = indexInCurrentBlock;
				lastReaderOnMemoryPlaceToWrite = (lastReaderOnMemoryPlaceToWrite + 1);
				if (lastReaderOnMemoryPlaceToWrite == 3)
					lastReaderOnMemoryPlaceToWrite = 0;
			}



			//******************************************
			//We set the destination

			ac_int<1, false> global_succ_ena_1 = 0;
			ac_int<1, false> global_succ_ena_2 = 0;
			ac_int<1, false> global_succ_ena_3 = 0;
			ac_int<1, false> global_succ_ena_4 = 0;

			ac_int<8, false> global_succ_src_1;
			ac_int<8, false> global_succ_src_2;
			ac_int<8, false> global_succ_src_3;
			ac_int<8, false> global_succ_src_4;


			ac_int<10, true> temp_destination = indexInCurrentBlock;
			ac_int<9, false> destination;



			ac_int<1, false> alloc = 1;



			//When reading global name, we make sure the global variable has not been named by pred1 or pred2
			dest_global_access = (dest_global_address == pred1_global_address) ? pred1_global_access : dest_global_access;
			dest_global_access = (dest_global_address == pred2_global_address) ? pred2_global_access : dest_global_access;

			if (dest_ena) {

				registers[dest_reg] = indexInCurrentBlock;

				if (dest_global_access < 0){

					registers[dest_reg] = indexInCurrentBlock;

					//We mark the value as a write which is potentially not read
					currentRegistresUsageWord[dest_reg] = 1;
				}
				else{
					alloc = 0;
					temp_destination = dest_global_access;

				}

			}
			else {
				alloc=0;
			}
			destination = temp_destination;

			if (isStoreType || isFSW || isSpecMemType){

				if (lastReaderOnMemoryCounter == 0 && lastWriterOnMemory != -1){
					global_succ_ena_1 = 1;
					global_succ_src_1 = lastWriterOnMemory;
				}

				if (lastReaderOnMemoryCounter >= 1){
					global_succ_ena_1 = 1;
					global_succ_src_1 = lastReaderOnMemory[0];
				}
				if (lastReaderOnMemoryCounter >= 2){
					global_succ_ena_2 = 1;
					global_succ_src_2 = lastReaderOnMemory[1];
				}
				if (lastReaderOnMemoryCounter >= 3){
					global_succ_ena_3 = 1;
					global_succ_src_3 = lastReaderOnMemory[2];
				}

				lastReaderOnMemoryCounter = 0;
				lastWriterOnMemory = indexInCurrentBlock;

				lastWriterOnMemoryReg = pred1_reg;
				lastWriterOnMemoryImm = imm13;
				lastWriterOnMemoryRegUnchanged = 1;

			}


			//We write back values in corresponding memories for pred1
			if (pred1_ena){
				lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite_access_pred1_old] = lastReaderOnGlobal_value_pred1;
				lastReaderOnGlobalCounter[pred1_reg] = lastReaderOnGlobalCounter_access_pred1;
				lastReaderOnGlobalPlaceToWrite[pred1_reg] = lastReaderOnGlobalPlaceToWrite_access_pred1;
				globalVariables[pred1_reg] = pred1_global_access;
			}

			//We write back values in corresponding memories for pred2
			if (pred2_ena){
				lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite_access_pred2_old] = lastReaderOnGlobal_value_pred2;
				lastReaderOnGlobalCounter[pred2_reg] = lastReaderOnGlobalCounter_access_pred2;
				lastReaderOnGlobalPlaceToWrite[pred2_reg] = lastReaderOnGlobalPlaceToWrite_access_pred2;
				globalVariables[pred2_reg] = pred2_global_access;
			}


			//We write back values in corresponding memories for dest
			if (dest_ena){
				lastWriterOnGlobal[dest_reg] = lastWriterOnGlobal_access_dest;
				lastReaderOnGlobalCounter[dest_reg] = lastReaderOnGlobalCounter_access_dest;
				lastReaderOnGlobalPlaceToWrite[dest_reg] = lastReaderOnGlobalPlaceToWrite_access_dest;
			}




			/********************************************************************
			 * Generation of the bytecode instruction
			 *****************************************
			 *
			 * Generating the bytecode instruction is quite simple: we only need to pick the correct encoding
			 * for the opcode used and to follow the pattern.
			 * This step rely on the assembleBytecodeInstructions() which allow to easily modify the way an instruction is
			 * encoded.
			 *
			 * The second step of the process is to add dependencies in the IR representation using the dedicated functions.
			 ********************************************************************/

			ac_int<128, false> oneBytecode = 0;
			if (insertMove_ena){
				//TODO

				#ifndef __CATAPULT
				printf("Implementation do not support mov insertion yet...\n Exiting...\n");
				exit(-1);
				#endif

			}
			else if (isBranchWithNoReg || isBranchWithReg){

				if (opcode == VEX_GOTO){
					numberSuccessors = 1;
					successor1 = imm19;
					pred1 = 0;
				}
				else if (opcode == VEX_BR || opcode == VEX_BRF){
					numberSuccessors = 2;
					successor1 = (imm13 + indexInSourceBinaries - 1) + 1; //FIXME
					successor2 = indexInSourceBinaries - 1 + 1;
				}
				else if (opcode == VEX_CALL){
					isCallBlock = 1;
					numberSuccessors = 1;
					successor1 = imm19;

					pred1 = destination;
				}

				oneBytecode = assembleIBytecodeInstruction_hw(0, 0, opcode, pred1, imm19, 0);

				haveJump = 1;
				jumpID = indexInCurrentBlock;

			}
			else if (isBranchWithTwoReg){

				oneBytecode = assembleRiBytecodeInstruction_hw(0, 0, opcode, pred1, imm13, pred2, 0);

				haveJump = 1;
				jumpID = indexInCurrentBlock;
			}
			else if (isMovi){
				oneBytecode = assembleIBytecodeInstruction_hw(2, alloc, opcode, destination, imm19, 0);
			}
			else if (isStoreType || isFSW){
				oneBytecode = assembleRiBytecodeInstruction_hw(1, 0, opcode, pred1, imm13, pred2, 0);
			}
			else if (isLoadType || isFLW){
				oneBytecode = assembleRiBytecodeInstruction_hw(1, alloc, opcode, pred1, imm13, destination, 0);
			}
			else if (isMultType){
				oneBytecode = assembleRBytecodeInstruction_hw(3, alloc, opcode, pred2, pred1, destination, 0);
			}
			else if (isArith1 || isArithImm){
				oneBytecode = assembleRiBytecodeInstruction_hw(2, alloc, opcode, pred1, imm13, destination, 0);
			}
			else if (isArith2){
				oneBytecode = assembleRBytecodeInstruction_hw(2, alloc, opcode, pred2, pred1, destination, 0);
			}
			else if (isProfile){
				oneBytecode = assembleRiBytecodeInstruction_hw(1, 0, opcode, 256, imm13, 256, 0);
			}
			else if (isFMADD){
				oneBytecode = assembleRiBytecodeInstruction_hw(1, 0, opcode, 256, imm13, 256, 0); //TODO
			}
			else if (isFP){
				oneBytecode = assembleFPBytecodeInstruction_hw(3, alloc, opcode, funct, pred2, pred1, destination, 0);
			}
			else if (isSpecMemType){
				oneBytecode = assembleRiBytecodeInstruction_hw(1, 0, opcode, 0, imm13, 0, 0);
			}
			else if (isNop){

			}
			else{
				#ifndef __CATAPULT
				printf("While generating IR, this case should never happen... %x\n", oneInstruction);
				std::cout << printDecodedInstr(oneInstruction);
				#endif
			}

			//*********************************
			//We add dependencies
			//Note: we never have more than 5 dependencies to add for one single instruction

			if (pred1_succ_ena){
				ac_int<3, false> nbSucc_pred1 = writeDependency_ac(bytecode, pred1_succ_src, indexInCurrentBlock, pred1_succ_isData, &oneBytecode);
				if (pred1_succ_isData & (nbSucc_pred1 == 7)){
						insertMove_ena = 1;
						insertMove_src = pred1;
						alloc = 1;
						destination = indexInCurrentBlock;
						dest_ena = 1;
				}
			}

			if (pred2_succ_ena){
				ac_int<3, false> nbSucc_pred2 = writeDependency_ac(bytecode, pred2_succ_src, indexInCurrentBlock, pred2_succ_isData,&oneBytecode);
				if (pred2_succ_isData & (nbSucc_pred2 == 7)){
						insertMove_ena = 1;
						insertMove_src = pred2;
						alloc = 1;
						destination = indexInCurrentBlock;
						dest_ena = 1;
				}
			}


			if (global_succ_ena_1)
				writeDependency_ac(bytecode, global_succ_src_1, indexInCurrentBlock, const0, &oneBytecode);


			if (global_succ_ena_2)
				writeDependency_ac(bytecode, global_succ_src_2, indexInCurrentBlock, const0, &oneBytecode);


			if (global_succ_ena_3)
				writeDependency_ac(bytecode, global_succ_src_3, indexInCurrentBlock, const0, &oneBytecode);






			/**************************************************************************
			 *  We place the instruction in memory
			 *************************************
			 *
			 * Last step is to place the instruction in the bytecode memory.
			 */

			if (!droppedInstruction)
			bytecode[indexInCurrentBlock] = oneBytecode;

			if (!insertMove_ena & !droppedInstruction)
				indexInCurrentBlock++;


			#ifndef __CATAPULT
			//Performance simulation
			timeTakenIRGeneration += 4;
			#endif
		}
		while (indexInSourceBinaries<=blockSize && indexInCurrentBlock<255);



		/*******************************************************************************
		 * Outside the main loop: adjustments on the block
		 * **********************************
		 *
		 * Here are executed some adjustments on the block structure: it can be for exemple the addition of
		 * dependencies from all nodes without successors to the jump instruction of the modification of all
		 * last writer to some renamed registers...
		 *
		 * At the moment, those two adjustments are some prototype and nothing is clearly defined in the flow...
		 */

		//Addint dependencies to the jump
		if (haveJump/* &&  bytecode[jumpID].slc<7>(96+19) == VEX_CALL*/){
			ac_int<128, false> jumpBytecodeWord = bytecode[jumpID];
			ac_int<8, false> numberDependencies = jumpBytecodeWord.slc<8>(64+6);

			#ifdef IR_SUCC

			for (int oneInstructionFromBlock = 0; oneInstructionFromBlock < indexInCurrentBlock; oneInstructionFromBlock++){

				ac_int<128, false> bytecodeWord = bytecode[oneInstructionFromBlock];
				ac_int<3, false> nbSucc = bytecodeWord.slc<3>(64);

				if (nbSucc == 0 && oneInstructionFromBlock != jumpID){
					writeDependency_ac(bytecode, oneInstructionFromBlock,jumpID, 0, &bytecode[jumpID]);
				}
			}

			#else

			ac_int<8, false> olderDependency[4];
			ac_int<3, false> nbOlderDependency = 0;
			ac_int<2, false> writeOlderDependency = 0;

			for (int oneInstructionFromBlock = 0; oneInstructionFromBlock < indexInCurrentBlock; oneInstructionFromBlock++){

				ac_int<128, false> bytecodeWord = bytecode[oneInstructionFromBlock];
				ac_int<8, false> nbSucc = bytecodeWord.slc<8>(64+6);

				if (nbSucc == 0 && oneInstructionFromBlock != jumpID){
					if (nbOlderDependency < 4){
						olderDependency[writeOlderDependency] = oneInstructionFromBlock;
						nbOlderDependency++;
						writeOlderDependency++;
					}
					else{
						writeDependency_ac(bytecode, olderDependency[writeOlderDependency],oneInstructionFromBlock, 0, &bytecode[oneInstructionFromBlock]);
						olderDependency[writeOlderDependency] = oneInstructionFromBlock;

						writeOlderDependency++;
					}
				}
				#ifndef __CATAPULT
				//Performance simulation
				timeTakenIRGeneration++;
				#endif
			}

			for (int oneOlder=0; oneOlder<nbOlderDependency; oneOlder++){
				writeOlderDependency--;
				writeDependency_ac(bytecode, olderDependency[writeOlderDependency],jumpID, 0, &bytecode[jumpID]);
			}
		#endif

		}

		//Modification of last writer on renamed registers
		for (ac_int<9, false> oneRegister=FIRST_RENAME; oneRegister<LAST_RENAME; oneRegister++){
			ac_int<9, false> lastWriter = registers[oneRegister];
			if (!lastWriter[8]){
				ac_int<9, false> newDestination = oneRegister+256;
				bytecode[lastWriter].set_slc(64+14, newDestination);
				bytecode[lastWriter][96+27] = 0;

				int depToAdd = 0;
				if (lastReaderOnGlobalCounter[oneRegister] >= 1 && lastReaderOnGlobal[oneRegister][0] != indexInCurrentBlock){
					writeDependency_ac(bytecode, lastReaderOnGlobal[oneRegister][0], lastWriter, const0, &bytecode[lastWriter]);
					depToAdd++;
				}

				if (lastReaderOnGlobalCounter[oneRegister] >= 2 && lastReaderOnGlobal[oneRegister][1] != indexInCurrentBlock){
					writeDependency_ac(bytecode, lastReaderOnGlobal[oneRegister][1], lastWriter, const0, &bytecode[lastWriter]);
					depToAdd++;
				}
				if (lastReaderOnGlobalCounter[oneRegister] >= 3 && lastReaderOnGlobal[oneRegister][2] != indexInCurrentBlock){
					writeDependency_ac(bytecode, lastReaderOnGlobal[oneRegister][2], lastWriter, const0, &bytecode[lastWriter]);
					depToAdd++;
				}
				ac_int<8, false> newDepNumber = bytecode[lastWriter].slc<8>(64+6);
			//	bytecode[lastWriter].set_slc(64+6, newDepNumber);
			}
		}

#ifndef __CATAPULT
//Performance simulation
timeTakenIRGeneration+=5;
#endif

unsigned int valueToReturn = indexInSourceBinaries;
valueToReturn = (valueToReturn<<16) + indexInCurrentBlock;
return valueToReturn;


}

#endif
#endif
