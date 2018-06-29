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
#include <sstream>
#include <cstring>
#include <lib/log.h>

#ifndef __NIOS
/********************************************************************
 * Declaration functions to assemble uint128 instruction for IR
 * ******************************************************************/

struct uint128_struct assembleRBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short regA, short regB, short regDest,
		unsigned char nbDep){


	struct uint128_struct result = {0, 0, 0, 0};


	result.word96 += ((stageCode & 0x3) << 30);
	result.word96 += ((isAlloc & 0x1) << 27);
	result.word96 += ((opcode & 0x7f) << 19);
	result.word96 += ((regA & 0x1ff) << 0);


	result.word64 += ((regB & 0x1ff) << 23);
	result.word64 += ((regDest & 0x1ff) << 14);
	result.word64 += ((nbDep & 0xff) << 6);

	return result;
}

struct uint128_struct assembleFPBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, char funct, short regA, short regB, short regDest,
		unsigned char nbDep){

	struct uint128_struct result = {0, 0, 0, 0};


	result.word96 += ((stageCode & 0x3) << 30);
	result.word96 += ((isAlloc & 0x1) << 27);
	result.word96 += ((opcode & 0x7f) << 19);
	result.word96 += ((funct & 0x1f) << 13);
	result.word96 += ((regA & 0x1ff) << 0);


	result.word64 += ((regB & 0x1ff) << 23);
	result.word64 += ((regDest & 0x1ff) << 14);
	result.word64 += ((nbDep & 0xff) << 6);

	return result;
}

struct uint128_struct assembleRiBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short regA, short imm13,
		short regDest, unsigned char nbDep){

	struct uint128_struct result = {0, 0, 0, 0};
	char isImm = 1;

	//Node: Type is zero: no need to write it for real.

	result.word96 += ((stageCode & 0x3) << 30);
	result.word96 += ((isAlloc & 0x1) << 27);
	result.word96 += ((opcode & 0x7f) << 19);
	result.word96 += ((isImm & 0x1) << 18);
	result.word96 += ((imm13 & 0x1fff) << 0);


	result.word64 += ((regA & 0x1ff) << 23);
	result.word64 += ((regDest & 0x1ff) << 14);
	result.word64 += ((nbDep & 0xff) << 6);

	return result;
}

struct uint128_struct assembleMemoryBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short regA, short imm12, bool isSpec, char specId,
		short regDest, unsigned char nbDep){

	struct uint128_struct result = {0, 0, 0, 0};
	char isImm = 1;

	//Node: Type is zero: no need to write it for real.

	result.word96 += ((stageCode & 0x3) << 30);
	result.word96 += ((isAlloc & 0x1) << 27);
	result.word96 += ((opcode & 0x7f) << 19);
	result.word96 += ((isImm & 0x1) << 18);

	if (isSpec){
		result.word96 += ((specId & 0x1f) << 1);
		result.word96 += ((imm12 & 0x07f) << 6);
	}
	else{
		result.word96 += ((imm12 & 0x0fff) << 1);
	}
	result.word96 += (((isSpec ? 1 : 0) & 0x1) << 0);


	result.word64 += ((regA & 0x1ff) << 23);
	result.word64 += ((regDest & 0x1ff) << 14);
	result.word64 += ((nbDep & 0xff) << 6);

	return result;
}

struct uint128_struct assembleIBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short reg, int imm19, unsigned char nbDep){

	struct uint128_struct result = {0, 0, 0, 0};
	char typeCode = 2;
	char isImm = 1;


	result.word96 += ((stageCode & 0x3) << 30);
	result.word96 += ((typeCode & 0x3) << 28);
	result.word96 += ((isAlloc & 0x1) << 27);
	result.word96 += ((opcode & 0x7f) << 19);
	result.word96 += ((isImm & 0x1) << 18);
	result.word96 += ((imm19 >> 9) & 0x3ff);

	result.word64 += ((imm19 & 0x1ff) << 23);
	result.word64 += ((reg & 0x1ff) << 14);
	result.word64 += ((nbDep & 0xff) << 6);

	return result;
}



#ifndef __SW
#ifndef __HW

