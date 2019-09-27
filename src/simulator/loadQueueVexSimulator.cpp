#include <simulator/loadQueueVexSimulator.h>
#include <isa/vexISA.h>
#include <lib/log.h>
#include <stdio.h>


ac_int<64, true> LoadQueueVexSimulator::lddSpec(ac_int<64, false> addr, unsigned char specId){

	return 0;
}


ac_int<1, false> LoadQueueVexSimulator::checkSpec(ac_int<64, false> addr, unsigned char specId){
	ac_int<3, false> bankId = bankId >> 2;
	ac_int<2, false> indexInBank = specId & 0x3;

	for (int oneLoad = 0; oneLoad < LOAD_QUEUE_BANK_SIZE; oneLoad++){
		if (this->loadQueue_age[bankId][oneLoad] && this->loadQueue_addr[bankId][indexInBank] == addr.slc<24>(3)){
			fprintf(stderr, "Mispeculation !!\n");
			exit(-1);
		}
	}
}

ac_int<1, false> LoadQueueVexSimulator::stbSpec(ac_int<64, false> addr, ac_int<8, true> value, unsigned char specId){
	stb(addr, value);
	return checkSpec(addr, specId);
}
ac_int<1, false> LoadQueueVexSimulator::sthSpec(ac_int<64, false> addr, ac_int<16, true> value, unsigned char specId){
	sth(addr, value);
	return checkSpec(addr, specId);
}

ac_int<1, false> LoadQueueVexSimulator::stwSpec(ac_int<64, false> addr, ac_int<32, true> value, unsigned char specId){
	stw(addr, value);
	return checkSpec(addr, specId);
}

ac_int<1, false> LoadQueueVexSimulator::stdSpec(ac_int<64, false> addr, ac_int<64, true> value, unsigned char specId){
	std(addr, value);
	return checkSpec(addr, specId);
}


LoadQueueVexSimulator::LoadQueueVexSimulator(unsigned int *instructionMemory, unsigned int *specData) : VexSimulator(instructionMemory){this->speculationData = specData;};
LoadQueueVexSimulator::~LoadQueueVexSimulator(void){};


void LoadQueueVexSimulator::doMem(ExtoMem extoMem, MemtoWB *memtoWB){


	ac_int<1, false> rollback = 0;
	ac_int<1, false> clear = 0;
	ac_int<1, false> init = 0;

	//If instruction is speculative, we check the LSQ before
	if (((extoMem.opCode >> 4) == 0x1 || extoMem.opCode == VEX_FLW || extoMem.opCode == VEX_FSW) && extoMem.isSpec){

		memory_accesses++;

		if (extoMem.opCode == VEX_SPEC_RST){
			clear = 1;
		}
		else if (extoMem.opCode == VEX_SPEC_INIT){
			init = 1;

		}

		ac_int<128, false> mask = 0;
		ac_int<64, false> rollbackPoint;

		partitionnedLoadQueue(extoMem.pc, extoMem.result, extoMem.funct, clear, &rollback,
				this->speculationData, init, extoMem.result, &mask, &rollbackPoint);


		if (rollback && !extoMem.isRollback && (extoMem.opCode >> 3) != (VEX_LDD>>3) && extoMem.opCode != VEX_FLW){
	/*		fprintf(stderr, "In LQ vex simulator, system detected that we had to rollback...\n");
			fprintf(stderr, "Mask to apply would be %llx starting from %lld\n", (unsigned long long int) mask.slc<64>(0), (unsigned long long int) rollbackPoint);
			fprintf(stderr, "endrollback is %lld\n", (unsigned long long int) endRollback); */

			this->rollback = 1;
			this->rollBackPoint = rollbackPoint;
			this->endRollback = extoMem.pc;
			this->mask = mask;
		}


	}


	VexSimulator::doMem(extoMem, memtoWB);

}

