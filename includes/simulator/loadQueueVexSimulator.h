/*
 * loadQueueVexSimulator.cpp
 *
 *  Created on: 12 juin 2018
 *      Author: simon
 */

#ifndef INCLUDES_SIMULATOR_LOADQUEUEVEXSIMULATOR_CPP_
#define INCLUDES_SIMULATOR_LOADQUEUEVEXSIMULATOR_CPP_

#define LOAD_QUEUE_NB_BANK 4
#define LOAD_QUEUE_BANK_SIZE 4
#define LOAD_QUEUE_ADDRESS_SIZE 24
#define LOAD_QUEUE_AGE_SIZE 5


class LoadQueueVexSimulator: public VexSimulator
{
public:

	//Definition of the different memories used by the load queue
	ac_int<LOAD_QUEUE_ADDRESS_SIZE, false> loadQueue_addr[LOAD_QUEUE_NB_BANK][LOAD_QUEUE_BANK_SIZE];
	ac_int<LOAD_QUEUE_AGE_SIZE, false> loadQueue_age[LOAD_QUEUE_NB_BANK][LOAD_QUEUE_BANK_SIZE];

	//Functions that will be modified
	void doDCMem(struct FtoDC ftoDC, struct DCtoEx *dctoEx);
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

#endif /* INCLUDES_SIMULATOR_LOADQUEUEVEXSIMULATOR_CPP_ */
