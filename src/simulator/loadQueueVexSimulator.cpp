#include <simulator/loadQueueVexSimulator.h>
#include <isa/vexISA.h>


ac_int<64, true> LoadQueueVexSimulator::lddSpec(ac_int<64, false> addr, unsigned char specId){

	ac_int<3, false> bankId = bankId >> 2;
	ac_int<2, false> indexInBank = specId & 0x3;

	this->loadQueue_addr[bankId][indexInBank] = addr.slc<24>(3);
	this->loadQueue_age[bankId][indexInBank] = 1;

	return this->ldd(addr);
}


ac_int<1, false> LoadQueueVexSimulator::checkSpec(ac_int<64, false> addr, unsigned char specId){
	ac_int<3, false> bankId = bankId >> 2;
	ac_int<2, false> indexInBank = specId & 0x3;

	for (int oneLoad = 0; oneLoad < LOAD_QUEUE_BANK_SIZE; oneLoad++){
		if (this->loadQueue_age[bankId][oneLoad] && this->loadQueue_addr[bankId][indexInBank] == addr.slc<24>(3)){
			fprintf(stderr, "Mispeculation !!\n");
			exit(-1);
		}
	}
}

ac_int<1, false> LoadQueueVexSimulator::stbSpec(ac_int<64, false> addr, ac_int<8, true> value, unsigned char specId){
	stb(addr, value);
	return checkSpec(addr, specId);
}
ac_int<1, false> LoadQueueVexSimulator::sthSpec(ac_int<64, false> addr, ac_int<16, true> value, unsigned char specId){
	sth(addr, value);
	return checkSpec(addr, specId);
}

ac_int<1, false> LoadQueueVexSimulator::stwSpec(ac_int<64, false> addr, ac_int<32, true> value, unsigned char specId){
	stw(addr, value);
	return checkSpec(addr, specId);
}

ac_int<1, false> LoadQueueVexSimulator::stdSpec(ac_int<64, false> addr, ac_int<64, true> value, unsigned char specId){
	std(addr, value);
	return checkSpec(addr, specId);
}


LoadQueueVexSimulator::LoadQueueVexSimulator(unsigned int *instructionMemory) : VexSimulator(instructionMemory){};
LoadQueueVexSimulator::~LoadQueueVexSimulator(void){};


void LoadQueueVexSimulator::doMem(ExtoMem extoMem, MemtoWB *memtoWB){


	ac_int<1, false> rollback = 0;
	ac_int<1, false> clear = 0;
	ac_int<1, false> init = 0;

	//If instruction is speculative, we check the LSQ before
	if (((extoMem.opCode >> 4) == 0x1 || extoMem.opCode == VEX_FLW || extoMem.opCode == VEX_FSW) && extoMem.isSpec){

		if (extoMem.opCode == VEX_SPEC_RST){
			clear = 1;
		}
		else if (extoMem.opCode == VEX_SPEC_INIT){
			init = 1;
		}

		partitionnedLoadQueue(extoMem.result, extoMem.funct, clear, &rollback,
				this->speculationData, init, extoMem.result);


		if (rollback){
			fprintf(stderr, "In LQ vex simulator, system detected that we had to rollback...\n");
			exit(-1);
		}


	}


	VexSimulator::doMem(extoMem, memtoWB);

}
