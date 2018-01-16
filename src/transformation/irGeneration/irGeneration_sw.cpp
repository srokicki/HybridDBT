/*
 * irGeneration_sw.cpp
 *
 *  Created on: 5 janv. 2018
 *      Author: simon
 */

#ifndef __CATAPULT
//Includes not required by catapult
#include <cstdio>
#include <cstdlib>

#include <lib/endianness.h>
#include <lib/tools.h>
#include <dbt/dbtPlateform.h>
#endif

//Includes required by catapult
#include <isa/vexISA.h>
#include <isa/irISA.h>

//* Global values */
bool isOutsideNext_sw = 0;
bool droppedInstruction_sw = 0;

unsigned short outsideNext_pred1_reg_sw;
unsigned short outsideNext_pred1_sw;
unsigned short outsideNext_pred2_reg_sw;
short outsideNext_dest_reg_sw;
short outsideNext_imm_sw;

bool outsideNext_isImm_sw;
bool outsideNext_isLongImm_sw;
bool outsideNext_pred1_ena_sw;
bool outsideNext_pred1_solved_sw;

bool outsideNext_pred2_ena_sw;
bool outsideNext_dest_ena_sw;
bool outsideNext_dest_alloc_sw;




/***********************************************************************************
 * Function that adds a dependency on the IR.
 ******************
 * In this implementation it is just a renaming of functions from IR implementations.
 * In HW we needed s)êcial implementation to increase perfs
 ***********************************************************************************/

unsigned int writeDependency_sw(unsigned int *ir, unsigned char srcInstr, unsigned char destInstr, bool isData){

	if (isData)
		addDataDep(ir, srcInstr, destInstr);
	else
		addControlDep(ir, srcInstr, destInstr);

	return 0;

}


