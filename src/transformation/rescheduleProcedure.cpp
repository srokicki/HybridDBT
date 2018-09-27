/*
 * rescheduleProcedure.cpp
 *
 *  Created on: 30 mai 2017
 *      Author: Simon Rokicki
 */
#include <cstring>
#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>
#include <transformation/irScheduler.h>
#include <lib/endianness.h>
#include <transformation/reconfigureVLIW.h>
#include <transformation/rescheduleProcedure.h>
#include <lib/log.h>

/******************************************************************************************
 ******************************  Reschedule Procedure
 ******************************************************************************************
 * This transformation will take as argument a procedure and will schedule and place all
 * blocks. It will also modify the branch parameters so that they target the new block locations
 * and will insert some jump in the old code to link toward the new location of binaries.
 *
 * Note that this transformation MODIFY the vliw code memory but DO NOT NEED THE VLIW TO BE STOPPED.
 * Indeed, the code modification is done at a new location and the linkage is only done at the end
 * of the transformation. At any time, if the vliw is executing the code modified, it should not affect
 * the normal execution.
 *
 * Arguments are :
 * 	-> platform is the DBTPlatform containing all global memories
 * 	-> procedure is the procedure that we want to schedule
 * 	-> writePlace is the destination when we will generate the new binaries
 *
 * 	Transformation returns the sum of writePlace and the size of the generated binaries.
 ******************************************************************************************/

int rescheduleProcedure(DBTPlateform *platform, IRProcedure *procedure,int writePlace){

	IRProcedure *scheduledProc = rescheduleProcedure_schedule(platform, procedure, writePlace);
	return rescheduleProcedure_commit(platform, procedure, writePlace, scheduledProc);
}

