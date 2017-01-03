/*
 * riscvSimulator.h
 *
 *  Created on: 2 déc. 2016
 *      Author: simon
 */

#ifndef INCLUDES_SIMULATOR_RISCVSIMULATOR_H_
#define INCLUDES_SIMULATOR_RISCVSIMULATOR_H_

#include <map>
#include <unordered_map>
#include <string>
#include <types.h>

class RiscvSimulator {
	public:
	std::map<int, ac_int<8, true>> memory;

	RiscvSimulator(void): memory(){};

	int doSimulation(int start);
	void stw(int addr, ac_int<32, true> value);
	void sth(int addr, ac_int<16, true> value);
	void stb(int addr, ac_int<8, true> value);

	ac_int<32, false> ldw(int addr);
	ac_int<16, true> ldh(int addr);
	ac_int<8, true> ldb(int addr);

	void doStep();
};



#endif /* INCLUDES_SIMULATOR_RISCVSIMULATOR_H_ */