ac_int<128, false> assembleRBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<9, false> regB, ac_int<9, false> regDest,
		ac_int<8, false> nbDep){

	ac_int<128, false> fakeResult = 0;
	//Node: Type is zero: no need to write it for real. Same for isImm

	fakeResult.set_slc(96+30, stageCode);
	fakeResult.set_slc(96+27, isAlloc);
	fakeResult.set_slc(96+19, opcode);
	fakeResult.set_slc(96+0, regA);

	fakeResult.set_slc(64+23, regB);
	fakeResult.set_slc(64+14, regDest);
	fakeResult.set_slc(64+6, nbDep);

	return fakeResult;
}

ac_int<128, false> assembleFPBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<5, false> funct, ac_int<9, false> regA, ac_int<9, false> regB, ac_int<9, false> regDest,
		ac_int<8, false> nbDep){

	ac_int<128, false> fakeResult = 0;
	//Node: Type is zero: no need to write it for real. Same for isImm

	fakeResult.set_slc(96+30, stageCode);
	fakeResult.set_slc(96+27, isAlloc);
	fakeResult.set_slc(96+19, opcode);
	fakeResult.set_slc(96+13, funct);
	fakeResult.set_slc(96+0, regA);

	fakeResult.set_slc(64+23, regB);
	fakeResult.set_slc(64+14, regDest);
	fakeResult.set_slc(64+6, nbDep);

	return fakeResult;
}

ac_int<128, false> assembleRiBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<13, false> imm13,
		ac_int<9, false> regDest, ac_int<8, false> nbDep){


	ac_int<128, false> fakeResult = 0;
	ac_int<1, false> isImm = 1;

	fakeResult.set_slc(96+30, stageCode);
	fakeResult.set_slc(96+27, isAlloc);
	fakeResult.set_slc(96+19, opcode);
	fakeResult.set_slc(96+18, isImm);
	fakeResult.set_slc(96+0, imm13);

	fakeResult.set_slc(64+23, regA);
	fakeResult.set_slc(64+14, regDest);
	fakeResult.set_slc(64+6, nbDep);

	return fakeResult;
}

ac_int<128, false> assembleIBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> reg, ac_int<19, true> imm19, ac_int<8, false> nbDep){

	ac_int<2, false> typeCode = 2;
	ac_int<1, false> isImm = 1;


	ac_int<128, false> fakeResult = 0;
	fakeResult.set_slc(96+30, stageCode);
	fakeResult.set_slc(96+28, typeCode);
	fakeResult.set_slc(96+27, isAlloc);
	fakeResult.set_slc(96+19, opcode);
	fakeResult.set_slc(96+18, isImm);

	fakeResult.set_slc(64+23, imm19);
	fakeResult.set_slc(64+14, reg);
	fakeResult.set_slc(64+6, nbDep);

	return fakeResult;
}

#endif
#endif

/********************************************************************
 * Declaration of debug function
 * ******************************************************************/