IRProcedure* rescheduleProcedure_schedule(DBTPlateform *platform, IRProcedure *procedure,int writePlace){

	char issueWidth = getIssueWidth(procedure->configuration);
	char incrementInBinaries = (getIssueWidth(procedure->configuration)>4) ? 2 : 1;

	IRBlock **blocks = (IRBlock **) malloc(procedure->nbBlock * sizeof(IRBlock*));
	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		blocks[oneBlock] = new IRBlock(-1,-1,-1);
		if (1){
			for (int i=procedure->blocks[oneBlock]->vliwStartAddress;i<procedure->blocks[oneBlock]->vliwEndAddress;i++){
				Log::printf(LOG_SCHEDULE_PROC,"%d ", i);

				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+0]).c_str());
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+1]).c_str());
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+2]).c_str());
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+3]).c_str());


				if (platform->vliwInitialIssueWidth>4){
					Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+4]).c_str());
					Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+5]).c_str());
					Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+6]).c_str());
					Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+7]).c_str());
					i++;
				}
				Log::printf(LOG_SCHEDULE_PROC,"\n");
			}
		}

	}
	IRProcedure *result = new IRProcedure(blocks[0], procedure->nbBlock);
	result->blocks = blocks;
	result->configuration = procedure->configuration;

	/******************************************************************************************
	 ******************************  Scheduling of all blocks
	 ******************************************************************************************
	 * For each block of the procedure, we will do the instruction scheduling and keep track of their
	 * previous start address as well as the place where the jump instruction (if any) as been scheduled.
	 ******************************************************************************************/

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];

		bool isCallBlock = false;
		bool isReturnBlock = false;

		if (block->nbJumps > 0){
			char opcode = getOpcode(block->instructions, block->jumpIds[block->nbJumps-1]);
			isCallBlock = opcode == VEX_CALL || opcode == VEX_CALLR;
			isReturnBlock = opcode == VEX_GOTOR;

		}


		//We move instructions into bytecode memory
		for (int oneBytecodeInstr = 0; oneBytecodeInstr<block->nbInstr; oneBytecodeInstr++){
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, block->instructions[4*oneBytecodeInstr + 0]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, block->instructions[4*oneBytecodeInstr + 1]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, block->instructions[4*oneBytecodeInstr + 2]);
			writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, block->instructions[4*oneBytecodeInstr + 3]);
		}

		//We initialize other memories
		for (int oneFreeRegister = 34; oneFreeRegister<63; oneFreeRegister++)
			platform->freeRegisters[oneFreeRegister-34] = oneFreeRegister;

		for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
			platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;

		//same for FP registers
		for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
			platform->placeOfRegisters[256+64+onePlaceOfRegister] = onePlaceOfRegister;


		//This is only for debug
		if (platform->debugLevel > 1 || 1){
			Log::printf(LOG_SCHEDULE_PROC,"Block %x:\n", block->sourceStartAddress);
			for (int i=0; i<block->nbInstr; i++)
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12)).c_str());
		}


		//We call the scheduler
		int binaSize = irScheduler(platform, 1,block->nbInstr, writePlace, 29, procedure->configuration);

		result->blocks[oneBlock]->vliwStartAddress = writePlace;
		result->blocks[oneBlock]->vliwEndAddress = writePlace + binaSize;

		//We copy and modify the jump ids and their location
		result->blocks[oneBlock]->jumpIds = (unsigned char*) malloc(block->nbJumps * sizeof(unsigned char));
		result->blocks[oneBlock]->jumpPlaces = (unsigned int*) malloc(block->nbJumps * sizeof(unsigned int));
		result->blocks[oneBlock]->nbJumps = block->nbJumps;

		for (int oneJump=0; oneJump<block->nbJumps; oneJump++){
			result->blocks[oneBlock]->jumpIds[oneJump] = block->jumpIds[oneJump];
#ifdef IR_SUCC
			result->blocks[oneBlock]->jumpPlaces[oneJump] = ((int) platform->placeOfInstr[block->jumpIds[oneJump]])+writePlace;
#else
			result->blocks[oneBlock]->jumpPlaces[oneJump] = incrementInBinaries*((int) platform->placeOfInstr[block->jumpIds[oneJump]])+writePlace;
#endif

		}

		for (int i=result->blocks[oneBlock]->vliwStartAddress;i<result->blocks[oneBlock]->vliwEndAddress;i++){
			Log::printf(LOG_SCHEDULE_PROC,"%d ", i);

			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+0]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+1]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+2]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+3]).c_str());


			if (platform->vliwInitialIssueWidth>4){
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+4]).c_str());
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+5]).c_str());
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+6]).c_str());
				Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+7]).c_str());
				i++;
			}
			Log::printf(LOG_SCHEDULE_PROC,"\n");
		}

		if ((isReturnBlock || isCallBlock) && readInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->jumpPlaces[result->blocks[oneBlock]->nbJumps-1] + 16*incrementInBinaries) != 0){

//			if (((readInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->vliwEndAddress - 16*incrementInBinaries)>>20) & 0x3f) == 33){
//				std::cerr << printDecodedInstr(readInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->vliwEndAddress - 16*incrementInBinaries))<<"\n";
//
//				writeInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->vliwEndAddress - 2*16*incrementInBinaries, 0);
//				writeInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->vliwEndAddress, readInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->vliwEndAddress - 2*16*incrementInBinaries));
//
//
//			}
//			else{
				//We need room for the reconf instruction, we add two lines at the end. TODO: this should be done only when reconf is activated
				writeInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->jumpPlaces[result->blocks[oneBlock]->nbJumps-1] + 16*2*incrementInBinaries, readInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->jumpPlaces[result->blocks[oneBlock]->nbJumps-1]));
				result->blocks[oneBlock]->jumpPlaces[result->blocks[oneBlock]->nbJumps-1]+=2;
				result->blocks[oneBlock]->vliwEndAddress += 2*incrementInBinaries;
				binaSize += 2*incrementInBinaries;
