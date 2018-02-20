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
#include <lib/ac_int.h>
/****************************************************************************
 * Header file for VEX Simulator
 *************************************
 * This file is the header file for VEX simulator.
 * It has the specificity to be compatible with Catapult HLS tool so that
 * hardware can be generated from its functional description.
 *
 * Vex processor is a dynamically adaptable VLIW core. It can run at different
 * configurations, ranging from a 2-issue up to a 8-issue VLIW core.
 * It can also modify the size of its register file between 32 and 64 registers.
 * This reconfiguration is triggered by the custom instruction RECONFFS.
 *
 * The simulator in itself extends our Generic simulator which give access to initialization function
 * and methods to solve system calls (open/close/read/write file).
 *
 *****************************************************************************/


/****************************************************************************
 * Definition of structs for the different pipeline stages
 ***************************************************************************/

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
	float floatRes, floatValueA, floatValueB, floatValueC;
	ac_int<5, false> funct;

};

struct ExtoMem {
	ac_int<64, true> result;	//Result of the EX stage
	ac_int<64, true> datac;		//Data to be stored in memory (if needed)
	ac_int<6, false> dest;		//Register to be written at WB stage
	ac_int<1, false> WBena;		//Is a WB is needed ?
	ac_int<7, false> opCode;	//OpCode of the operation
	ac_int<64, true> memValue; //Second data, from register file or immediate value
	float floatRes;
	ac_int<1, false> isFloat;

};

struct MemtoWB {
	ac_int<64, true> result;	//Result to be written back
	ac_int<6, false> dest;		//Register to be written at WB stage
	ac_int<1, false> WBena;		//Is a WB is needed ?
	float floatRes;
	ac_int<1, false> isFloat;
};


/****************************************************************************
 * Definition of the class for VEXSimulator
 ***************************************************************************/


#ifndef __CATAPULT

class VexSimulator : public GenericSimulator {
	public:

	int typeInstr[230000];
	int nbCycleType[4] = {0,0,0,0};

	//Instruction memory is a 128-bit memory
	unsigned int *RI;

	//Definition of PC and NEXT_PC
	ac_int<64, false> PC, NEXT_PC;

	//Small memory containing profile information and accessed using dedicated instructions
	ac_int<8, false> profileResult[8192];

	//Definition of the dynamic configuration of the VLIW
	ac_int<4, false> issueWidth;
	ac_int<1, false> unitActivation[8];
	ac_int<1, false> muxValues[3];

	//Speculation
	ac_int<1, false> enable;

	//Tools for statistics
	uint64_t nbInstr, lastNbInstr;
	uint64_t lastNbCycle;
	uint64_t lastReconf;
	char currentConfig;
	uint64_t timeInConfig[32];

	//Object constructor
	VexSimulator(unsigned int *instructionMemory): GenericSimulator() {
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

	//Objet destructor
	~VexSimulator(void) {
		memory.clear();
	}


	//Tools for initialization/execution
	void initializeDataMemory(unsigned char* content, unsigned int size, unsigned int start);
	int initializeRun(int mainPc, int argc, char* argv[]);
	virtual int doStep();
	int doStep(int nbStep);

	//Statistics
	float getAverageIPC();



	protected:

	//Different parts of the execution
	void doWB(struct MemtoWB memtoWB);
	void doMemNoMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);
	void doMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);
	void doEx(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doExMult(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doExMem(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	virtual void doDC(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
	void doDCMem(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
	virtual void doDCBr(struct FtoDC ftoDC, struct DCtoEx *dctoEx);


	struct MemtoWB memtoWB1;	struct MemtoWB memtoWB2;	struct MemtoWB memtoWB3;	struct MemtoWB memtoWB4;	struct MemtoWB memtoWB5; 	struct MemtoWB memtoWB6;	struct MemtoWB memtoWB7;	struct MemtoWB memtoWB8;
	struct ExtoMem extoMem1;	struct ExtoMem extoMem2;	struct ExtoMem extoMem3;	struct ExtoMem extoMem4;	struct ExtoMem extoMem5;	struct ExtoMem extoMem6;	struct ExtoMem extoMem7;	struct ExtoMem extoMem8;
	struct DCtoEx dctoEx1; struct DCtoEx dctoEx2;	struct DCtoEx dctoEx3;	struct DCtoEx dctoEx4;	struct DCtoEx dctoEx5;	struct DCtoEx dctoEx6;	struct DCtoEx dctoEx7;	struct DCtoEx dctoEx8;
	struct FtoDC ftoDC1;	struct FtoDC ftoDC2;	struct FtoDC ftoDC3;	struct FtoDC ftoDC4;	struct FtoDC ftoDC5;	struct FtoDC ftoDC6;	struct FtoDC ftoDC7;	struct FtoDC ftoDC8;


};


#endif
#endif


#endif /* INCLUDES_VEXSIMULATOR_H_ */
