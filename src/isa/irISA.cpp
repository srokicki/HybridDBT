/*
 * irISA.cpp
 *
 *  Created on: 24 nov. 2016
 *      Author: Simon Rokicki
 */

#include <stdlib.h>
#include <stdio.h>

#include <types.h>
#include <isa/vexISA.h>
#include <isa/irISA.h>
#include <lib/endianness.h>

#ifndef __NIOS
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


void printBytecodeInstruction(int index, uint32  instructionPart1, uint32  instructionPart2, uint32 instructionPart3, uint32 instructionPart4){

	uint2 stageCode = ((instructionPart1>>30) & 0x3);
	uint2 typeCode = ((instructionPart1>>28) & 0x3);
	uint1 alloc = ((instructionPart1>>27) & 0x1);
	uint1 allocBr = ((instructionPart1>>26) & 0x1);
	uint7 opCode = ((instructionPart1>>19) & 0x7f);
	uint1 isImm = ((instructionPart1>>18) & 0x1);
	uint1 isBr = ((instructionPart1>>17) & 0x1);
	uint9 virtualRDest = ((instructionPart2>>14) & 0x1ff);
	uint9 virtualRIn2 = ((instructionPart2>>23) & 0x1ff);
	uint9 virtualRIn1_imm9 = ((instructionPart1>>0) & 0x1ff);
	ac_int<13, false> imm13 = ((instructionPart1>>0) & 0x1fff);

	uint11 imm11 = ((instructionPart1>>23) & 0x7ff);
	uint19 imm19 = 0;
	imm19 = ((instructionPart2>>23) & 0x1ff);
	imm19 += ((instructionPart1>>0) & 0x3ff)<<9;
	uint9 brCode = ((instructionPart1>>9) & 0x1ff);;

	uint8 nbDep = ((instructionPart2>>6) & 0xff);
	uint3 nbDSucc = ((instructionPart2>>3) & 7);
	uint3 nbSucc = ((instructionPart2>>0) & 7);

	fprintf(stderr, "%d : ", index);

	if (typeCode == 0){
		//R type
		fprintf(stderr, "%s r%d = r%d, ", opcodeNames[opCode], (int) virtualRDest, (int) virtualRIn2);
		if (isImm)
			fprintf(stderr, "%d ", (int) imm13);
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

	for (int oneSucc = 0; oneSucc < 7; oneSucc++){
		int succ = 0;
		if (oneSucc >= 4)
			succ = (instructionPart3 >> (8*(oneSucc-4))) & 0xff;
		else
			succ = (instructionPart4 >> (8*(oneSucc))) & 0xff;


		fprintf(stderr, " %d", succ);
	}
	fprintf(stderr, "\n");

}

#endif


/********************************************************************
 * Declaration of a data structure to represent the control flow of the binaries analyzed.
 * ******************************************************************/

IRProcedure::IRProcedure(IRBlock *entryBlock, int nbBlock){
	this->entryBlock = entryBlock;
	this->nbBlock = nbBlock;
}


void IRProcedure::print(){
	/********************************************************************************************
	 * This procedure is a debug procedure that will print a CDFG representation of the procedure
	 *
	 ********************************************************************************************/

	fprintf(stderr, "digraph{\n");
	for (int oneBlockInProcedure = 0; oneBlockInProcedure < this->nbBlock; oneBlockInProcedure++){
		fprintf(stderr, "node_%d[label=\"node %d - size %d\"];\n",this->blocks[oneBlockInProcedure]->vliwStartAddress,  this->blocks[oneBlockInProcedure]->vliwStartAddress, this->blocks[oneBlockInProcedure]->vliwEndAddress - this->blocks[oneBlockInProcedure]->vliwStartAddress);
	}
	for (int oneBlockInProcedure = 0; oneBlockInProcedure < this->nbBlock; oneBlockInProcedure++){

		if (this->blocks[oneBlockInProcedure]->nbSucc >= 1)
			fprintf(stderr, "node_%d -> node_%d;\n", this->blocks[oneBlockInProcedure]->vliwStartAddress, this->blocks[oneBlockInProcedure]->successor1->vliwStartAddress);
		if (this->blocks[oneBlockInProcedure]->nbSucc >= 2)
			fprintf(stderr, "node_%d -> node_%d;\n", this->blocks[oneBlockInProcedure]->vliwStartAddress, this->blocks[oneBlockInProcedure]->successor2->vliwStartAddress);

	}
	fprintf(stderr, "}\n");
}

IRBlock::IRBlock(int startAddress, int endAddress, int section){
	this->vliwEndAddress = endAddress;
	this->vliwStartAddress = startAddress;
	this->blockState = IRBLOCK_STATE_FIRSTPASS;
	this->nbSucc = -1;
	this->section = section;
	this->nbInstr = 0;
	this->jumpID=-1;
}

IRBlock::~IRBlock(){
	free(this->instructions);
}

IRApplication::IRApplication(int numberSections){
	this->numberOfSections = numberSections;
	this->blocksInSections = (IRBlock***) malloc(sizeof(IRBlock**) * numberOfSections);
	this->numbersBlockInSections= (int*) malloc(sizeof(int) * numberOfSections);

	this->numbersAllocatedBlockInSections = (int*) malloc(sizeof(int) * numberOfSections);
	for (int oneSection = 0; oneSection<numberSections; oneSection++){
		this->numbersBlockInSections[oneSection] = 0;
		this->numbersAllocatedBlockInSections[oneSection] = 0;
	}

	this->numberAllocatedProcedures = 0;
}

void IRApplication::addBlock(IRBlock* block, int sectionNumber){
	if (this->numbersAllocatedBlockInSections[sectionNumber] == this->numbersBlockInSections[sectionNumber]){
		//We allocate new blocks
		int numberBlocks = this->numbersBlockInSections[sectionNumber];
		int newAllocation = numberBlocks + 5;
		IRBlock** oldList = this->blocksInSections[sectionNumber];
		this->blocksInSections[sectionNumber] = (IRBlock**) malloc(newAllocation * sizeof(IRBlock*));
		memcpy(this->blocksInSections[sectionNumber], oldList, numberBlocks*sizeof(IRBlock*));
		this->numbersAllocatedBlockInSections[sectionNumber] = newAllocation;
		free(oldList);
	}


	this->blocksInSections[sectionNumber][this->numbersBlockInSections[sectionNumber]] = block;
	this->numbersBlockInSections[sectionNumber]++;
}

void IRApplication::addProcedure(IRProcedure *procedure){

	if (this->numberAllocatedProcedures == this->numberProcedures){
		//We allocate new procedures
		int numberProc = this->numberProcedures;
		int newAllocation = numberProc + 5;
		IRProcedure** oldList = this->procedures;
		this->procedures = (IRProcedure**) malloc(newAllocation * sizeof(IRProcedure*));
		memcpy(this->procedures, oldList, numberProc*sizeof(IRProcedure*));
		this->numberAllocatedProcedures = newAllocation;
		free(oldList);
	}


	this->procedures[this->numberProcedures] = procedure;
	this->numberProcedures++;
}

char getOpcode(uint32 *bytecode, char index){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1

	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);
	return (bytecodeWord96>>19) & 0x7f;
}

