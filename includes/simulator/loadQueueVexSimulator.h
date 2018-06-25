/*
 * loadQueueVexSimulator.cpp
 *
 *  Created on: 12 juin 2018
 *      Author: simon
 */

#ifndef INCLUDES_SIMULATOR_LOADQUEUEVEXSIMULATOR_CPP_
#define INCLUDES_SIMULATOR_LOADQUEUEVEXSIMULATOR_CPP_

#include <types.h>
#include <simulator/vexSimulator.h>

#define LOAD_QUEUE_NB_BANK 4
#define LOAD_QUEUE_BANK_SIZE 4
#define LOAD_QUEUE_ADDRESS_SIZE 24
#define LOAD_QUEUE_AGE_SIZE 5


class LoadQueueVexSimulator : public VexSimulator {
public:

	LoadQueueVexSimulator(unsigned int *instructionMemory);
	~LoadQueueVexSimulator(void);

	//Definition of the different memories used by the load queue
	ac_int<LOAD_QUEUE_ADDRESS_SIZE, false> loadQueue_addr[LOAD_QUEUE_NB_BANK][LOAD_QUEUE_BANK_SIZE];
	ac_int<LOAD_QUEUE_AGE_SIZE, false> loadQueue_age[LOAD_QUEUE_NB_BANK][LOAD_QUEUE_BANK_SIZE];

	ac_int<64+32, false> speculationData[256];

	//Functions that will be modified
	void doMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB);


	//New functions will be added to handle speculation
	ac_int<1, false> stbSpec(ac_int<64, false> addr, ac_int<8, true> value, unsigned char specId);
	ac_int<1, false> sthSpec(ac_int<64, false> addr, ac_int<16, true> value, unsigned char specId);
	ac_int<1, false> stwSpec(ac_int<64, false> addr, ac_int<32, true> value, unsigned char specId);
	ac_int<1, false> stdSpec(ac_int<64, false> addr, ac_int<64, true> value, unsigned char specId);

	ac_int<8, true> ldbSpec(ac_int<64, false> addr, unsigned char specId);
	ac_int<16, true> ldhSpec(ac_int<64, false> addr, unsigned char specId);
	ac_int<32, true> ldwSpec(ac_int<64, false> addr, unsigned char specId);
	ac_int<64, true> lddSpec(ac_int<64, false> addr, unsigned char specId);

private:
	ac_int<1, false> checkSpec(ac_int<64, false> addr, unsigned char specId);
};

//Other functions, related with the load Queue Simlulator
void partitionnedLoadQueue(ac_int<64, false> address, ac_int<5, false> specId, ac_int<1, false> clear, ac_int<1, false> *rollback,
		ac_int<64+32, false> speculationData[256], ac_int<1, false> specInit, ac_int<8, false> specParam);


#endif /* INCLUDES_SIMULATOR_LOADQUEUEVEXSIMULATOR_CPP_ */
