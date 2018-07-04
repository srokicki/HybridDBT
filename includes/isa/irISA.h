/*
 * irISA.h
 *
 *  Created on: 24 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_IRISA_H_
#define INCLUDES_ISA_IRISA_H_

#include <types.h>
#include <string>

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
	char configuration, previousConfiguration;
	int configurationScores[32];
	char state;

	unsigned int procedureState;	//A value to store its state (optimized/translated or other things like that)

	/**
	 * @brief print: prints the procedure's CFG in .dot format in a file
	 * @param output: the file to print in
	 */
	void print(FILE *output);
	IRProcedure(IRBlock *entryBlock, int nbBlock);

};

/* IRBlock represent a block in the binaries.
 * The data structure only store start/end address as well as a pointer to some instructions if the IR was stored.*/
class IRBlock
{
public:
	//address of the unique reference to the pointer
	IRBlock** reference = NULL;

	//Link with source binaries
	unsigned int sourceStartAddress; //This represent the block start address in source binaries
	unsigned int sourceEndAddress;	 //This represent the block end address in source binaries
	unsigned int sourceDestination;	 //This represent the jump destination if any. If there are no jump of unpredictable jump its value is 0.

	//Link with VLIW binaries
	unsigned int vliwStartAddress;	//Address of the first instruction in the block
	unsigned int oldVliwStartAddress;	//Address of the first instruction in the block

	unsigned int vliwEndAddress;   	//End address is the address of the first instruction not in the block

	//Control flow graph
	char nbSucc;					//Number of successors
	IRBlock* successors[10];
	IRBlock* successor1;			//pointer to first successor
	IRBlock* successor2;			//pointer to second successor

	short jumpID;					//Index of the jump instruction in the block's list of instruction
	unsigned int jumpPlace; 		//Address of the jump instruction in the VLIW memory

	char nbJumps;
	unsigned char *jumpIds;
	unsigned int *jumpPlaces;

	unsigned int *instructions;			//A pointer to an array of uint128 describing the instructions
	int nbInstr;					//The number of instructions

	unsigned int blockState;		//A value to store its state (optimized/translated or other things like that)

	int section;
	IRBlock** placeInProfiler;

	short specAddr[4];

	void addJump(unsigned char jumpID, unsigned int jumpPlace);

	IRBlock(int startAddress, int endAddress, int section);
	~IRBlock();

	/**
	 * @brief print: prints the block's DFG in .dot format in a file
	 * @param output: the file to print in
	 */
	void print(FILE *output);
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
#define IRBLOCK_PERFECT_LOOP 4
#define IRBLOCK_UNROLLED 5


#define IRBLOCK_STATE_RECONF 6

class IRApplication{
public:
	int numberOfSections;
	IRBlock*** blocksInSections;
	int *numbersBlockInSections;

	IRProcedure** procedures;
	int numberProcedures;
	int numberInstructions;


	void addBlock(IRBlock *block, int sectionNumber);
	void addProcedure(IRProcedure *procedure);

	IRApplication(int numberSections);
	~IRApplication();

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

struct uint128_struct assembleRBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short regA, short regB, short regDest,	unsigned char nbDep);
struct uint128_struct assembleFPBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, char funct, short regA, short regB, short regDest, unsigned char nbDep);
struct uint128_struct assembleRiBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short regA, short imm13, short regDest, unsigned char nbDep);
struct uint128_struct assembleIBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short reg, int imm19, unsigned char nbDep);
struct uint128_struct assembleMemoryBytecodeInstruction(char stageCode, char isAlloc,
		char opcode, short regA, short imm12, bool isSpec, char specId,
		short regDest, unsigned char nbDep);


#ifndef __SW
#ifndef __HW

ac_int<128, false> assembleRBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<9, false> regB, ac_int<9, false> regDest,
		ac_int<8, false> nbDep);

ac_int<128, false> assembleRiBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> regA, ac_int<13, false> imm13,
		ac_int<9, false> regDest, ac_int<8, false> nbDep);

ac_int<128, false> assembleIBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<9, false> reg, ac_int<19, true> imm19, ac_int<8, false> nbDep);
ac_int<128, false> assembleFPBytecodeInstruction_hw(ac_int<2, false> stageCode, ac_int<1, false> isAlloc,
		ac_int<7, false> opcode, ac_int<5, false> funct, ac_int<9, false> regA, ac_int<9, false> regB, ac_int<9, false> regDest,
		ac_int<8, false> nbDep);

#endif
#endif


/********************************************************************
 * Declaration of debug function
 * ******************************************************************
 *
 * These functions are used to print IR elements in order to debug the DBT process.
 *
 *******************************************************************/

std::string printBytecodeInstruction(int index, unsigned int  instructionPart1, unsigned int instructionPart2, unsigned int instructionPart3, unsigned int instructionPart4);

/********************************************************************
 * Declaration utilization functions
 * ******************************************************************
 *
 * These functions are used to access to information from the IR
 *
 *******************************************************************/

short getDestinationRegister(unsigned int *bytecode, unsigned char index);
char getOperands(unsigned int *bytecode, unsigned char index, short result[2]);
void setOperands(unsigned int *bytecode, unsigned char index, short operands[2]);

void setImmediateValue(unsigned int *bytecode, unsigned char index, int value);
bool getImmediateValue(unsigned int *bytecode, unsigned char index, int* result);

char getOpcode(unsigned int *bytecode, unsigned char index);
void setOpcode(unsigned int *bytecode, unsigned char index, char newOpcode);

void setDestinationRegister(unsigned int *bytecode, unsigned char index, short newDestinationRegister);
void setAlloc(unsigned int *bytecode, unsigned char index, char newAlloc);
void addDataDep(unsigned int *bytecode, unsigned char index, unsigned char successor);
void addControlDep(unsigned int *bytecode, unsigned char index, unsigned char successor);
void clearControlDep(unsigned int *ir, unsigned char index);
char getControlDep(unsigned int *ir, unsigned char index, unsigned char *result);
void addOffsetToDep(unsigned int *bytecode, unsigned char index, unsigned char offset);
char getStageCode(unsigned int *bytecode, unsigned char index);

int getNbInstr(IRProcedure *procedure);
int getNbInstr(IRProcedure *procedure, int type);

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