void setOpcode(uint32 *bytecode, char index, char newOpcode){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1

	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);
	unsigned int longOpcode = newOpcode<<19;
	bytecodeWord96 = (bytecodeWord96 & 0xfc07ffff) | longOpcode;
	writeInt(bytecode, index*16+0, bytecodeWord96);
}

short getDestinationRegister(uint32 *bytecode, char index){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1

	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	if ((opcode != 0) //not a nop
			&&((opcode>>4) != 2 || opcode == VEX_MOVI) //if I-type then movi
			&& ((opcode>>3) != 0x3)) //not a store
		return (bytecodeWord64>>14) & 0x1ff;

	return -1;
}

char getOperands(uint32 *bytecode, char index, short result[2]){
	//This function returns the number of register operand used by the bytecode instruction

	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	short virtualRDest = ((bytecodeWord64>>14) & 0x1ff);
	short virtualRIn2 = ((bytecodeWord64>>23) & 0x1ff);
	short virtualRIn1 = ((bytecodeWord96>>0) & 0x1ff);

	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	unsigned char shiftedOpcode = opcode>>4;

	char isNop = (opcode == 0);
	char isArith2 = (shiftedOpcode == 4 || shiftedOpcode == 5 || shiftedOpcode == 0 || shiftedOpcode == 3);
	char isLoad = (opcode>>3) == 0x2;
	char isStore = (opcode>>3) == 0x3;
	char isArith1 = (shiftedOpcode == 6 || shiftedOpcode == 7);
	char isBranchWithReg = (opcode == VEX_BR) || (opcode == VEX_BRF) ||(opcode == VEX_CALLR) ||(opcode == VEX_GOTOR);

	if (isNop)
		return 0;
	else if (isArith2){
		result[0] = virtualRIn1;
		result[1] = virtualRIn2;
		return 2;
	}
	else if (isStore){
		result[0] = virtualRIn2;
		result[1] = virtualRDest;
		return 2;
	}
	else if (isArith1 || isLoad){
		result[0] = virtualRIn2;
		return 1;
	}
	else if (isBranchWithReg){
		result[0] = virtualRDest;
		return 1;
	}
	else
		return 0;
}

