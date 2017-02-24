/*
 * reconfigureVLIW.cpp
 *
 *  Created on: 9 févr. 2017
 *      Author: simon
 */


#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>
#include <lib/endianness.h>

#include <transformation/irGenerator.h>
#include <transformation/irScheduler.h>

void reconfigureVLIW(DBTPlateform *platform, IRProcedure *procedure){
	uint128 result[65536];
	int placeInResult = 0;

	fprintf(stderr, "*************************************************************************\n");
	fprintf(stderr, "Optimizing a procedure : \n");
	fprintf(stderr, "\n*****************\n");


	for (int oneBlock = 0; oneBlock < procedure->nbBlock; oneBlock++){

		IRBlock *block = procedure->blocks[oneBlock];

		int basicBlockStart = block->vliwStartAddress;
		int basicBlockEnd = block->vliwEndAddress;
		int blockSize;

		if (block->blockState >= IRBLOCK_STATE_SCHEDULED){

			memcpy(platform->bytecode, block->instructions, block->nbInstr*sizeof(uint128)); //TODO this is not correct...
			blockSize = block->nbInstr;

		}
		else{




			//We store old jump instruction. Its places is known from the basicBlockEnd value
			uint32 jumpInstruction = readInt(platform->vliwBinaries, (basicBlockEnd-2)*16 + 0);


			int globalVariableCounter = 288;

			for (int oneGlobalVariable = 0; oneGlobalVariable < 64; oneGlobalVariable++)
				platform->globalVariables[oneGlobalVariable] = 256 + oneGlobalVariable;

			int blockSize = basicBlockEnd - basicBlockStart - 1;

			blockSize = irGenerator(platform, basicBlockStart, blockSize, globalVariableCounter);

			fprintf(stderr, "Optimizing a block of size %d : \n", blockSize);


		}
		//First try to reduce the false dependencies
//		int registerUsing[64];
//		for (int oneReg = 0; oneReg<64; oneReg++){
//			registerUsing[oneReg] = -1;
//		}
//		for (int oneIRInstruction = 0; oneIRInstruction<blockSize; oneIRInstruction++){
//			uint128 instruction = platform->bytecode[oneIRInstruction];
//			short registerWritten = instruction.slc<9>(64+14) - 256;
//			unsigned char opcode = instruction.slc<7>(96+19);
//			fprintf(stderr, "opcode %x dest %d\n", opcode, registerWritten);
//
//			if (opcode != VEX_STB && opcode != VEX_STH && opcode != VEX_STW && registerWritten != 0){
//				if (registerUsing[registerWritten] != -1){
//					fprintf(stderr, "changed alloc on %d\n", oneIRInstruction);
//
//					platform->bytecode[oneIRInstruction][96+27] = 1;
//
//				}
//				registerUsing[registerWritten] = oneIRInstruction;
//
//			}
//		}
//
//		//We restore last write on each register
//		for (int oneReg = 0; oneReg<64; oneReg++){
//			if (registerUsing[oneReg] != -1){
//				fprintf(stderr, "restored alloc on %d\n", registerUsing[oneReg]);
//				platform->bytecode[registerUsing[oneReg]][96+27] = 0;
//			}
//		}


		for (int i=0; i<blockSize; i++)
			printBytecodeInstruction(i, readInt(platform->bytecode, i*16+0), readInt(platform->bytecode, i*16+4), readInt(platform->bytecode, i*16+8), readInt(platform->bytecode, i*16+12));

		//Preparation of required memories
		for (int oneFreeRegister = 36; oneFreeRegister<63; oneFreeRegister++)
			platform->freeRegisters[oneFreeRegister-36] = oneFreeRegister;

		for (int onePlaceOfRegister = 0; onePlaceOfRegister<64; onePlaceOfRegister++)
			platform->placeOfRegisters[256+onePlaceOfRegister] = onePlaceOfRegister;


		int binaSize = irScheduler(platform, 1,blockSize, basicBlockStart, 27, 4, 0x001e);
		binaSize = binaSize & 0xffff;

		for (int oneCycle = 0; oneCycle<(binaSize/2) + 1; oneCycle++){
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle].slc<32>(0));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle].slc<32>(32));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle].slc<32>(64));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle].slc<32>(96));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle+1].slc<32>(0));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle+1].slc<32>(32));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle+1].slc<32>(64));
			fprintf(stderr, " - ");
			std::cerr << printDecodedInstr(result[placeInResult + 2*oneCycle+1].slc<32>(96));
			fprintf(stderr, "\n");

		}

		fprintf(stderr, "Block is scheduled in %d cycles\n", binaSize);
	}



}