//			}

			if (readInt(platform->vliwBinaries, 16*result->blocks[oneBlock]->vliwEndAddress - 16*incrementInBinaries) != 0){
				fprintf(stderr, "Failede at moving...\n");
				exit(-1);
			}

		}

		//We handle speculation information if necessary
#ifndef __HW
#ifndef __SW
		for (int oneSpec = 0; oneSpec<4; oneSpec++){
			if (block->specAddr[oneSpec] != 0){
				platform->specInfo[8*block->specAddr[oneSpec]+2] = readInt(maskVal, oneSpec*16+12);
				platform->specInfo[8*block->specAddr[oneSpec]+3] = readInt(maskVal, oneSpec*16+8);
				platform->specInfo[8*block->specAddr[oneSpec]+4] = readInt(maskVal, oneSpec*16+4);
				platform->specInfo[8*block->specAddr[oneSpec]+5] = readInt(maskVal, oneSpec*16);

			}
		}
#endif
#endif

		if (isCallBlock){
			writeInt(platform->vliwBinaries, 16*(writePlace+binaSize), 0);
			writeInt(platform->vliwBinaries, 16*(writePlace+binaSize)+4, 0);
			writeInt(platform->vliwBinaries, 16*(writePlace+binaSize)+8, 0);
			writeInt(platform->vliwBinaries, 16*(writePlace+binaSize)+12, 0);
			binaSize += 1;

			if (getIssueWidth(platform->vliwInitialConfiguration)>4){
				writeInt(platform->vliwBinaries, 16*(writePlace + binaSize+1), 0);
				writeInt(platform->vliwBinaries, 16*(writePlace + binaSize+1)+4, 0);
				writeInt(platform->vliwBinaries, 16*(writePlace + binaSize+1)+8, 0);
				writeInt(platform->vliwBinaries, 16*(writePlace + binaSize+1)+12, 0);
				binaSize += 1;

			}
		}

		writePlace+=binaSize;

	}

	return result;

}

