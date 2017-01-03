/*
 * vexSimulator.h
 *
 *  Created on: 18 août 2016
 *      Author: simon
 */

#ifndef __VEXSIMULATOR
#define __VEXSIMULATOR

#ifndef __VLIW

#include <map>
#include <lib/ac_int.h>
#include <isa/vexISA.h>





class VexSimulator {
	public:

	int debugLevel = 0;


	std::map<unsigned int, ac_int<8, false>> memory;
	int cycle = 0;
	int PC, NEXT_PC;
	ac_int<32, false> REG[64];

	VexSimulator(void): memory(){
		cycle=0;
		for (int oneReg = 0; oneReg < 64; oneReg++){
			REG[oneReg] = 0;
		}
		REG[29] = 0;
	};

	~VexSimulator(void) {
		memory.clear();
	}

	void initializeDataMemory(unsigned char* content, unsigned int size, unsigned int start);
	void initializeDataMemory(ac_int<32, false>* content, unsigned int size, unsigned int start);


	void initializeCodeMemory(unsigned char* content, unsigned int size, unsigned int start);
	void initializeCodeMemory(ac_int<128, false>* content, unsigned int size, unsigned int start);
	int run(int mainPc);

	private:
	void stb(unsigned int addr, ac_int<8, false> value);
	void sth(unsigned int addr, ac_int<16, false> value);
	void stw(unsigned int addr, ac_int<32, false> value);

	ac_int<8, false> ldb(unsigned int addr);
	ac_int<16, false> ldh(unsigned int addr);
	ac_int<32, false> ldw(unsigned int addr);

	void doWB(struct MemtoWB memtoWB);
	void doMemNoMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);
	void doMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);
	void doEx(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doExMult(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doExMem(struct DCtoEx dctoEx, struct ExtoMem *extoMem);
	void doDC(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
	void doDCMem(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
	void doDCBr(struct FtoDC ftoDC, struct DCtoEx *dctoEx);

	void doStep();

};


#endif

#endif /* INCLUDES_VEXSIMULATOR_H_ */
