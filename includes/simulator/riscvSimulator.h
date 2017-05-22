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
#include <simulator/genericSimulator.h>

class RiscvSimulator : public GenericSimulator{
	public:
	ac_int<64, true> pc;
	uint64_t n_inst;

	RiscvSimulator(void) : GenericSimulator(){};
	int doSimulation(int nbCycles);

	void doStep();
};

#endif

#endif /* INCLUDES_SIMULATOR_RISCVSIMULATOR_H_ */
