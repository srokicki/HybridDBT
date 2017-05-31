/*
 * vexSimulator.h
 *
 *  Created on: 18 août 2016
 *      Author: simon
 */

#ifndef __VEXSIMULATOR
#define __VEXSIMULATOR

#ifndef __NIOS

#ifndef __CATAPULT
#include <map>
#endif

#include <types.h>
#include <isa/vexISA.h>
#include <simulator/genericSimulator.h>

struct FtoDC {
	ac_int<64, false> instruction; //Instruction to execute
};

struct DCtoEx {
	ac_int<64, true> dataa; //First data from register file
	ac_int<64, true> datab; //Second data, from register file or immediate value
	ac_int<64, true> datac; //Third data used only for store instruction and corresponding to rb
	ac_int<6, false> dest;  //Register to be written
	ac_int<7, false> opCode;//OpCode of the instruction
	ac_int<64, true> memValue; //Second data, from register file or immediate value

};

struct ExtoMem {
	ac_int<64, true> result;	//Result of the EX stage
	ac_int<64, true> datac;		//Data to be stored in memory (if needed)
	ac_int<6, false> dest;		//Register to be written at WB stage
	ac_int<1, false> WBena;		//Is a WB is needed ?
	ac_int<7, false> opCode;	//OpCode of the operation
	ac_int<64, true> memValue; //Second data, from register file or immediate value
};

struct MemtoWB {
	ac_int<64, true> result;	//Result to be written back
	ac_int<6, false> dest;		//Register to be written at WB stage
	ac_int<1, false> WBena;		//Is a WB is needed ?
};

#ifndef __CATAPULT

class VexSimulator : public GenericSimulator {
	public:

	ac_int<128, false> *RI;

	int cycle = 0;
	ac_int<64, false> PC, NEXT_PC;
	ac_int<4, false> issueWidth;
	ac_int<1, false> cond;
	ac_int<1, false> unitActivation[8];
	ac_int<8, false> profileResult[64];

	//Tools for printing average IPC
	uint64_t nbInstr;
	uint64_t lastNbCycle;
	float getAverageIPC();


	VexSimulator(ac_int<128, false> *instructionMemory): GenericSimulator() {
		cycle=0;
		issueWidth = 4;
		unitActivation[0] = 1;
		unitActivation[1] = 1;
		unitActivation[2] = 1;
		unitActivation[3] = 1;
		unitActivation[4] = 0;
		unitActivation[5] = 0;
		unitActivation[6] = 0;
		unitActivation[7] = 0;

		this->RI = instructionMemory;

	};

	~VexSimulator(void) {
		memory.clear();
	}

	void initializeDataMemory(unsigned char* content, unsigned int size, unsigned int start);
	void initializeDataMemory(ac_int<64, false>* content, unsigned int size, unsigned int start);


	int initializeRun(int mainPc, int argc, char* argv[]);
	int doStep();
	int doStep(int nbStep);


	void doWB(struct MemtoWB memtoWB);
	void doMemNoMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);
	void doMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);
	void doEx(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doExMult(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doExMem(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doDC(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
	void doDCMem(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
	void doDCBr(struct FtoDC ftoDC, struct DCtoEx *dctoEx);


	struct MemtoWB memtoWB1;	struct MemtoWB memtoWB2;	struct MemtoWB memtoWB3;	struct MemtoWB memtoWB4;	struct MemtoWB memtoWB5; 	struct MemtoWB memtoWB6;	struct MemtoWB memtoWB7;	struct MemtoWB memtoWB8;
	struct ExtoMem extoMem1;	struct ExtoMem extoMem2;	struct ExtoMem extoMem3;	struct ExtoMem extoMem4;	struct ExtoMem extoMem5;	struct ExtoMem extoMem6;	struct ExtoMem extoMem7;	struct ExtoMem extoMem8;
	struct DCtoEx dctoEx1; struct DCtoEx dctoEx2;	struct DCtoEx dctoEx3;	struct DCtoEx dctoEx4;	struct DCtoEx dctoEx5;	struct DCtoEx dctoEx6;	struct DCtoEx dctoEx7;	struct DCtoEx dctoEx8;
	struct FtoDC ftoDC1;	struct FtoDC ftoDC2;	struct FtoDC ftoDC3;	struct FtoDC ftoDC4;	struct FtoDC ftoDC5;	struct FtoDC ftoDC6;	struct FtoDC ftoDC7;	struct FtoDC ftoDC8;

};


#endif
#endif


#endif /* INCLUDES_VEXSIMULATOR_H_ */
