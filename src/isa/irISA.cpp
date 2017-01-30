/*
 * irISA.cpp
 *
 *  Created on: 24 nov. 2016
 *      Author: Simon Rokicki
 */



#include <lib/ac_int.h>
#include <types.h>
#include <isa/vexISA.h>
#include <isa/irISA.h>


/********************************************************************
 * Declaration functions to assemble uint128 instruction for IR
 * ******************************************************************/

ac_int<128, false> assembleRBytecodeInstruction(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<9, false> regB, ac_int<9, false> regDest,
		ac_int<8, false> nbDep){

	ac_int<128, false> result = 0;
	//Node: Type is zero: no need to write it for real. Same for isImm

	result.set_slc(96+30, stageCode);
	result.set_slc(96+27, isAlloc);
	result.set_slc(96+19, opcode);
	result.set_slc(96+0, regA);

	result.set_slc(64+23, regB);
	result.set_slc(64+14, regDest);
	result.set_slc(64+6, nbDep);

	return result;
}

ac_int<128, false> assembleRiBytecodeInstruction(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<13, false> imm13,
		ac_int<9, false> regDest, ac_int<8, false> nbDep){

	ac_int<128, false> result = 0;
	ac_int<1, false> isImm = 1;

	//Node: Type is zero: no need to write it for real.

	result.set_slc(96+30, stageCode);
	result.set_slc(96+27, isAlloc);
	result.set_slc(96+19, opcode);
	result.set_slc(96+18, isImm);
	result.set_slc(96+0, imm13);

	result.set_slc(64+23, regA);
	result.set_slc(64+14, regDest);
	result.set_slc(64+6, nbDep);

	return result;
}

ac_int<128, false> assembleIBytecodeInstruction(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> reg, ac_int<19, true> imm19, ac_int<8, false> nbDep){

	ac_int<128, false> result = 0;
	ac_int<2, false> typeCode = 2;
	ac_int<1, false> isImm = 1;

	result.set_slc(96+30, stageCode);
	result.set_slc(96+28, typeCode);
	result.set_slc(96+27, isAlloc);
	result.set_slc(96+19, opcode);
	result.set_slc(96+18, isImm);

	result.set_slc(64+23, imm19);
	result.set_slc(64+14, reg);
	result.set_slc(64+6, nbDep);

	return result;
}


/********************************************************************
 * Declaration of debug function
 * ******************************************************************/
void printBytecodeInstruction(int index, ac_int<128, false> oneInstruction){
	uint32 instructionPart1 = oneInstruction.slc<32>(96);
	uint32 instructionPart2 = oneInstruction.slc<32>(64);
	uint32 instructionPart3 = oneInstruction.slc<32>(32);
	uint32 instructionPart4 = oneInstruction.slc<32>(0);

	printBytecodeInstruction(index, instructionPart1, instructionPart2, instructionPart3, instructionPart4);

}

void printBytecodeInstruction(int index, uint32  instructionPart1, uint32  instructionPart2, uint32 instructionPart3, uint32 instructionPart4){

	ac_int<2, false> stageCode = instructionPart1.slc<2>(30);
	ac_int<2, false> typeCode = instructionPart1.slc<2>(28);
	ac_int<1, false> alloc = instructionPart1[27];
	ac_int<1, false> allocBr = instructionPart1[26];
	ac_int<7, false> opCode = instructionPart1.slc<7>(19);
	ac_int<1, false> isImm = instructionPart1[18];
	ac_int<1, false> isBr = instructionPart1[17];
	ac_int<9, false> virtualRDest = instructionPart2.slc<9>(14);
	ac_int<9, false> virtualRIn2 = instructionPart2.slc<9>(23);
	ac_int<9, false> virtualRIn1_imm9 = instructionPart1.slc<9>(0);
	ac_int<11, false> imm11 = instructionPart1.slc<11>(0);
	ac_int<19, false> imm19 = 0;
	imm19.set_slc(0, instructionPart2.slc<9>(23));
	imm19.set_slc(9, instructionPart1.slc<10>(0));
	ac_int<9, false> brCode = instructionPart1.slc<9>(9);

	ac_int<8, false> nbDep = instructionPart2.slc<8>(6);
	ac_int<3, false> nbDSucc = instructionPart2.slc<3>(3);
	ac_int<3, false> nbSucc = instructionPart2.slc<3>(0);

	fprintf(stderr, "%d : ", index);

	if (typeCode == 0){
		//R type
		fprintf(stderr, "%s r%d = r%d, ", opcodeNames[opCode], (int) virtualRDest, (int) virtualRIn2);
		if (isImm)
			fprintf(stderr, "%d ", (int) imm11);
		else
			fprintf(stderr, "r%d ", (int) virtualRIn1_imm9);


	}
	else if (typeCode == 1){
		//Rext Type
	}
	else {
		//I type
		fprintf(stderr, "%s r%d %d, ", opcodeNames[opCode], (int) virtualRDest, (int) imm19);

	}

	fprintf(stderr, "nbDep=%d, nbDSucc = %d, nbSucc = %d, ", (int) nbDep, (int) nbDSucc, (int) nbSucc);
	fprintf(stderr, "alloc=%d  successors:", (int) alloc);

	for (int oneSucc = 0; oneSucc < nbSucc; oneSucc++){
		int succ = 0;
		if (oneSucc >= 4)
			succ = (instructionPart3 >> (8*(oneSucc-4))) & 0xff;
		else
			succ = (instructionPart4 >> (8*(oneSucc))) & 0xff;


		fprintf(stderr, " %d", succ);
	}
	fprintf(stderr, "\n");

}

/********************************************************************
 * Declaration of a data structure to represent the control flow of the binaries analyzed.
 * ******************************************************************/

IRProcedure::IRProcedure(int startAddress, int endAddress, IRBlock* blocks, int nbBlock){
	this->blocks = blocks;
	this->vliwStartAddress = startAddress;
	this->vliwEndAddress = endAddress;
	this->nbBlock = nbBlock;
}


IRBlock::IRBlock(int startAddress, int endAddress){
	this->vliwEndAddress = endAddress;
	this->vliwStartAddress = startAddress;
}