int rescheduleProcedure_commit(DBTPlateform *platform, IRProcedure *procedure,int writePlace, IRProcedure *scheduledProc){

	char issueWidth = getIssueWidth(procedure->configuration);
	char incrementInBinaries = (getIssueWidth(procedure->configuration)>4) ? 2 : 1;
	int *oldBlockStarts = (int*) malloc(procedure->nbBlock * sizeof(int));
	int originalWritePlace = writePlace;


	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		oldBlockStarts[oneBlock] = procedure->blocks[oneBlock]->vliwStartAddress;


		procedure->blocks[oneBlock]->vliwStartAddress = scheduledProc->blocks[oneBlock]->vliwStartAddress;
		procedure->blocks[oneBlock]->vliwEndAddress = scheduledProc->blocks[oneBlock]->vliwEndAddress;

		for (int oneJump = 0; oneJump<procedure->blocks[oneBlock]->nbJumps; oneJump++)
			procedure->blocks[oneBlock]->jumpPlaces[oneJump] = scheduledProc->blocks[oneBlock]->jumpPlaces[oneJump];


		if (procedure->blocks[oneBlock]->vliwEndAddress > writePlace)
			writePlace = procedure->blocks[oneBlock]->vliwEndAddress;
	}



	/******************************************************************************************
	 ******************************  Jump correction
	 ******************************************************************************************
	 * Now that all blocks have been scheduled correctly, we can go through all jump instruction and
	 * modify their destination to fit with the new block position.
	 ******************************************************************************************/

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];

		for (int oneJump = 0; oneJump<block->nbJumps; oneJump++){
			char jumpOpcode = getOpcode(block->instructions, block->jumpIds[oneJump]);

			if (jumpOpcode == VEX_BR || jumpOpcode == VEX_BRF || jumpOpcode == VEX_BGE || jumpOpcode == VEX_BLT || jumpOpcode == VEX_BGEU || jumpOpcode == VEX_BLTU){
				//Conditional block (br)
				int offset = (block->successors[oneJump]->vliwStartAddress - block->jumpPlaces[oneJump]);
				unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump]);
				writeInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump], (oldJump & 0xfff0007f) | ((offset & 0x1fff) << 7));
			}
			else if (jumpOpcode == VEX_GOTO){
					int dest = block->successors[oneJump]->vliwStartAddress;
					unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump]);
					writeInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump], (oldJump & 0xfc00007f) | ((dest & 0x7ffff) << 7));


			}
			else if (jumpOpcode != VEX_CALL && jumpOpcode != VEX_CALLR && jumpOpcode != VEX_GOTOR){

				int dest = block->successors[oneJump]->vliwStartAddress;
				unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump]);
				writeInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump], (oldJump & 0xfc00007f) | ((dest & 0x7ffff) << 7));


			}

		}

	}

	/******************************************************************************************
	 ******************************  Link to the newly generated code
	 ******************************************************************************************
	 * For each basic block in the procedure, we will insert jump instructions at the previous start
	 * address targeting to the new start address. This way, the execution will simply switch toward the newly
	 * generated binaries.
	 ******************************************************************************************/

	for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){
		IRBlock *block = procedure->blocks[oneBlock];
		int originalEntry = oldBlockStarts[oneBlock];

		if (block->vliwEndAddress - block->vliwStartAddress > 2){

			if (getIssueWidth(procedure->previousConfiguration) <= 4){
				if (platform->vexSimulator->PC == 4*originalEntry || platform->vexSimulator->PC == 4*(originalEntry+1))
					platform->vexSimulator->doStep(2);

				writeInt(platform->vliwBinaries, 16*originalEntry+0, assembleIInstruction_sw(VEX_GOTO, block->vliwStartAddress, 0));
				writeInt(platform->vliwBinaries, 16*originalEntry+4, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+8, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+12, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+16, getReconfigurationInstruction(procedure->configuration));
				writeInt(platform->vliwBinaries, 16*originalEntry+20, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+24, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+28, 0);

				for (int oneSpec = 0; oneSpec<4; oneSpec++){
					if (block->specAddr[oneSpec] != 0){
						writeInt(platform->vliwBinaries, 16*originalEntry+4, assembleMemoryInstruction_sw(VEX_SPEC_INIT, 0, 0, block->specAddr[oneSpec], 1, oneSpec));
						//TODO make it work for others specs
					}
				}
			}
			else{
				if (platform->vexSimulator->PC == 4*originalEntry || platform->vexSimulator->PC == 4*(originalEntry+2))
					platform->vexSimulator->doStep(2);

				writeInt(platform->vliwBinaries, 16*originalEntry+0, assembleIInstruction_sw(VEX_GOTO, block->vliwStartAddress, 0));
				writeInt(platform->vliwBinaries, 16*originalEntry+4, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+8, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+12, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+16, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+20, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+24, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+28, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+0, getReconfigurationInstruction(procedure->configuration));
				writeInt(platform->vliwBinaries, 16*originalEntry+32+4, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+8, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+12, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+16, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+20, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+24, 0);
				writeInt(platform->vliwBinaries, 16*originalEntry+32+28, 0);

			}

			if (getIssueWidth(platform->vliwInitialConfiguration) > 4){
				if (platform->vexSimulator->PC == 4*block->oldVliwStartAddress ||  platform->vexSimulator->PC == 4*(block->oldVliwStartAddress+2))
					platform->vexSimulator->doStep(2);

				writeInt(platform->vliwBinaries, 16*block->oldVliwStartAddress+0, assembleIInstruction_sw(VEX_GOTO, block->vliwStartAddress, 0));
				writeInt(platform->vliwBinaries, 16*block->oldVliwStartAddress+32, getReconfigurationInstruction(procedure->configuration));

			}
			else{
				if (platform->vexSimulator->PC == 4*block->oldVliwStartAddress ||  platform->vexSimulator->PC == 4*(block->oldVliwStartAddress+1))
					platform->vexSimulator->doStep(2);

				writeInt(platform->vliwBinaries, 16*block->oldVliwStartAddress+0, assembleIInstruction_sw(VEX_GOTO, block->vliwStartAddress, 0));
				writeInt(platform->vliwBinaries, 16*block->oldVliwStartAddress+16, getReconfigurationInstruction(procedure->configuration));

			}
		}

		bool isReturnBlock = false;
		bool isCallBlock = false;
		if (block->nbJumps > 0){
			char opcode = getOpcode(block->instructions, block->jumpIds[block->nbJumps-1]);
			isCallBlock = opcode == VEX_CALL || opcode == VEX_CALLR;
			isReturnBlock = opcode == VEX_GOTOR;
		}

		if (isReturnBlock){

			if (block->nbJumps == 1){
				if (readInt(platform->vliwBinaries, 16*block->jumpPlaces[block->nbJumps-1] +16*incrementInBinaries) == 0)
					writeInt(platform->vliwBinaries, 16*block->jumpPlaces[block->nbJumps-1] +16*incrementInBinaries, getReconfigurationInstruction(platform->vliwInitialConfiguration));
				else{
					Log::printf(LOG_ERROR,"Failing when inserting reconfs at the end of a procedure...\nExiting...");
					exit(-1);
				}
			}
			else{
				if (readInt(platform->vliwBinaries, 16*block->vliwEndAddress -16*incrementInBinaries) == 0)
					writeInt(platform->vliwBinaries, 16*block->vliwEndAddress -16*incrementInBinaries, getReconfigurationInstruction(platform->vliwInitialConfiguration));
				else{
					Log::printf(LOG_ERROR,"Failing when inserting reconfs at the end of a procedure...\nExiting...");
					exit(-1);
				}
			}

		}



		if (isCallBlock){
			char offsetSecondLine, offsetFirstLine;

			if (getIssueWidth(platform->vliwInitialConfiguration) > 4){
				offsetSecondLine = 2*16;
			}
			else{
				offsetSecondLine = 16;
			}

			if (issueWidth>4){
				offsetFirstLine = 2*16;
			}
			else{
				offsetFirstLine = 16;
			}

			writeInt(platform->vliwBinaries, 16*block->vliwEndAddress-offsetFirstLine, getReconfigurationInstruction(platform->vliwInitialConfiguration));
			writeInt(platform->vliwBinaries, 16*block->vliwEndAddress, getReconfigurationInstruction(procedure->configuration));

		}

	}

	//*************************************************************************
	//This is only for debug
	for (int i=originalWritePlace;i<writePlace;i++){
		Log::printf(LOG_SCHEDULE_PROC,"%d ", i);


		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+0]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+1]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+2]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+3]).c_str());


		if (platform->vliwInitialIssueWidth>4){
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+4]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+5]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+6]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+7]).c_str());
			i++;
		}
		Log::printf(LOG_SCHEDULE_PROC,"\n");
	}


	for (int i=originalWritePlace;i<writePlace;i++){



		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+0]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+1]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+2]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+3]).c_str());


		if (platform->vliwInitialIssueWidth>4){
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+4]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+5]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+6]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+7]).c_str());
			i++;
		}
		Log::printf(LOG_SCHEDULE_PROC,"\n");

		platform->vexSimulator->typeInstr[i-1+incrementInBinaries] = 2;
	}
	//*************************************************************************
	return writePlace;

}


