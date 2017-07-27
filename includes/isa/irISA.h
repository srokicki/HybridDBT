/*
 * irISA.h
 *
 *  Created on: 24 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_IRISA_H_
#define INCLUDES_ISA_IRISA_H_

#include <types.h>


/********************************************************************
 * IR configuration
 * ******************************************************************
 *
 * We define here preprocessor values which change the IR configuration.
 * IR_SUCC says that the IR contains forward dependencies: each instruction knows all its successors
 * 			If not defined, the IR will be backward: each instruction will know all its predecessors
 *
 *******************************************************************/

//#define IR_SUCC

/********************************************************************
 * Declaration of a data structure to represent the control flow of the binaries analyzed.
 * ******************************************************************
 *
 * The idea here is to offer a light representation of the control flow which can be built dynamically
 * and allow the DBT process to keep track of what has been done and what still need to be done.
 *
 * This representation will have to be extended during the development of the DBT tool in order to
 * track more advances information (like for example if a procedure was moved to a different location
 * or optimized with some specifics parameters...)
 *
 *******************************************************************/
class IRBlock;


/* IRProcedure is meant to represent a procedure in the binaries.
 * Its current implementation only store its start/end address and a pointer to the basic blocks*/
class IRProcedure
{
public:
	IRBlock *entryBlock;			//pointer to the entry block of the procedure
	IRBlock **blocks;				//A pointer to an array of blocks
	int nbBlock;
	char issueWidth;
	char configuration;

	unsigned int procedureState;	//A value to store its state (optimized/translated or other things like that)

	void print();
	IRProcedure(IRBlock *entryBlock, int nbBlock);

};

/* IRBlock represent a block in the binaries.
 * The data structure only store start/end address as well as a pointer to some instructions if the IR was stored.*/
class IRBlock
{
public:
	//Link with source binaries
	unsigned int sourceStartAddress; //This represent the block start address in source binaries
	unsigned int sourceEndAddress;	 //This represent the block end address in source binaries
	unsigned int sourceDestination;	 //This represent the jump destination if any. If there are no jump of unpredictable jump its value is 0.

	//Link with VLIW binaries
	unsigned int vliwStartAddress;	//Address of the first instruction in the block
	unsigned int vliwEndAddress;   	//End address is the address of the first instruction not in the block

	//Control flow graph
	char nbSucc;					//Number of successors
	IRBlock* successor1;			//pointer to first successor
	IRBlock* successor2;			//pointer to second successor
	short jumpID;					//Index of the jump instruction in the block's list of instruction
	unsigned int jumpPlace;			//Address of the jump instruction in the VLIW memory

	uint32 *instructions;			//A pointer to an array of uint128 describing the instructions
	int nbInstr;					//The number of instructions

	unsigned int blockState;		//A value to store its state (optimized/translated or other things like that)

	int section;



	IRBlock(int startAddress, int endAddress, int section);
	~IRBlock();
};

/* Definition of different states possible for the IRBlock:
 * IRBLOCK_STATE_FIRSTPASS the block is simply translated
 * IRBLOCK_STATE_PROFILED the block has been translated and additional code is added to profile it
 * IRBLOCK_STATE_SCHEDULED the block has been elected to be scheduled. As a consequence, instructions* hold the IR instructions
 */

#define IRBLOCK_STATE_FIRSTPASS 0
#define IRBLOCK_STATE_PROFILED 1
#define IRBLOCK_STATE_SCHEDULED 2
#define IRBLOCK_PROC 3
#define IRBLOCK_UNROLLED 4


#define IRBLOCK_STATE_RECONF 5

class IRApplication{
public:
	int numberOfSections;
	IRBlock*** blocksInSections;
	int *numbersBlockInSections;

	IRProcedure** procedures;
	int numberProcedures;

	void addBlock(IRBlock *block, int sectionNumber);
	void addProcedure(IRProcedure *procedure);

	IRApplication(int numberSections);

	int numberAllocatedProcedures;
	int *numbersAllocatedBlockInSections;


};


/********************************************************************
 * Declaration functions to assemble uint128 instruction for IR
 * ******************************************************************
 *
 * To represent blocks, the IR uses a set of uint128 instruction which describe the data-flow graph inside the block.
 * Above are defined a set of procedure which encode instruction following our encoding.
 *
 *******************************************************************/

#ifndef __NIOS

ac_int<128, false> assembleRBytecodeInstruction(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<9, false> regB, ac_int<9, false> regDest,
		ac_int<8, false> nbDep);

ac_int<128, false> assembleRiBytecodeInstruction(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<13, false> imm13,
		ac_int<9, false> regDest, ac_int<8, false> nbDep);

ac_int<128, false> assembleIBytecodeInstruction(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> reg, ac_int<19, true> imm19, ac_int<8, false> nbDep);

#endif

/********************************************************************
 * Declaration of debug function
 * ******************************************************************
 *
 * These functions are used to print IR elements in order to debug the DBT process.
 *
 *******************************************************************/

void printBytecodeInstruction(int index, uint32  instructionPart1, uint32  instructionPart2, uint32 instructionPart3, uint32 instructionPart4);

/********************************************************************
 * Declaration utilization functions
 * ******************************************************************
 *
 * These functions are used to access to information from the IR
 *
 *******************************************************************/

short getDestinationRegister(uint32 *bytecode, char index);
char getOperands(uint32 *bytecode, char index, short result[2]);
void setOperands(uint32 *bytecode, char index, short operands[2]);

char getOpcode(uint32 *bytecode, char index);
void setOpcode(uint32 *bytecode, char index, char newOpcode);

void setDestinationRegister(uint32 *bytecode, char index, short newDestinationRegister);
void setAlloc(uint32 *bytecode, char index, char newAlloc);
void addDataDep(uint32 *bytecode, char index, char successor);
void addControlDep(uint32 *bytecode, char index, char successor);
void addOffsetToDep(uint32 *bytecode, char index, char offset);

/********************************************************************
 * Declaration of stage codes
 * ******************************************************************
 *
 * Stage code are used to define the kind of instruction used
 *
 *******************************************************************/

#define STAGE_CODE_CONTROL 0
#define STAGE_CODE_MEMORY 1
#define STAGE_CODE_ARITH 2
#define STAGE_CODE_MULT 3


#endif /* INCLUDES_ISA_IRISA_H_ */
