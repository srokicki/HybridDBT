/*
 * riscvSimulator.h
 *
 *  Created on: 2 déc. 2016
 *      Author: simon
 */

#ifndef INCLUDES_SIMULATOR_RISCVSIMULATOR_H_
#define INCLUDES_SIMULATOR_RISCVSIMULATOR_H_

#ifndef __NIOS


#include <map>
#include <unordered_map>
#include <string>
#include <types.h>

class RiscvSimulator {
	public:
	int debugLevel;

	std::map<ac_int<64, false>, ac_int<8, true>> memory;
	ac_int<64, true> reg[32];


	RiscvSimulator(void): fileMap(), memory(){this->debugLevel = 0;};
	void initialize(int argc, char* argv[]);

	int doSimulation(int start);
	void std(ac_int<64, false> addr, ac_int<64, true> value);
	void stw(ac_int<64, false> addr, ac_int<32, true> value);
	void sth(ac_int<64, false> addr, ac_int<16, true> value);
	void stb(ac_int<64, false> addr, ac_int<8, true> value);

	ac_int<64, false> ldd(ac_int<64, false> addr);
	ac_int<32, true> ldw(ac_int<64, false> addr);
	ac_int<16, true> ldh(ac_int<64, false> addr);
	ac_int<8, true> ldb(ac_int<64, false> addr);


	std::map<ac_int<16, true>, FILE*> fileMap;

	ac_int<64, false> doRead(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size);
	ac_int<64, false> doWrite(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size);
	ac_int<64, false> doOpen(ac_int<64, false> name, ac_int<64, false> flags, ac_int<64, false> mode);
	ac_int<64, false> doOpenat(ac_int<64, false> dir, ac_int<64, false> name, ac_int<64, false> flags, ac_int<64, false> mode);
	ac_int<64, false> doLseek(ac_int<64, false> file, ac_int<64, false> ptr, ac_int<64, false> dir);
	ac_int<64, false> doClose(ac_int<64, false> file);
	ac_int<64, false> doStat(ac_int<64, false> filename, ac_int<64, false> ptr);

	void doStep();
};

#endif

#endif /* INCLUDES_SIMULATOR_RISCVSIMULATOR_H_ */