unsigned int irGenerator_sw(unsigned int *srcBinaries, unsigned int addressInBinaries, unsigned int blockSize,
		unsigned int *bytecode, int globalVariables[128], unsigned int globalVariableCounter){


		unsigned char indexInCurrentBlock = 0;
		short registers[128];

		/* Datastructures for dag construction*/
		for (int i=0; i<128; i++)
			registers[i] = -1;


		/* Generated code */
		unsigned char numbersSuccessor[256];
		unsigned char numbersDataSuccessor[256];
		unsigned char successors[256][30];

		unsigned char numbersPredecessor[256];
		int predecessors[256][8];

		/* Datastructure for RAW dependencies on global registers */
		int lastWriterOnGlobal[128] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
		unsigned char lastReaderOnGlobalCounter[128] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		unsigned char lastReaderOnGlobalPlaceToWrite[128] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int lastReaderOnGlobal[128][4] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

		/* Datastructure for control dependencies on memories */
		int lastWriterOnMemory = -1;

		bool lastWriterOnMemoryRegUnchanged = 0;
		unsigned char lastWriterOnMemoryReg = 0;
		unsigned short lastWriterOnMemoryImm = 0;

		unsigned char lastReaderOnMemoryCounter = 0;
		unsigned char lastReaderOnMemoryPlaceToWrite = 0;
		int lastReaderOnMemory[4] = {0,0,0,0};

		int isCallBlock = 0;
		bool haveJump = 0;
		unsigned char jumpID = 0;
		short indexInSourceBinaries_instr = -1;
		unsigned char instructionTranslatedFromPreviousSyllabus = 0;

		do {

			/********************************************************************
			 * First step is to fetch the instruction to translate
			 *
			 * For this, we load a complete 128-bits instruction word from the VLIW
			 * and consider each instruction not empty. If there is more than one, the
			 * second one is kept for next iteration
			 ********************************************************************/
			//FIXME handle the mov insertion
			bool insertMove_ena = 0;
			droppedInstruction_sw = 0;
			unsigned short insertMove_src;


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
			 */

			unsigned int oneInstruction = 0;
			bool found = false;

			indexInSourceBinaries_instr++;
			for (int oneOffset = 0; indexInSourceBinaries_instr+oneOffset<(blockSize+1)*4; oneOffset++){
				if (srcBinaries[indexInSourceBinaries_instr+oneOffset+addressInBinaries*4] != 0){
					oneInstruction = srcBinaries[indexInSourceBinaries_instr+oneOffset+addressInBinaries*4];
					indexInSourceBinaries_instr = indexInSourceBinaries_instr+oneOffset;
					found = true;
					break;

				}
			}

			//If no instruction to analyze has been found, we can break
			if (!found)
				break;

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

			unsigned char opcode = oneInstruction & 0x7f;
			short imm13 = ((oneInstruction>>7) & 0x1fff);
			if (imm13 >= 4096)
				imm13 -= 8192;

			int imm19 = ((oneInstruction>>7) & 0x7ffff);
			unsigned char reg8 = ((oneInstruction>>8) & 0x3f);
			unsigned char reg14 = ((oneInstruction>>14) & 0x3f);
			unsigned char reg20 = ((oneInstruction>>20) & 0x3f);
			unsigned char reg26 = ((oneInstruction>>26) & 0x3f);

			unsigned char funct = ((oneInstruction>>7) & 0x1f);
			bool isIType = ((opcode>>4) & 0x7) == 2;

			bool isLoadType = opcode == VEX_LDB | opcode == VEX_LDBU | opcode == VEX_LDH
					| opcode == VEX_LDHU | opcode == VEX_LDW | opcode == VEX_LDWU | opcode == VEX_LDD;
			bool isStoreType = opcode == VEX_STB | opcode == VEX_STH | opcode == VEX_STW | opcode == VEX_STD;
			bool isBranchWithNoReg = opcode == VEX_GOTO | opcode == VEX_CALL | opcode == VEX_RETURN
					| opcode == VEX_STOP | opcode == VEX_ECALL;
			bool isBranchWithReg = opcode == VEX_GOTOR | opcode == VEX_CALLR | opcode == VEX_BR
					| opcode == VEX_BRF;
			bool isMovi = opcode == VEX_MOVI;
			bool isArith1 = opcode == VEX_NOT;
			bool isArith2 = (((opcode>>4) & 0x7) == 4 | ((opcode>>4) & 0x7) == 5) & !isArith1;
			bool isArithImm = ((opcode>>4) & 0x7) == 6 | ((opcode>>4) & 0x7) == 7;
			bool isMultType = ((opcode>>4) & 0x7) == 0;
			bool isProfile = opcode == VEX_PROFILE;
			bool isFMADD = opcode == VEX_FMADD || opcode == VEX_FMSUB ||opcode == VEX_FNMADD ||opcode == VEX_FNMSUB;
			bool isFSW = opcode == VEX_FSW || opcode == VEX_FSH || opcode == VEX_FSB;
			bool isFLW = opcode == VEX_FLW || opcode == VEX_FLH || opcode == VEX_FLB;
			bool isFP = opcode == VEX_FP;

			bool isFloatRa = isFSW || isFMADD || (isFP && funct != VEX_FP_FCVTWS && funct != VEX_FP_FCVTWUS && funct != VEX_FP_FMVWX);
			bool isFloatDest = isFLW || isFMADD || (isFP && funct != VEX_FP_FCVTSW && funct != VEX_FP_FCVTSWU && funct != VEX_FP_FMVXW
					&& funct != VEX_FP_FMIN && funct != VEX_FP_FMAX && funct != VEX_FP_FCLASS);


			bool pred1_ena = 0, pred2_ena = 0, dest_ena = 0;
			unsigned char pred1_reg = reg26, pred2_reg = reg20, dest_reg=0;

			//Solving accessed register 1
			if (!isBranchWithNoReg && !isMovi && !isProfile)
				pred1_ena = 1;

			//Solving accessed register 2
			if (isStoreType || isArith2 || isMultType || isFSW || isFP)
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

			if (opcode == VEX_NOP){
				dest_ena = 0;
				pred2_ena = 0;
				pred1_ena = 0;
				droppedInstruction_sw = 1;
			}

			if (isFloatRa){
				pred1_reg += 64;
				if (pred2_ena)
					pred2_reg += 64;
			}

			if (isFloatDest){
				dest_reg += 64;
			}


			numbersSuccessor[indexInCurrentBlock] = 0;
			numbersPredecessor[indexInCurrentBlock] = 0;

			/********************************************************************
			 * Third step is to solve/build dependencies in the current block
			 *
			 * This is the delicate step of the generation
			 * TODO detailled description of the process
			 ********************************************************************/


			bool pred1_succ_ena = 0;
			unsigned char pred1_succ_src;
			bool pred1_succ_isData = 0;

			short temp_pred1 = registers[pred1_reg];


			//We perform memory accesses for pred1
			short pred1;
			bool pred1_global = 0;
			short pred1_global_address = pred1_reg;
			short pred1_global_access = globalVariables[pred1_reg];
			unsigned char lastReaderOnGlobalCounter_access_pred1 = lastReaderOnGlobalCounter[pred1_reg];
			unsigned char lastReaderOnGlobalPlaceToWrite_access_pred1 = lastReaderOnGlobalPlaceToWrite[pred1_reg];
			unsigned char lastReaderOnGlobalPlaceToWrite_access_pred1_old = lastReaderOnGlobalPlaceToWrite_access_pred1;
			short lastWriterOnGlobal_access_pred1 = lastWriterOnGlobal[pred1_reg];
			short lastReaderOnGlobal_value_pred1 = lastReaderOnGlobal[pred1_reg][lastReaderOnGlobalPlaceToWrite_access_pred1];
			unsigned short pred1_global_value = 0;

			//We perform memory accesses for pred2
			bool pred2_global = 0;
			unsigned short pred2_global_address = pred2_reg;
			short pred2_global_access = globalVariables[pred2_reg];
			unsigned char lastReaderOnGlobalCounter_access_pred2 = lastReaderOnGlobalCounter[pred2_reg];
			unsigned char lastReaderOnGlobalPlaceToWrite_access_pred2 = lastReaderOnGlobalPlaceToWrite[pred2_reg];
			unsigned char lastReaderOnGlobalPlaceToWrite_access_pred2_old = lastReaderOnGlobalPlaceToWrite_access_pred2;
			short lastWriterOnGlobal_access_pred2 = lastWriterOnGlobal[pred2_reg];
			short lastReaderOnGlobal_value_pred2 = lastReaderOnGlobal[pred2_reg][lastReaderOnGlobalPlaceToWrite_access_pred2];
			unsigned short pred2_global_value = 0;

			//We perform memory accesses for dest
			unsigned short dest_global_address = dest_reg;
			short dest_global_access = globalVariables[dest_reg];
			unsigned char lastReaderOnGlobalCounter_access_dest = lastReaderOnGlobalCounter[dest_reg];
			short lastWriterOnGlobal_access_dest = lastWriterOnGlobal[dest_reg];
			short lastReaderOnGlobal_value_dest_1 = lastReaderOnGlobal[dest_reg][0];
			short lastReaderOnGlobal_value_dest_2 = lastReaderOnGlobal[dest_reg][1];
			short lastReaderOnGlobal_value_dest_3 = lastReaderOnGlobal[dest_reg][2];
			unsigned char lastReaderOnGlobalPlaceToWrite_access_dest = lastReaderOnGlobalPlaceToWrite[dest_reg];


			if (pred1_ena){

				pred1_global = (temp_pred1 < 0);

				if (pred1_global){ //If value comes from global register

					if (pred1_global_access < 0){ //If value is not assigned yet, we allocate the value from globalVariableCounter

						temp_pred1 = globalVariableCounter;
						pred1_global_access = globalVariableCounter++;
					}
					else{ //Otherwise we use the already allocated value

						temp_pred1 = pred1_global_access;
					}



					pred1_global_value = temp_pred1;

					//** We also mark this node as a reader of the global value. Should this value be modified
					//** in the block, we will add dependencies


					if (lastReaderOnGlobalCounter_access_pred1 < 3){
						lastReaderOnGlobalCounter_access_pred1++;
						//** We handle successors: if the value in a global register has been written in the same
						//** basic block, we add a dependency from the writer node to this instruction.
						if (lastWriterOnGlobal_access_pred1 >= 0){
							pred1_succ_ena = 1;
							pred1_succ_isData = 1;
							pred1_succ_src = lastWriterOnGlobal_access_pred1;
						}
					}
					else{

						int readerToEvince = lastReaderOnGlobal_value_pred1;
						pred1_succ_ena = 1;
						pred1_succ_src = readerToEvince;
					}
					lastReaderOnGlobal_value_pred1 = indexInCurrentBlock;

					lastReaderOnGlobalPlaceToWrite_access_pred1 = (lastReaderOnGlobalPlaceToWrite_access_pred1 + 1)%3;



					//We use the last writer as a source and not the global variable
					if (lastWriterOnGlobal_access_pred1 != -1)
						temp_pred1 = lastWriterOnGlobal_access_pred1;
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
			short temp_pred2 = registers[pred2_reg];
			unsigned short pred2;


			bool pred2_succ_ena = 0;
			unsigned char pred2_succ_src = 0;
			bool pred2_succ_isData = 0;



			pred2_global_access = (pred2_global_address == pred1_global_address) ? pred1_global_access : pred2_global_access;


			if (pred2_ena){

				//To gather succ information on different path
				pred2_global = (temp_pred2 < 0);
				if (pred2_global){ //If value comes from global register
					if (pred2_global_access < 0){
						temp_pred2 = globalVariableCounter;
						pred2_global_access = globalVariableCounter++;
					}
					else
						temp_pred2 = pred2_global_access;


					pred2_global_value = temp_pred2;

					//If the global register has been used in the current block, we add a control dependency

					if (!(pred1_global && pred1_global_address == pred2_global_address)){
						if (lastReaderOnGlobalCounter_access_pred2 < 3){
							lastReaderOnGlobalCounter_access_pred2++;
							if (lastWriterOnGlobal_access_pred2 >= 0){
								pred2_succ_ena = 1;
								pred2_succ_isData = 1;
								pred2_succ_src = lastWriterOnGlobal_access_pred2;
							}
						}
						else{
							int readerToEvince = lastReaderOnGlobal_value_pred2;
							pred2_succ_ena = 1;
							pred2_succ_src = readerToEvince;
						}
						lastReaderOnGlobal_value_pred2 = indexInCurrentBlock;
						lastReaderOnGlobalPlaceToWrite_access_pred2 = (lastReaderOnGlobalPlaceToWrite_access_pred2 + 1)%3;


					}


					//We use the last writer as a source and not the global variable
					if (lastWriterOnGlobal_access_pred2 != -1)
						temp_pred2 = lastWriterOnGlobal_access_pred2;
				}
				else{
					//We are facing a simple data dependency
					pred2_succ_ena = 1;
					pred2_succ_src = temp_pred2;
					pred2_succ_isData = 1;
				}

				pred2 = temp_pred2;



				predecessors[indexInCurrentBlock][numbersPredecessor[indexInCurrentBlock]++] = pred2;
			}


			if (isLoadType || isFLW){
				/****************************/
				/* We update lastReaderOneMemory and add required dependencies to keep memory coherence */
				unsigned short succ_src;
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
				lastReaderOnMemoryPlaceToWrite = (lastReaderOnMemoryPlaceToWrite + 1) % 3;
			}



			//******************************************
			//We set the destination

			bool global_succ_ena_1 = 0;
			bool global_succ_ena_2 = 0;
			bool global_succ_ena_3 = 0;
			bool global_succ_ena_4 = 0;

			unsigned char global_succ_src_1;
			unsigned char global_succ_src_2;
			unsigned char global_succ_src_3;
			unsigned char global_succ_src_4;


			short temp_destination = indexInCurrentBlock;
			unsigned short destination;



			bool alloc = 1;



			//When reading global name, we make sure the global variable has not been named by pred1 or pred2
			dest_global_access = (dest_global_address == pred1_global_address) ? pred1_global_access : dest_global_access;
			dest_global_access = (dest_global_address == pred2_global_address) ? pred2_global_access : dest_global_access;


			if (dest_ena) {

				if (lastWriterOnMemoryReg == dest_reg)
					lastWriterOnMemoryRegUnchanged = 0;

				if (dest_global_access < 0 || insertMove_ena){

					registers[dest_reg] = indexInCurrentBlock;

				}
				else{


					alloc = 0;
					temp_destination = dest_global_access;
					lastWriterOnGlobal_access_dest = indexInCurrentBlock;

					if (lastReaderOnGlobalCounter_access_dest >= 1 && lastReaderOnGlobal_value_dest_1 != indexInCurrentBlock){
						global_succ_ena_1 = 1;
						global_succ_src_1 = lastReaderOnGlobal_value_dest_1;
					}
					if (lastReaderOnGlobalCounter_access_dest >= 2 && lastReaderOnGlobal_value_dest_2 != indexInCurrentBlock){
						global_succ_ena_2 = 1;
						global_succ_src_2 = lastReaderOnGlobal_value_dest_2;
					}
					if (lastReaderOnGlobalCounter_access_dest >= 3 && lastReaderOnGlobal_value_dest_3 != indexInCurrentBlock){
						global_succ_ena_3 = 1;
						global_succ_src_3 = lastReaderOnGlobal_value_dest_3;
					}

					lastReaderOnGlobalCounter_access_dest = 0;
					lastReaderOnGlobalPlaceToWrite_access_dest = 0;

				}
			}
			else {
				alloc=0;
			}
			destination = temp_destination;

			if (isStoreType || isFSW){

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

			struct uint128_struct oneBytecodeStruct = {0,0,0,0};

			if (insertMove_ena){
				//TODO

				#ifndef __CATAPULT
				printf("Implementation do not support mov insertion yet...\n Exiting...\n");
				exit(-1);
				#endif

			}
			else if (isBranchWithNoReg || isBranchWithReg){

				if (opcode == VEX_GOTO){
					pred1 = 0;
				}
				else if (opcode == VEX_CALL){
					isCallBlock = 1;
					pred1 = destination;
				}

				oneBytecodeStruct = assembleIBytecodeInstruction(0, 0, opcode, pred1, imm19, 0);

				haveJump = 1;
				jumpID = indexInCurrentBlock;

			}
			else if (isMovi){
				oneBytecodeStruct = assembleIBytecodeInstruction(2, alloc, opcode, destination, imm19, 0);

			}
			else if (isStoreType || isFSW){
				oneBytecodeStruct = assembleRiBytecodeInstruction(1, 0, opcode, pred1, imm13, pred2, 0);

			}
			else if (isLoadType || isFLW){
				oneBytecodeStruct = assembleRiBytecodeInstruction(1, alloc, opcode, pred1, imm13, destination, 0);

			}
			else if (isMultType){
				oneBytecodeStruct = assembleRBytecodeInstruction(3, alloc, opcode, pred2, pred1, destination, 0);
			}
			else if (isArith1 || isArithImm){
				oneBytecodeStruct = assembleRiBytecodeInstruction(2, alloc, opcode, pred1, imm13, destination, 0);
			}
			else if (isArith2){
				oneBytecodeStruct = assembleRBytecodeInstruction(2, alloc, opcode, pred2, pred1, destination, 0);
			}
			else if (isProfile){
				oneBytecodeStruct = assembleRiBytecodeInstruction(1, 0, opcode, 256, imm13, 256, 0);
			}
			else if (isFMADD){
				oneBytecodeStruct = assembleRiBytecodeInstruction(1, 0, opcode, 256, imm13, 256, 0); //TODO
			}
			else if (isFP){
				oneBytecodeStruct = assembleFPBytecodeInstruction(3, alloc, opcode, funct, pred2, pred1, destination, 0);

			}
			else{
				#ifndef __CATAPULT
				printf("While generating IR, this case should never happen... %x\n", oneInstruction);
				#endif
			}


			/**************************************************************************
			 *  We place the instruction in memory
			 *************************************
			 *
			 * Last step is to place the instruction in the bytecode memory.
			 */

			write128(bytecode, indexInCurrentBlock*16, oneBytecodeStruct);

			//*********************************
			//We add dependencies
			//Note: we never have more than 5 dependencies to add for one single instruction

			if (pred1_succ_ena){
				unsigned char nbSucc_pred1 = writeDependency_sw(bytecode, pred1_succ_src, indexInCurrentBlock, pred1_succ_isData);
				if (pred1_succ_isData & (nbSucc_pred1 == 7)){
						insertMove_ena = 1;
						insertMove_src = pred1;
						alloc = 1;
						destination = indexInCurrentBlock;
						dest_ena = 1;
				}
			}

			if (pred2_succ_ena){
				unsigned char nbSucc_pred2 = writeDependency_sw(bytecode, pred2_succ_src, indexInCurrentBlock, pred2_succ_isData);
				if (pred2_succ_isData & (nbSucc_pred2 == 7)){
						insertMove_ena = 1;
						insertMove_src = pred2;
						alloc = 1;
						destination = indexInCurrentBlock;
						dest_ena = 1;
				}
			}


			if (global_succ_ena_1)
				writeDependency_sw(bytecode, global_succ_src_1, indexInCurrentBlock, false);


			if (global_succ_ena_2)
				writeDependency_sw(bytecode, global_succ_src_2, indexInCurrentBlock, false);


			if (global_succ_ena_3)
				writeDependency_sw(bytecode, global_succ_src_3, indexInCurrentBlock, false);









			if (!insertMove_ena & !droppedInstruction_sw)
				indexInCurrentBlock++;


		}
		while ((indexInSourceBinaries_instr/4)<=blockSize && indexInCurrentBlock<255);



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
		if (haveJump){

			#ifdef IR_SUCC

			for (int oneInstructionFromBlock = 0; oneInstructionFromBlock < indexInCurrentBlock; oneInstructionFromBlock++){
//TODO
				ac_int<128, false> bytecodeWord = bytecode[oneInstructionFromBlock];
				unsigned char nbSucc = bytecodeWord.slc<3>(64);

				if (nbSucc == 0 && oneInstructionFromBlock != jumpID){
					writeDependency_sw(bytecode, oneInstructionFromBlock,jumpID, false);
				}
			}

			#else

			unsigned char olderDependency[4];
			unsigned char nbOlderDependency = 0;
			unsigned char writeOlderDependency = 0;

			for (int oneInstructionFromBlock = 0; oneInstructionFromBlock < indexInCurrentBlock; oneInstructionFromBlock++){

				unsigned int bytecodeWord64 = bytecode[oneInstructionFromBlock*4+1];
				unsigned char nbSucc = (bytecodeWord64>>6)&0xff;

				if (nbSucc == 0 && oneInstructionFromBlock != jumpID){
					if (nbOlderDependency < 4){
						olderDependency[writeOlderDependency] = oneInstructionFromBlock;
						nbOlderDependency++;
						writeOlderDependency = (writeOlderDependency + 1) % 4;
					}
					else{
						writeDependency_sw(bytecode, olderDependency[writeOlderDependency],oneInstructionFromBlock, false);
						olderDependency[writeOlderDependency] = oneInstructionFromBlock;

						writeOlderDependency = (writeOlderDependency + 1) % 4;
					}
				}
			}

			for (int oneOlder=0; oneOlder<nbOlderDependency; oneOlder++){
				writeOlderDependency = (writeOlderDependency - 1) & 0x3;
				writeDependency_sw(bytecode, olderDependency[writeOlderDependency],jumpID, false);
			}
		#endif

		}




unsigned int valueToReturn = indexInSourceBinaries_instr*4;
valueToReturn = (valueToReturn<<16) + indexInCurrentBlock;
return valueToReturn;


}