std::string printBytecodeInstruction(int index, unsigned int  instructionPart1, unsigned int  instructionPart2, unsigned int instructionPart3, unsigned int instructionPart4){

	int stageCode = ((instructionPart1>>30) & 0x3);
	int typeCode = ((instructionPart1>>28) & 0x3);
	int alloc = ((instructionPart1>>27) & 0x1);
	int allocBr = ((instructionPart1>>26) & 0x1);
	int opCode = ((instructionPart1>>19) & 0x7f);
	int isImm = ((instructionPart1>>18) & 0x1);
	int isBr = ((instructionPart1>>17) & 0x1);
	short virtualRDest = ((instructionPart2>>14) & 0x1ff);
	short virtualRIn2 = ((instructionPart2>>23) & 0x1ff);
	short virtualRIn1_imm9 = ((instructionPart1>>0) & 0x1ff);
	short imm13 = ((instructionPart1>>0) & 0x1fff);
	int funct = (instructionPart1 >> 7) & 0x1f;

	short imm11 = ((instructionPart1>>23) & 0x7ff);
	short imm19 = 0;
	imm19 = ((instructionPart2>>23) & 0x1ff);
	imm19 += ((instructionPart1>>0) & 0x3ff)<<9;
	short brCode = ((instructionPart1>>9) & 0x1ff);;

	int nbDep = ((instructionPart2>>6) & 0xff);
	int nbDSucc = ((instructionPart2>>3) & 7);
	int nbSucc = ((instructionPart2>>0) & 7);

	std::stringstream result;

	result << index << " : ";

	if (typeCode == 0){
		//R type
		result << opcodeNames[opCode];
		if (opCode == VEX_FP){
				result << " " << fpNames[funct];
		}

		result << " r" << virtualRDest << " = r" << virtualRIn2 << ", ";

		if (opCode == VEX_FMADD || opCode == VEX_FMSUB ||opCode == VEX_FNMADD ||opCode == VEX_FNMSUB){
			result << "r" << virtualRIn1_imm9 << " ";

		}

		if (isImm)
			result << imm13 << " ";
		else
			result << "r" << virtualRIn1_imm9 << " ";

	}
	else if (typeCode == 1){
		//Rext Type
	}
	else {
		//I type
		result << opcodeNames[opCode] << " r" << virtualRDest << " " << imm19 << ", ";
	}


#ifdef IR_SUCC
	result << "nbDep=" << nbDep << ", nbDSucc=" << nbDSucc << ", nbSucc = " << nbSucc;
#else
	result << "nbSucc=" << nbDep << ", nbDPred=" << nbDSucc << ", nbPred = " << nbSucc;
#endif

	result << " alloc=" << alloc << "  successors:";

	for (int oneSucc = 0; oneSucc < 7; oneSucc++){
		int succ = 0;
		if (oneSucc >= 4)
			succ = (instructionPart3 >> (8*(oneSucc-4))) & 0xff;
		else
			succ = (instructionPart4 >> (8*(oneSucc))) & 0xff;


		result << " " << succ;
	}
	result << "\n";

	std::string returnedRes(result.str());
	return returnedRes;

}

#endif


/********************************************************************
 * Declaration of a data structure to represent the control flow of the binaries analyzed.
 * ******************************************************************/

IRProcedure::IRProcedure(IRBlock *entryBlock, int nbBlock){
	this->entryBlock = entryBlock;
	this->nbBlock = nbBlock;

	for (int oneConfiguration = 0; oneConfiguration < 32; oneConfiguration++){
		configurationScores[oneConfiguration] = 0;
	}
	this->state = 0;
}


void IRProcedure::print(FILE * output){
	/********************************************************************************************
	 * This procedure is a debug procedure that will print a CDFG representation of the procedure
	 *
	 ********************************************************************************************/

	fprintf(output, "digraph{\n");
	for (int oneBlockInProcedure = 0; oneBlockInProcedure < this->nbBlock; oneBlockInProcedure++){
		fprintf(output, "node_%d[label=\"node %d - size %d  - nbJump %d place %lx\"];\n",this->blocks[oneBlockInProcedure]->sourceStartAddress,  this->blocks[oneBlockInProcedure]->vliwStartAddress, this->blocks[oneBlockInProcedure]->nbInstr, this->blocks[oneBlockInProcedure]->nbJumps, this->blocks[oneBlockInProcedure]);
	}
	for (int oneBlockInProcedure = 0; oneBlockInProcedure < this->nbBlock; oneBlockInProcedure++){

		for (int oneSuccessor = 0; oneSuccessor<this->blocks[oneBlockInProcedure]->nbSucc; oneSuccessor++){
			fprintf(output, "node_%d -> node_%d;\n", this->blocks[oneBlockInProcedure]->sourceStartAddress, this->blocks[oneBlockInProcedure]->successors[oneSuccessor]->sourceStartAddress);
		}

	}
	fprintf(output, "}\n");

	for (int oneBlockInProcedure = 0; oneBlockInProcedure < this->nbBlock-1; oneBlockInProcedure++){
		if (this->blocks[oneBlockInProcedure]->sourceEndAddress != this->blocks[oneBlockInProcedure+1]->sourceStartAddress){
			fprintf(output, "test block 1 %d to %d and block 2 is %d to %d\n", this->blocks[oneBlockInProcedure]->sourceStartAddress, this->blocks[oneBlockInProcedure]->sourceEndAddress, this->blocks[oneBlockInProcedure+1]->sourceStartAddress, this->blocks[oneBlockInProcedure+1]->sourceEndAddress);
		}
	}
}