void LoadQueueVexSimulator::doMemNoMem(ExtoMem extoMem, MemtoWB *memtoWB){


	ac_int<1, false> rollback = 0;
	ac_int<1, false> clear = 0;
	ac_int<1, false> init = 0;

	//If instruction is speculative, we check the LSQ before
	if (((extoMem.opCode >> 4) == 0x1 || extoMem.opCode == VEX_FLW || extoMem.opCode == VEX_FSW) && extoMem.isSpec){

		if (extoMem.opCode == VEX_SPEC_RST){
			clear = 1;
		}
		else if (extoMem.opCode == VEX_SPEC_INIT){
			init = 1;

		}

		ac_int<128, false> mask = 0;
		ac_int<64, false> rollbackPoint;

		partitionnedLoadQueue(extoMem.pc, extoMem.result, extoMem.funct, clear, &rollback,
				this->speculationData, init, extoMem.result, &mask, &rollbackPoint);




	}


	VexSimulator::doMemNoMem(extoMem, memtoWB);

}


int LoadQueueVexSimulator::doStep(){


	doWB(memtoWB2);
	doWB(memtoWB3);
	doWB(memtoWB7);
	doWB(memtoWB8);


	///////////////////////////////////////////////////////
	//													 //
	//                         EX                        //
	//													 //
	///////////////////////////////////////////////////////

	doEx(dctoEx1, &extoMem1);
	doExMult(dctoEx2, &extoMem2);
	doExMult(dctoEx3, &extoMem3);
	doEx(dctoEx4, &extoMem4);
	doEx(dctoEx5, &extoMem5);
	doEx(dctoEx6, &extoMem6);
	doEx(dctoEx7, &extoMem7);
	doExMult(dctoEx8, &extoMem8);



	///////////////////////////////////////////////////////
	//													 //
	//                       M                           //
	//													 //
	///////////////////////////////////////////////////////







	doMemNoMem(extoMem1, &memtoWB1);
	doMemNoMem(extoMem3, &memtoWB3);
	doMemNoMem(extoMem4, &memtoWB4);
	doMemNoMem(extoMem5, &memtoWB5);
	doMemNoMem(extoMem6, &memtoWB6);
	doMemNoMem(extoMem8, &memtoWB8);

#ifdef __CATAPULT
	doMem(extoMem2, &memtoWB2, memory0, memory1, memory2, memory3, memory4, memory5, memory6, memory7);
	doMem(extoMem7, &memtoWB7, memory0, memory1, memory2, memory3, memory4, memory5, memory6, memory7);

#else
	doMem(extoMem2, &memtoWB2);
	doMem(extoMem7, &memtoWB7);
#endif

	//		doMem(extoMem6, &memtoWB6, DATA0, DATA1, DATA2, DATA3);



	///////////////////////////////////////////////////////
	//													 //
	//                       WB                          //
	//  												 //
	///////////////////////////////////////////////////////

	doWB(memtoWB1);
	doWB(memtoWB4);
	doWB(memtoWB5);
	doWB(memtoWB6);


	///////////////////////////////////////////////////////
	//													 //
	//                       DC                          //
	//													 //
	///////////////////////////////////////////////////////

	NEXT_PC = PC+4;
	if (this->issueWidth > 4)
		NEXT_PC += 4;

	doDCBr(ftoDC1, &dctoEx1);

#ifdef __CATAPULT
	doDCMem(ftoDC2, &dctoEx2, memory0, memory1, memory2, memory3, memory4, memory5, memory6, memory7);
#else
	doDCMem(ftoDC2, &dctoEx2);
#endif
	doDC(ftoDC3, &dctoEx3);
	doDC(ftoDC4, &dctoEx4);

	doDC(ftoDC5, &dctoEx5);
	doDC(ftoDC6, &dctoEx6);

#ifdef __CATAPULT
	doDCMem(ftoDC7, &dctoEx7, memory0, memory1, memory2, memory3, memory4, memory5, memory6, memory7);
#else
	doDCMem(ftoDC7, &dctoEx7);
#endif
	doDC(ftoDC8, &dctoEx8);

	ac_int<7, false> OP1 = ftoDC1.instruction.slc<7>(0);

	// If the operation code is 0x2f then the processor stops
	if(OP1 == 0x2F){
		stop = 1;
		NEXT_PC = PC;
	}
	// If the operation code is 0x2f then the processor stops
	if(stop == 1){


		return PC;
	}

	///////////////////////////////////////////////////////
	//                       F                           //
	///////////////////////////////////////////////////////


	// Retrieving new instruction

	ac_int<64, false> secondLoadAddress = (PC>>2)+1;


	ac_int<32, false> instructions[8];
	instructions[0] = RI[(int) PC+0];
	instructions[1] = RI[(int) PC+1];
	instructions[2] = RI[(int) PC+2];
	instructions[3] = RI[(int) PC+3];
	instructions[4] = RI[(int) PC+4];
	instructions[5] = RI[(int) PC+5];
	instructions[6] = RI[(int) PC+6];
	instructions[7] = RI[(int) PC+7];

	ac_int<32, false> nopInstr = 0;

	// Redirect instructions to thier own ways
	#ifndef __CATAPULT

	ftoDC1.instruction = (this->mask[63] | !this->rollback) ? instructions[0] : nopInstr;
	ftoDC2.instruction = (this->unitActivation[1] & (this->mask[62] | !this->rollback)) ? instructions[1] : nopInstr;
	ftoDC3.instruction = (this->unitActivation[1] & (this->mask[61] | !this->rollback)) ? instructions[2] : nopInstr;
	ftoDC4.instruction = (this->unitActivation[1] & (this->mask[60] | !this->rollback)) ? instructions[3] : nopInstr;

	ftoDC5.instruction = (this->unitActivation[4] & (this->mask[59] | !this->rollback)) ? instructions[4] : nopInstr;
	ftoDC6.instruction = (this->unitActivation[5] & (this->mask[58] | !this->rollback)) ? (this->muxValues[0] ? instructions[1] : instructions[5]) : nopInstr;
	ftoDC7.instruction = (this->unitActivation[6] & (this->mask[57] | !this->rollback)) ? (this->muxValues[1] ? instructions[2] : instructions[6]) : nopInstr;
	ftoDC8.instruction = (this->unitActivation[7] & (this->mask[56] | !this->rollback)) ? (this->muxValues[2] ? instructions[3] : instructions[7]) : nopInstr;


	ftoDC1.pc = PC;
	ftoDC2.pc = PC;
	ftoDC3.pc = PC;
	ftoDC4.pc = PC;
	ftoDC5.pc = PC;
	ftoDC6.pc = PC;
	ftoDC7.pc = PC;
	ftoDC8.pc = PC;

	ftoDC1.isRollback = this->rollback;
	ftoDC2.isRollback = this->rollback;
	ftoDC3.isRollback = this->rollback;
	ftoDC4.isRollback = this->rollback;
	ftoDC5.isRollback = this->rollback;
	ftoDC6.isRollback = this->rollback;
	ftoDC7.isRollback = this->rollback;
	ftoDC8.isRollback = this->rollback;

	//We increment IPc counters
	if (ftoDC1.instruction != 0)
		nbInstr++;
	if (ftoDC2.instruction != 0)
		nbInstr++;
	if (ftoDC3.instruction != 0)
		nbInstr++;
	if (ftoDC4.instruction != 0)
		nbInstr++;
	if (ftoDC5.instruction != 0)
		nbInstr++;
	if (ftoDC6.instruction != 0)
		nbInstr++;
	if (ftoDC7.instruction != 0)
		nbInstr++;
	if (ftoDC8.instruction != 0)
		nbInstr++;

	#else
	ftoDC1.instruction = instructions[0];
	ftoDC2.instruction = instructions[1];
	ftoDC3.instruction = instructions[2];
	ftoDC4.instruction = instructions[3];

	#endif


	nbCycleType[typeInstr[(int) PC/4]]++;


	int pcValueForDebug = PC;
	// Next instruction

	if (this->rollback){
		if (rollBackPoint != 0){
			this->PC = rollBackPoint;
			rollBackPoint = 0;

			while (mask != 0 && this->mask.slc<4>(60) == 0){
				this->mask = this->mask<<(issueWidth>4?8:4);
			}

			this->ftoDC1.instruction = 0;
			this->ftoDC2.instruction = 0;
			this->ftoDC3.instruction = 0;
			this->ftoDC4.instruction = 0;
			this->ftoDC5.instruction = 0;
			this->ftoDC6.instruction = 0;
			this->ftoDC7.instruction = 0;
			this->ftoDC8.instruction = 0;

			this->dctoEx1.opCode = 0;
			this->dctoEx2.opCode = 0;
			this->dctoEx3.opCode = 0;
			this->dctoEx4.opCode = 0;
			this->dctoEx5.opCode = 0;
			this->dctoEx6.opCode = 0;
			this->dctoEx7.opCode = 0;
			this->dctoEx8.opCode = 0;

			this->extoMem1.opCode = 0;
			this->extoMem2.opCode = 0;
			this->extoMem3.opCode = 0;
			this->extoMem4.opCode = 0;
			this->extoMem5.opCode = 0;
			this->extoMem6.opCode = 0;
			this->extoMem7.opCode = 0;
			this->extoMem8.opCode = 0;

			this->memtoWB1.WBena = 0;
			this->memtoWB2.WBena = 0;
			this->memtoWB3.WBena = 0;
			this->memtoWB4.WBena = 0;
			this->memtoWB5.WBena = 0;
			this->memtoWB6.WBena = 0;
			this->memtoWB7.WBena = 0;
			this->memtoWB8.WBena = 0;

		}
		else if ((this->issueWidth <= 4 && NEXT_PC != PC + 4) || (this->issueWidth > 4 && NEXT_PC != PC + 8) || PC == endRollback){
			rollback = 0;
			mask = 0;
			PC = NEXT_PC;

		}
		else{
			mask = mask<<(issueWidth>4?8:4);
			PC = NEXT_PC;

		}

	}
	else{
		PC = NEXT_PC;
	}
	cycle++;


	// DISPLAY



#ifndef __CATAPULT

	if (debugLevel >= 1){

		std::cerr << std::to_string(cycle) + ";" + std::to_string(pcValueForDebug) + ";";
//		if (this->unitActivation[0])
//			std::cerr << "\033[1;31m" << printDecodedInstr(ftoDC1.instruction) << "\033[0m;";
//		if (this->unitActivation[1])
//			std::cerr << "\033[1;35m" << printDecodedInstr(ftoDC2.instruction) << "\033[0m;";
//		if (this->unitActivation[2])
//			std::cerr << "\033[1;34m" << printDecodedInstr(ftoDC3.instruction) << "\033[0m;";
//		if (this->unitActivation[3])
//			std::cerr << "\033[1;33m" << printDecodedInstr(ftoDC4.instruction) << "\033[0m;";
//		if (this->unitActivation[4])
//			std::cerr << "\033[1;33m" << printDecodedInstr(ftoDC5.instruction) << "\033[0m;";
//		if (this->unitActivation[5])
//			std::cerr << "\033[1;33m" << printDecodedInstr(ftoDC6.instruction) << "\033[0m;";
//		if (this->unitActivation[6])
//			std::cerr << "\033[1;32m" << printDecodedInstr(ftoDC7.instruction) << "\033[0m;";
//		if (this->unitActivation[7])
//			std::cerr << "\033[1;34m" << printDecodedInstr(ftoDC8.instruction) << "\033[0m;";

		if (this->unitActivation[0])
			std::cerr << printDecodedInstr(ftoDC1.instruction);
		if (this->unitActivation[1])
			std::cerr << printDecodedInstr(ftoDC2.instruction);
		if (this->unitActivation[2])
			std::cerr << printDecodedInstr(ftoDC3.instruction);
		if (this->unitActivation[3])
			std::cerr << printDecodedInstr(ftoDC4.instruction);
		if (this->unitActivation[4])
			std::cerr << printDecodedInstr(ftoDC5.instruction);
		if (this->unitActivation[5])
			std::cerr << printDecodedInstr(ftoDC6.instruction);
		if (this->unitActivation[6])
			std::cerr << printDecodedInstr(ftoDC7.instruction);
		if (this->unitActivation[7])
			std::cerr << printDecodedInstr(ftoDC8.instruction);

		fprintf(stderr, ";");



		for (int oneRegister =0; oneRegister<38; oneRegister++){
			fprintf(stderr, "%lx;", (long) REG[oneRegister]);
		}
		fprintf(stderr, ";;%lx;", (long) REG[41]);
		fprintf(stderr, ";;%lx;", (long) REG[42]);

		fprintf(stderr, "\n");

	}


#endif

	return 0;
}

