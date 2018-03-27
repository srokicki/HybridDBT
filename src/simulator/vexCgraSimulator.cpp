#include <simulator/vexCgraSimulator.h>
#include <lib/log.h>
#include <isa/cgraIsa.h>

VexCgraSimulator::VexCgraSimulator(unsigned int *bin)
	: VexSimulator(bin), cgraStall(0), cgraSimulator(&this->memory, this->REG), cgraRunning(false)
{
}

int VexCgraSimulator::doStep(){

	cgraRunning = false;
	if (cgraStall == 5){
		//cgraSimulator.print();
		cgraRunning = cgraSimulator.doStep();
	}

	if (!cgraRunning && cgraStall == 5)
		cgraStall = 0;

	if (cgraStall > 0 && cgraStall < 5)
	{
		cgraStall++;
	}

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

	doMem(extoMem2, &memtoWB2);
	doMem(extoMem7, &memtoWB7);

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

	doDCMem(ftoDC2, &dctoEx2);

	doDC(ftoDC3, &dctoEx3);
	doDC(ftoDC4, &dctoEx4);

	doDC(ftoDC5, &dctoEx5);
	doDC(ftoDC6, &dctoEx6);

	doDCMem(ftoDC7, &dctoEx7);

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

	ftoDC1.instruction = !cgraStall ? instructions[0] : nopInstr;
	ftoDC2.instruction = this->unitActivation[1] && !cgraStall ? instructions[1] : nopInstr;
	ftoDC3.instruction = this->unitActivation[2] && !cgraStall ? instructions[2] : nopInstr;
	ftoDC4.instruction = this->unitActivation[3] && !cgraStall ? instructions[3] : nopInstr;

	ftoDC5.instruction = this->unitActivation[4] && !cgraStall ? instructions[4] : nopInstr;
	ftoDC6.instruction = this->unitActivation[5] && !cgraStall ? (this->muxValues[0] ? instructions[1] : instructions[5]) : nopInstr;
	ftoDC7.instruction = this->unitActivation[6] && !cgraStall ? (this->muxValues[1] ? instructions[2] : instructions[6]) : nopInstr;
	ftoDC8.instruction = this->unitActivation[7] && !cgraStall ? (this->muxValues[2] ? instructions[3] : instructions[7]) : nopInstr;

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

	nbCycleType[typeInstr[(int) PC/4]]++;

	int pcValueForDebug = PC;

	if (!cgraStall)
	{
		// Next instruction
		PC = NEXT_PC;
		cycle++;
	}

	// DISPLAY

	if (0){//cgraStall > 0){//debugLevel >= 1/* || currentConf != -1*/){

		std::cerr << std::to_string(cycle) + ";" + std::to_string(pcValueForDebug) + ";";
		if (this->unitActivation[0])
			std::cerr << "\033[1;31m" << printDecodedInstr(ftoDC1.instruction) << "\033[0m;";
		if (this->unitActivation[1])
			std::cerr << "\033[1;35m" << printDecodedInstr(ftoDC2.instruction) << "\033[0m;";
		if (this->unitActivation[2])
			std::cerr << "\033[1;34m" << printDecodedInstr(ftoDC3.instruction) << "\033[0m;";
		if (this->unitActivation[3])
			std::cerr << "\033[1;33m" << printDecodedInstr(ftoDC4.instruction) << "\033[0m;";
		if (this->unitActivation[4])
			std::cerr << "\033[1;33m" << printDecodedInstr(ftoDC5.instruction) << "\033[0m;";
		if (this->unitActivation[5])
			std::cerr << "\033[1;33m" << printDecodedInstr(ftoDC6.instruction) << "\033[0m;";
		if (this->unitActivation[6])
			std::cerr << "\033[1;32m" << printDecodedInstr(ftoDC7.instruction) << "\033[0m;";
		if (this->unitActivation[7])
			std::cerr << "\033[1;34m" << printDecodedInstr(ftoDC8.instruction) << "\033[0m;";

//		if (this->unitActivation[0])
//			std::cerr << printDecodedInstr(ftoDC1.instruction);
//		if (this->unitActivation[1])
//			std::cerr << printDecodedInstr(ftoDC2.instruction);
//		if (this->unitActivation[2])
//			std::cerr << printDecodedInstr(ftoDC3.instruction);
//		if (this->unitActivation[3])
//			std::cerr << printDecodedInstr(ftoDC4.instruction);
//		if (this->unitActivation[4])
//			std::cerr << printDecodedInstr(ftoDC5.instruction);
//		if (this->unitActivation[5])
//			std::cerr << printDecodedInstr(ftoDC6.instruction);
//		if (this->unitActivation[6])
//			std::cerr << printDecodedInstr(ftoDC7.instruction);
//		if (this->unitActivation[7])
//			std::cerr << printDecodedInstr(ftoDC8.instruction);

		fprintf(stderr, ";");



		for (int oneRegister =0; oneRegister<36; oneRegister++){
			fprintf(stderr, "%lx;", (long) REG[oneRegister]);
		}
		fprintf(stderr, ";;%lx;", (long) REG[63]);

		fprintf(stderr, "\n");

		debugLevel = 0;
	}

	//Log::out(0) << "cgraStall = " << cgraStall << "     cgraCycle = " << cgraCycles << "       PC = " << PC << "\n";
	return 0;
}

void VexCgraSimulator::doDC(FtoDC ftoDC, DCtoEx *dctoEx)
{
	ac_int<7, false> OP = ftoDC.instruction.slc<7>(0);
	ac_int<19, false> IMM19_u = ftoDC.instruction.slc<19>(7);

	if (OP == VEX_CGRA)
	{
		cgraSimulator.start(IMM19_u);
		cgraStall = 1;
		debugLevel = 2;
		return;
	}

	VexSimulator::doDC(ftoDC, dctoEx);
}

void VexCgraSimulator::doDCBr(FtoDC ftoDC, DCtoEx *dctoEx)
{
	ac_int<7, false> OP = ftoDC.instruction.slc<7>(0);
	ac_int<19, false> IMM19_u = ftoDC.instruction.slc<19>(7);

	if (OP == VEX_CGRA)
	{
		cgraSimulator.start(IMM19_u);
		cgraStall = 1;
		debugLevel = 2;
		return;
	}

	VexSimulator::doDCBr(ftoDC, dctoEx);
}