void inPlaceBlockReschedule(IRBlock *block, DBTPlateform *platform, int writePlace){

	char isCurrentlyInBlock = (platform->vexSimulator->PC >= block->vliwStartAddress*4) &&
			(platform->vexSimulator->PC < block->vliwEndAddress*4);

	if (isCurrentlyInBlock){
		unsigned int instructionInEnd = readInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16);
		if (instructionInEnd == 0){
			writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0x2f);

			platform->vexSimulator->doStep(5 + block->vliwEndAddress - block->vliwStartAddress);
			writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0);
		}
		else if (readInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16+4) == 0){
			writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, 0x2f);
			writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16+4, instructionInEnd);

			platform->vexSimulator->doStep(5 + block->vliwEndAddress - block->vliwStartAddress);
			writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16, instructionInEnd);
			writeInt(platform->vliwBinaries, (block->vliwEndAddress-1)*16+4, 0);

		}
		else{
			Log::printf(LOG_ERROR, "In optimize basic block, execution is in the middle of a block and programm cannot stop it...\n exiting...\n");
			exit(-1);
		}
	}

	Log::printf(LOG_SCHEDULE_PROC,"*************************************************************************\n");
	Log::printf(LOG_SCHEDULE_PROC,"****                 In place block reschedule !                    *****\n");

	for (int i=block->vliwStartAddress-10;i<block->vliwEndAddress+10;i++){
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+0]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+1]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+2]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+3]).c_str());


		if (platform->vliwInitialIssueWidth>4){
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+4]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+5]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+6]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+7]).c_str());
			i++;
		}
		Log::printf(LOG_SCHEDULE_PROC,"\n");
	}

	Log::printf(LOG_SCHEDULE_PROC,"*************************************************************************\n");
	Log::printf(LOG_SCHEDULE_PROC,"*************************************************************************\n");

	//TODO WARNING We do not handle in place rescheduling when the configuration have been modified. We will have to handle that.
	char incrementInBinaries = (getIssueWidth(platform->vliwInitialConfiguration)>4) ? 2 : 1;

	//Preparation of required memories
	for (int oneFreeRegister = 33; oneFreeRegister<63; oneFreeRegister++)
		platform->freeRegisters[oneFreeRegister-33] = oneFreeRegister;


	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;
	//same for FP registers
	for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
		platform->placeOfRegisters[256+64+onePlaceOfRegister] = onePlaceOfRegister;

	//We move instructions into bytecode memory
	for (int oneBytecodeInstr = 0; oneBytecodeInstr<block->nbInstr; oneBytecodeInstr++){
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 0, block->instructions[4*oneBytecodeInstr + 0]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 4, block->instructions[4*oneBytecodeInstr + 1]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 8, block->instructions[4*oneBytecodeInstr + 2]);
		writeInt(platform->bytecode, 16*oneBytecodeInstr + 12, block->instructions[4*oneBytecodeInstr + 3]);
	}

	//This is only for debug
	if (platform->debugLevel > 1 || 1){
		Log::printf(LOG_SCHEDULE_PROC,"Block %x:\n", block->sourceStartAddress);
		for (int i=0; i<block->nbInstr; i++)
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12)).c_str());
	}


	//Calling scheduler
	int binaSize = irScheduler(platform, true, block->nbInstr, writePlace, 29, platform->vliwInitialConfiguration);
	binaSize = binaSize & 0xffff;


	if (block->vliwStartAddress + binaSize <= block->vliwEndAddress){

		memcpy(&platform->vliwBinaries[4*block->vliwStartAddress], &platform->vliwBinaries[4*writePlace], (binaSize)*4*sizeof(unsigned int));


		for (int i=block->vliwStartAddress+binaSize;i<block->vliwEndAddress;i++){
			writeInt(platform->vliwBinaries, i*16+0, 0);
			writeInt(platform->vliwBinaries, i*16+4, 0);
			writeInt(platform->vliwBinaries, i*16+8, 0);
			writeInt(platform->vliwBinaries, i*16+12, 0);

		}

		//We gather jump places
		for (int oneJump = 0; oneJump<block->nbJumps; oneJump++){
			#ifdef IR_SUCC
			block->jumpPlaces[oneJump] = ((int) platform->placeOfInstr[block->jumpIds[oneJump]])+basicBlockStart;
			#else
			block->jumpPlaces[oneJump] = incrementInBinaries*((int) platform->placeOfInstr[block->jumpIds[oneJump]])+block->vliwStartAddress;
			#endif
		}



		/*****************************************************************
		 *	Control Flow Correction
		 ************
		 * In this part we make the necessary work to make jumps correct.
		 * There are three different tasks:
		 * 	-> If the jump was relative jump then we have to correct the offset
		 * 	-> We have to write the (corrected) jump instruction because current scheduler just compute the place but do not place the instruction
		 * 	-> If the jump is a passthrough (eg. if the execution can go in the part after the schedule) we need to add a goto instruction
		 * 		to prevent the execution of a large area of nop instruction (which would remove the interest of the schedule
		 *
		 *****************************************************************/


		for (int oneJump = 0; oneJump<block->nbJumps; oneJump++){
			char jumpOpcode = getOpcode(block->instructions, block->jumpIds[oneJump]);

			if (jumpOpcode == VEX_BR || jumpOpcode == VEX_BRF || jumpOpcode == VEX_BGE || jumpOpcode == VEX_BLT || jumpOpcode == VEX_BGEU || jumpOpcode == VEX_BLTU){
				//Conditional block (br)
				int offset = (block->successors[oneJump]->vliwStartAddress - block->jumpPlaces[oneJump]);
				unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump]);
				writeInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump], (oldJump & 0xfff0007f) | ((offset & 0x1fff) << 7));
			}
			else if (jumpOpcode == VEX_GOTO){
					int dest = block->successors[oneJump]->vliwStartAddress;
					unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump]);
					writeInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump], (oldJump & 0xfc00007f) | ((dest & 0x7ffff) << 7));


			}
			else if (jumpOpcode != VEX_CALL && jumpOpcode != VEX_CALLR && jumpOpcode != VEX_GOTOR){

				int dest = block->successors[oneJump]->vliwStartAddress;
				unsigned int oldJump = readInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump]);
				writeInt(platform->vliwBinaries, 16*block->jumpPlaces[oneJump], (oldJump & 0xfc00007f) | ((dest & 0x7ffff) << 7));


			}

		}

		//Insertion of the new block with the goto instruction
		if (block->nbSucc == block->nbJumps + 1 && block->vliwStartAddress+binaSize+1*incrementInBinaries < block->vliwEndAddress){
			//We need to add a jump to correct the shortening of the block.

			unsigned int insertedJump = VEX_GOTO + (block->vliwEndAddress<<7);
			unsigned int placeOfNewJump = (block->vliwStartAddress+binaSize)*16;
			writeInt(platform->vliwBinaries, placeOfNewJump, insertedJump);
		}

	}
	else{
		Log::printf(LOG_SCHEDULE_BLOCK, "Schedule is dropped (%d cycles)\n", binaSize);
	}

	/*****************************************************************/
	// This only for debug
	Log::printf(LOG_SCHEDULE_PROC,"*************************************************************************\n");
	Log::printf(LOG_SCHEDULE_PROC,"****                 In place block reschedule !                    *****\n");

	for (int i=block->vliwStartAddress-10;i<block->vliwEndAddress+10;i++){
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+0]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+1]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+2]).c_str());
		Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+3]).c_str());


		if (platform->vliwInitialIssueWidth>4){
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+4]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+5]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+6]).c_str());
			Log::printf(LOG_SCHEDULE_PROC,"%s ", printDecodedInstr(platform->vliwBinaries[i*4+7]).c_str());
			i++;
		}
		Log::printf(LOG_SCHEDULE_PROC,"\n");
	}

	Log::printf(LOG_SCHEDULE_PROC,"*************************************************************************\n");
	Log::printf(LOG_SCHEDULE_PROC,"*************************************************************************\n");
	/*****************************************************************/

}

