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
	ac_int<128, false> result[65536];
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

			memcpy(platform->bytecode, block->instructions, block->nbInstr*sizeof(uint128));
			blockSize = block->nbInstr;

		}
		else{




			//We store old jump instruction. Its places is known from the basicBlockEnd value
			uint32 jumpInstruction = readInt(platform->vliwBinaries, (basicBlockEnd-2)*16 + 0);


			int globalVariableCounter = 288;
			unsigned long long registersUsage[256];

			int32 globalVariables[64] = {256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,
					279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,
					307,308,309,310,311,312,313,314,315,316,317,318,319
			};

			blockSize = basicBlockEnd - basicBlockStart - 1;

			uint64 local_registersUsage[1];

			blockSize = irGenerator_hw(platform->vliwBinaries,basicBlockStart, blockSize, platform->bytecode, globalVariables, local_registersUsage, globalVariableCounter);

			fprintf(stderr, "Optimizing a block of size %d : \n", blockSize);


		}
		//First try to reduce the false dependencies
		int registerUsing[64];
		for (int oneReg = 0; oneReg<64; oneReg++){
			registerUsing[oneReg] = -1;
		}
		for (int oneIRInstruction = 0; oneIRInstruction<blockSize; oneIRInstruction++){
			uint128 instruction = platform->bytecode[oneIRInstruction];
			short registerWritten = instruction.slc<9>(64+14) - 256;
			unsigned char opcode = instruction.slc<7>(96+19);
			fprintf(stderr, "opcode %x dest %d\n", opcode, registerWritten);

			if (opcode != VEX_STB && opcode != VEX_STH && opcode != VEX_STW && registerWritten != 0){
				if (registerUsing[registerWritten] != -1){
					fprintf(stderr, "changed alloc on %d\n", oneIRInstruction);

					platform->bytecode[oneIRInstruction][96+27] = 1;

				}
				registerUsing[registerWritten] = oneIRInstruction;

			}
		}

		//We restore last write on each register
		for (int oneReg = 0; oneReg<64; oneReg++){
			if (registerUsing[oneReg] != -1){
				fprintf(stderr, "restored alloc on %d\n", registerUsing[oneReg]);
				platform->bytecode[registerUsing[oneReg]][96+27] = 0;
			}
		}


		for (int i=0; i<blockSize; i++)
			printBytecodeInstruction(i,platform->bytecode[i]);

		ac_int<6, 0> placeOfRegisters[512] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
				40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
		ac_int<6, 0> freeRegisters[64] = {36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
		ac_int<32, false> placeOfInstr[256];

		int binaSize = scheduling(1,blockSize, platform->bytecode, result,placeInResult, placeOfRegisters, 27, freeRegisters, 5, 0x035c, placeOfInstr);
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