void setOperands(uint32 *bytecode, char index, short operands[2]){
	//This function returns the number of register operand used by the bytecode instruction

	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	short virtualRDest = ((bytecodeWord64>>14) & 0x1ff);
	short virtualRIn2 = ((bytecodeWord64>>23) & 0x1ff);
	short virtualRIn1 = ((bytecodeWord96>>0) & 0x1ff);

	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	unsigned char shiftedOpcode = opcode>>4;

	char isNop = (opcode == 0);
	char isArith2 = (shiftedOpcode == 4 || shiftedOpcode == 5 || shiftedOpcode == 0 || shiftedOpcode == 3);
	char isLoad = (opcode>>3) == 0x2;
	char isStore = (opcode>>3) == 0x3;
	char isArith1 = (shiftedOpcode == 6 || shiftedOpcode == 7);
	char isBranchWithReg = (opcode == VEX_BR) || (opcode == VEX_BRF) ||(opcode == VEX_CALLR) ||(opcode == VEX_GOTOR);

	if (isNop){

	}
	else if (isArith2){
		bytecodeWord96 = (bytecodeWord96 & ~0x1ff) | (operands[0] & 0x1ff);//in1
		bytecodeWord64 = (bytecodeWord64 & ~(0x1ff<<23)) | ((operands[1] & 0x1ff)<<23);//in2
		writeInt(bytecode, index*16+4, bytecodeWord64);
		writeInt(bytecode, index*16+0, bytecodeWord96);
	}
	else if (isStore){
		bytecodeWord64 = (bytecodeWord64 & ~(0x1ff<<23)) | ((operands[0] & 0x1ff)<<23);//in2
		bytecodeWord64 = (bytecodeWord64 & ~(0x1ff<<14)) | ((operands[1] & 0x1ff)<<14); //dest
		writeInt(bytecode, index*16+4, bytecodeWord64);
	}
	else if (isArith1 || isLoad){
		bytecodeWord64 = (bytecodeWord64 & ~(0x1ff<<23)) | ((operands[0] & 0x1ff)<<23);//in2
		writeInt(bytecode, index*16+4, bytecodeWord64);
	}
	else if (isBranchWithReg){
		bytecodeWord64 = (bytecodeWord64 & ~(0x1ff<<14)) | ((operands[0] & 0x1ff)<<14); //dest
		writeInt(bytecode, index*16+4, bytecodeWord64);
	}
}

void setDestinationRegister(uint32 *bytecode, char index, short newDestinationRegister){
	fprintf(stderr, "Not implemented yet\n");
	exit(-1);
}

void setAlloc(uint32 *bytecode, char index, char newAlloc){
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	if (newAlloc)
		bytecodeWord96 |= 0x08000000;
	else
		bytecodeWord96 &= 0xf7ffffff;

	writeInt(bytecode, index*16+0, bytecodeWord96);
}

