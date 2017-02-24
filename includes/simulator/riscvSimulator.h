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
	std::map<ac_int<64, false>, ac_int<8, true>> memory;

	RiscvSimulator(void): memory(){};

	int doSimulation(int start);
	void std(ac_int<64, false> addr, ac_int<64, true> value);
	void stw(ac_int<64, false> addr, ac_int<32, true> value);
	void sth(ac_int<64, false> addr, ac_int<16, true> value);
	void stb(ac_int<64, false> addr, ac_int<8, true> value);

	ac_int<64, false> ldd(ac_int<64, false> addr);
	ac_int<32, false> ldw(ac_int<64, false> addr);
	ac_int<16, true> ldh(ac_int<64, false> addr);
	ac_int<8, true> ldb(ac_int<64, false> addr);

	void doStep();
};

#endif

#endif /* INCLUDES_SIMULATOR_RISCVSIMULATOR_H_ */