void IRBlock::addJump(unsigned char jumpID, unsigned int jumpPlace){
	unsigned char* tempJumpIds = (unsigned char*) malloc(sizeof(unsigned char) * (this->nbJumps+1));
	unsigned int* tempJumpPlaces = (unsigned int*) malloc(sizeof(unsigned int) * (this->nbJumps+1));

	memcpy(tempJumpIds, this->jumpIds, sizeof(unsigned char) * this->nbJumps);
	memcpy(tempJumpPlaces, this->jumpPlaces, sizeof(unsigned int) * this->nbJumps);

	tempJumpIds[this->nbJumps] = jumpID;
	tempJumpPlaces[this->nbJumps] = jumpPlace;

	if (this->nbJumps>0){
		free(this->jumpIds);
		free(this->jumpPlaces);
	}


	this->nbJumps++;
	this->jumpIds = tempJumpIds;
	this->jumpPlaces = tempJumpPlaces;


}

IRBlock::IRBlock(int startAddress, int endAddress, int section){
	this->vliwEndAddress = endAddress;
	this->vliwStartAddress = startAddress;
	this->blockState = IRBLOCK_STATE_FIRSTPASS;
	this->nbSucc = -1;
	this->section = section;
	this->nbInstr = 0;
	this->jumpID=-1;
	this->nbJumps = 0;
	this->placeInProfiler = NULL;
	this->instructions = NULL;

}

IRBlock::~IRBlock(){
	if (placeInProfiler != 0)
		*placeInProfiler = 0;
	if (!this->instructions)
		free(this->instructions);
	if (this->nbJumps > 0){
		free(this->jumpIds);
		free(this->jumpPlaces);
	}
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
	this->numberProcedures = 0;
  this->procedures = nullptr;

}

IRApplication::~IRApplication(){


	for (int oneSection = 0; oneSection<this->numberOfSections; oneSection++){
		for (int oneBlock = 0; oneBlock<this->numbersBlockInSections[oneSection]; oneBlock++){
			if (this->blocksInSections[oneSection][oneBlock] != NULL)
				delete this->blocksInSections[oneSection][oneBlock];
		}
	}


	for (int oneProcedure = 0; oneProcedure<this->numberProcedures; oneProcedure++){
		delete this->procedures[oneProcedure];
	}
}

void IRApplication::addBlock(IRBlock* block, int sectionNumber){
	if (sectionNumber>this->numberOfSections){
		Log::printf(LOG_ERROR,"Error while adding a block in an application: section %d is higher than the total number of section (%d)\n", sectionNumber, this->numberOfSections);
		exit(-1);
	}

	if (this->numbersAllocatedBlockInSections[sectionNumber] == this->numbersBlockInSections[sectionNumber]){
		//We allocate new blocks
		int numberBlocks = this->numbersBlockInSections[sectionNumber];
		int newAllocation = numberBlocks + 5;
		IRBlock** oldList = this->blocksInSections[sectionNumber];
		this->blocksInSections[sectionNumber] = (IRBlock**) malloc(newAllocation * sizeof(IRBlock*));
		memcpy(this->blocksInSections[sectionNumber], oldList, numberBlocks*sizeof(IRBlock*));
		this->numbersAllocatedBlockInSections[sectionNumber] = newAllocation;
		for (int oneBlock = 0; oneBlock<numberBlocks; oneBlock++){
			this->blocksInSections[sectionNumber][oneBlock]->reference = &(this->blocksInSections[sectionNumber][oneBlock]);
		}

		free(oldList);
	}


	this->blocksInSections[sectionNumber][this->numbersBlockInSections[sectionNumber]] = block;
	block->reference = &(this->blocksInSections[sectionNumber][this->numbersBlockInSections[sectionNumber]]);
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

    if (oldList)
      free(oldList);
	}


	this->procedures[this->numberProcedures] = procedure;
	this->numberProcedures++;
}

char getOpcode(unsigned int *bytecode, unsigned char index){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1

	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);
	return (bytecodeWord96>>19) & 0x7f;
}