#ifdef IR_SUCC
void addDataDep(uint32 *bytecode, char index, char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, index*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, index*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);

	int extendedSuccessor = successor;
	if (nbSucc<7){
		bytecodeWord64 += 0x9; //plus one at both fields with no offset and fields with offset of 3
		if (nbDSucc<3){
			bytecodeWord32 |= (extendedSuccessor<<((2-nbDSucc)*8));
			writeInt(bytecode, index*16+8, bytecodeWord32);
		}
		else{
			bytecodeWord0 |= (extendedSuccessor<<((6-(nbDSucc))*8));
			writeInt(bytecode, index*16+12, bytecodeWord0);
		}
		writeInt(bytecode, index*16+4, bytecodeWord64);

	}
	//We add one to the number of dep
	unsigned int succBytecodeWord64 = readInt(bytecode, successor*16+4);
	succBytecodeWord64 += 0x40; //Plus one at the field located at an offset of 6 bits
	writeInt(bytecode, successor*16+4, succBytecodeWord64);
}

void addControlDep(uint32 *bytecode, char index, char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, index*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, index*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	int extendedSuccessor = successor;
	if (nbSucc<7){
		bytecodeWord64 += 0x1; //plus one at fields with no offset (eg nbSucc)
		if (nbCSucc>3){
			bytecodeWord32 |= (extendedSuccessor<<((nbCSucc-4)*8));
			writeInt(bytecode, index*16+8, bytecodeWord32);
		}
		else{
			bytecodeWord0 |= (extendedSuccessor<<((nbCSucc)*8));
			writeInt(bytecode, index*16+12, bytecodeWord0);
		}
		writeInt(bytecode, index*16+4, bytecodeWord64);

	}
	//We add one to the number of dep
	unsigned int succBytecodeWord64 = readInt(bytecode, successor*16+4);
	succBytecodeWord64 += 0x40; //Plus one at the field located at an offset of 6 bits
	writeInt(bytecode, successor*16+4, succBytecodeWord64);
}
#else

void addDataDep(uint32 *bytecode, char index, char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, successor*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, successor*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, successor*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);

	int extendedDepName = index;
	if (nbSucc<7){
		bytecodeWord64 += 0x9; //plus one at both fields with no offset and fields with offset of 3
		if (nbDSucc<3){
			bytecodeWord32 |= (extendedDepName<<((2-nbDSucc)*8));
			writeInt(bytecode, successor*16+8, bytecodeWord32);
		}
		else{
			bytecodeWord0 |= (extendedDepName<<((6-(nbDSucc))*8));
			writeInt(bytecode, successor*16+12, bytecodeWord0);
		}
		writeInt(bytecode, successor*16+4, bytecodeWord64);

	}
	//We add one to the number of dep
	unsigned int otherBytecodeWord64 = readInt(bytecode, index*16+4);
	otherBytecodeWord64 += 0x40; //Plus one at the field located at an offset of 6 bits
	writeInt(bytecode, index*16+4, otherBytecodeWord64);
}

void addControlDep(uint32 *bytecode, char index, char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, successor*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, successor*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, successor*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	int extendedDepName = index;
	if (nbSucc<7){
		bytecodeWord64 += 0x1; //plus one at fields with no offset (eg nbSucc)
		if (nbCSucc>3){
			bytecodeWord32 |= (extendedDepName<<((nbCSucc-4)*8));
			writeInt(bytecode, successor*16+8, bytecodeWord32);
		}
		else{
			bytecodeWord0 |= (extendedDepName<<((nbCSucc)*8));
			writeInt(bytecode, successor*16+12, bytecodeWord0);
		}
		writeInt(bytecode, successor*16+4, bytecodeWord64);

	}
	//We do not need to increment the number of dep...
}

#endif

void addOffsetToDep(uint32 *bytecode, char index, char offset){
	unsigned int bytecodeWord0 = readInt(bytecode, index*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, index*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	for (int oneDep = 0; oneDep < nbDSucc; oneDep++){
		char oldDep = readChar(bytecode, index*16+9+oneDep);
		writeChar(bytecode, index*16+9+oneDep, oldDep+offset);
	}

	for (int oneDep = 6; oneDep > 6-nbCSucc; oneDep--){
		char oldDep = readChar(bytecode, index*16+9+oneDep);
		writeChar(bytecode, index*16+9+oneDep, oldDep+offset);
	}
}