void setOpcode(unsigned int *bytecode, unsigned char index, char newOpcode){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1

	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);
	unsigned int longOpcode = newOpcode<<19;
	bytecodeWord96 = (bytecodeWord96 & 0xfc07ffff) | longOpcode;
	writeInt(bytecode, index*16+0, bytecodeWord96);
}

short getDestinationRegister(unsigned int *bytecode, unsigned char index){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1


	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	if ((opcode != 0) //not a nop
			&&((opcode>>4) != 2 || opcode == VEX_MOVI) //if I-type then movi
			&& ((opcode>>3) != 0x3) //not a store
		    && opcode != VEX_FSB && opcode != VEX_FSH && opcode != VEX_FSW) //not a FP store
		return (bytecodeWord64>>14) & 0x1ff;

	return -1;
}

char getOperands(unsigned int *bytecode, unsigned char index, short result[2]){
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
	char isBranchWithReg = (opcode == VEX_CALLR) ||(opcode == VEX_GOTOR);
	char isBranchWithTwoRegs = (opcode == VEX_BR) || (opcode == VEX_BRF) || (opcode == VEX_BGE) || (opcode == VEX_BLT) || (opcode == VEX_BGEU) || (opcode == VEX_BLTU);

	if (isNop)
		return 0;
	else if (isArith2){
		result[0] = virtualRIn1;
		result[1] = virtualRIn2;
		return 2;
	}
	else if (isStore || isBranchWithTwoRegs){
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


void setImmediateValue(unsigned int *bytecode, unsigned char index, int value){
	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	unsigned char shiftedOpcode = opcode>>4;

	bool isMemType = (opcode>>4) == 1 || opcode == VEX_FLW || opcode == VEX_FSW;
	bool isImmArith = (opcode>>4) >= 6;
	bool isIType = (opcode>>4) == 2 && (opcode != VEX_BLT) && (opcode != VEX_BGE) && (opcode != VEX_BLTU) && (opcode != VEX_BGEU) && (opcode != VEX_BR) && (opcode != VEX_BRF);
	bool isBranchWithTwoRegs = (opcode == VEX_BR) || (opcode == VEX_BRF) || (opcode == VEX_BGE) || (opcode == VEX_BLT) || (opcode == VEX_BGEU) || (opcode == VEX_BLTU);



	// we clear immediate values
	if (isMemType){
		bytecodeWord96 &= 0xffffe001;
		bytecodeWord96 |= (value<<1) & 0x1fff;
	}
	else if (isBranchWithTwoRegs || isImmArith){
		bytecodeWord96 &= 0xffffe000;
		bytecodeWord96 |= value & 0x1fff;
	}
	else if (isIType){
		bytecodeWord96 &= 0xfffffc00;
		bytecodeWord64 &= 0x007fffff;

		bytecodeWord96 |= (value>>9) & 0x3ff;
		bytecodeWord64 |= (value & 0x1ff)<<23;
	}
	writeInt(bytecode, index*16+4,bytecodeWord64);
	writeInt(bytecode, index*16+0, bytecodeWord96);
}

bool getImmediateValue(unsigned int *bytecode, unsigned char index, int *result){
	//This function returns the number of register operand used by the bytecode instruction

	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	int imm13 = ((bytecodeWord96>>0) & 0x1fff);
	int imm19 = 0;
	imm19 = ((bytecodeWord64>>23) & 0x1ff);
	imm19 += ((bytecodeWord96>>0) & 0x3ff)<<9;

	if (imm13 >= 4096)
		imm13 -= 8192;

	if (imm19 >= 262144)
		imm19 -= 524288;

	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	unsigned char shiftedOpcode = opcode>>4;

	bool isMemType = (opcode>>4) == 1 || opcode == VEX_FLW || opcode == VEX_FSW;
	bool isImmArith = (opcode>>4) >= 6;
	bool isIType = (opcode>>4) == 2 && (opcode != VEX_BLT) && (opcode != VEX_BGE) && (opcode != VEX_BLTU) && (opcode != VEX_BGEU) && (opcode != VEX_BR) && (opcode != VEX_BRF);
	bool isBranchWithTwoRegs = (opcode == VEX_BR) || (opcode == VEX_BRF) || (opcode == VEX_BGE) || (opcode == VEX_BLT) || (opcode == VEX_BGEU) || (opcode == VEX_BLTU);

	if (isMemType){
		*result = (imm13>>1);
		return true;
	}
	else if (isBranchWithTwoRegs || isImmArith){
		*result = imm13;
		return true;
	}
	else if (isIType){
		*result = imm19;
		return true;
	}
	else{
		return false;
	}
}

void setOperands(unsigned int *bytecode, unsigned char index, short operands[2]){
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
	char isBranchWithReg = (opcode == VEX_CALLR) ||(opcode == VEX_GOTOR);
	char isBranchWithTwoRegs = (opcode == VEX_BR) || (opcode == VEX_BRF) || (opcode == VEX_BGE) || (opcode == VEX_BLT) || (opcode == VEX_BGEU) || (opcode == VEX_BLTU);

	if (isNop){

	}
	else if (isArith2){
		bytecodeWord96 = (bytecodeWord96 & ~0x1ff) | (operands[0] & 0x1ff);//in1
		bytecodeWord64 = (bytecodeWord64 & ~(0x1ff<<23)) | ((operands[1] & 0x1ff)<<23);//in2
		writeInt(bytecode, index*16+4, bytecodeWord64);
		writeInt(bytecode, index*16+0, bytecodeWord96);
	}
	else if (isStore || isBranchWithTwoRegs){
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

void setDestinationRegister(unsigned int *bytecode, unsigned char index, short newDestinationRegister){
	//This function returns the destination register of a bytecode instruction
	//If bytecode instruction do not write any register then it returns -1


	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);
	unsigned int newDestinationRegisterExtended = newDestinationRegister;
	unsigned char opcode = (bytecodeWord96>>19) & 0x7f;
	if ((opcode != 0) //not a nop
			&&((opcode>>4) != 2 || opcode == VEX_MOVI) //if I-type then movi
			&& ((opcode>>3) != 0x3) //not a store
		    && opcode != VEX_FSB && opcode != VEX_FSH && opcode != VEX_FSW) //not a FP store
		bytecodeWord64 = (bytecodeWord64 & 0xff803fff) | (newDestinationRegisterExtended << 14);

	writeInt(bytecode, index*16+4, bytecodeWord64);
}

void setAlloc(unsigned int *bytecode, unsigned char index, char newAlloc){
	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);

	if (newAlloc)
		bytecodeWord96 |= 0x08000000;
	else
		bytecodeWord96 &= 0xf7ffffff;

	writeInt(bytecode, index*16+0, bytecodeWord96);
}

#ifdef IR_SUCC
void addDataDep(unsigned int *bytecode, unsigned char index, unsigned char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, index*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, index*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);

	int extendedSuccessor = successor;
	if (index != successor){

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
}

void addControlDep(unsigned int *bytecode, unsigned char index, unsigned char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, index*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, index*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, index*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	int extendedSuccessor = successor;
	if (index != successor){

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
}
#else

void addDataDep(unsigned int *bytecode, unsigned char index, unsigned char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, successor*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, successor*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, successor*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);

	int extendedDepName = index;
	if (index != successor){

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
}

void addControlDep(unsigned int *bytecode, unsigned char index, unsigned char successor){
	unsigned int bytecodeWord0 = readInt(bytecode, successor*16+12);
	unsigned int bytecodeWord32 = readInt(bytecode, successor*16+8);
	unsigned int bytecodeWord64 = readInt(bytecode, successor*16+4);

	char nbDSucc = ((bytecodeWord64>>3) & 7);
	char nbSucc = ((bytecodeWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	int extendedDepName = index;
	if (index != successor){
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
	}
	//We do not need to increment the number of dep...
}



/**************************************************************
 * Function clearControlDep
 * This function will remove all control predecessor of a given IR instruction
 * Arguments are :
 * - ir is the memory containing the IR
 * - index is the index of the instruction to clear in the IR block
 **************************************************************/

void clearControlDep(unsigned int *ir, unsigned char index){
	unsigned int irWord0 = readInt(ir, index*16+12);
	unsigned int irWord32 = readInt(ir, index*16+8);
	unsigned int irWord64 = readInt(ir, index*16+4);

	char nbDSucc = ((irWord64>>3) & 7);
	char nbSucc = ((irWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	//The mask to apply on the higher incomplete list of control dep
	unsigned int mask = (0xffffffff << ((nbCSucc & 0x3)<<3));
fprintf(stderr, "mask is %x\n", mask);
	if (nbCSucc >= 4){
		//We clear completely word0 and apply the mask to word32
		irWord0 = 0;
		irWord32 &= mask;
	}
	else{
		//We apply the mask to word0
		irWord0 &= mask;
	}

	//We update the number of predecessors: it is now only the number of data predecessors as
	// all control pred have been removed
	irWord64 = (irWord64 & 0xfffffff8) | nbDSucc;

	writeInt(ir, index*16+12, irWord0);
	writeInt(ir, index*16+8, irWord32);
	writeInt(ir, index*16+4, irWord64);

	fprintf(stderr, "Instruction %d has been cleared %x %x %x\n", index, irWord64, irWord32, irWord0);
}

char getControlDep(unsigned int *ir, unsigned char index, unsigned char *result){
	unsigned int irWord0 = readInt(ir, index*16+12);
	unsigned int irWord32 = readInt(ir, index*16+8);
	unsigned int irWord64 = readInt(ir, index*16+4);

	char nbDSucc = ((irWord64>>3) & 7);
	char nbSucc = ((irWord64>>0) & 7);
	char nbCSucc = nbSucc - nbDSucc;

	for (int onePred = 0; onePred < nbCSucc; onePred++){
		if (onePred >= 4)
			result[onePred] = (irWord32 >> (8*(onePred-4))) & 0xff;
		else
			result[onePred] = (irWord0 >> (8*onePred)) & 0xff;

	}
	return nbCSucc;
}

#endif

void addOffsetToDep(unsigned int *bytecode, unsigned char index, unsigned char offset){
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

char getStageCode(unsigned int *bytecode, unsigned char index){

	unsigned int bytecodeWord96 = readInt(bytecode, index*16+0);
	return (bytecodeWord96>>30) & 0x3;
}


int getNbInstr(IRProcedure *procedure){
	int result = 0;
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		result += procedure->blocks[oneBlock]->nbInstr;
	}
	return result;
}

int getNbInstr(IRProcedure *procedure, int type){
	int result = 0;
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++)
		for (int oneInstruction = 0; oneInstruction<procedure->blocks[oneBlock]->nbInstr; oneInstruction++)
			if (getStageCode(procedure->blocks[oneBlock]->instructions, oneInstruction) == type)
				result++;


	return result;
}

void IRBlock::print(FILE * output)
{
	fprintf(output, "digraph cgra {");
	for (uint32_t i = 0; i < this->nbInstr; ++i)
	{
		uint32_t instruction96, instruction64, instruction32, instruction0;
		instruction96 = readInt(instructions, i*16+0);
		instruction64 = readInt(instructions, i*16+4);
		instruction32 = readInt(instructions, i*16+8);
		instruction0 = readInt(instructions, i*16+12);

		uint8_t opCode = ((instruction96>>19) & 0x7f);
		uint8_t typeCode = ((instruction96>>28) & 0x3);
		bool isImm = ((instruction96>>18) & 0x1);
		uint16_t src1 = ((instruction96>>0) & 0x1ff);
		uint16_t src2 = ((instruction64>>23) & 0x1ff);
		uint16_t dst  = ((instruction64>>14) & 0x1ff);

		fprintf(output, "i%d [label=%s];", i, opcodeNames[opCode]);

		if (typeCode == 0)
		{
			if (opCode == VEX_STD || opCode == VEX_STW || opCode == VEX_STH || opCode == VEX_STB)
				if (dst < 256)
					fprintf(output, "i%d -> i%d;", dst, i);
				else
					fprintf(output, "r%d -> i%d;", dst-256, i);

			if (src2 < 256)
				fprintf(output, "i%d -> i%d;", src2, i);
			else
				fprintf(output, "r%d -> i%d;", src2-256, i);

			if (!isImm)
			{
				if (src1 < 256)
					fprintf(output, "i%d -> i%d;", src1, i);
				else
					fprintf(output, "r%d -> i%d;", src1-256, i);
			}
		}
	}
	fprintf(output, "}");
}
