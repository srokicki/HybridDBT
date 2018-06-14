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


void LoadQueueVexSimulator::doDCMem(struct FtoDC ftoDC, struct DCtoEx *dctoEx){
	VexSimulator::doDCMem(ftoDC, dctoEx);

	if ((dctoEx->opCode >> 3) == (VEX_LDD>>3) || dctoEx->opCode == VEX_FLW){
		if (dctoEx->isSpec){
			ac_int<64, false> address = dctoEx->dataa + dctoEx->datab;
			dctoEx->memValue = lddSpec(address, (ftoDC.instruction>>8) & 0x1f);
		}
	}

}

void LoadQueueVexSimulator::doMem(ExtoMem extoMem, MemtoWB *memtoWB){
	VexSimulator::doMem(extoMem, memtoWB);

	if (!extoMem.isSpec)
		return;

	ac_int<64, false> address = extoMem.result;

	if (extoMem.opCode == VEX_FSW || extoMem.opCode == VEX_FSH || extoMem.opCode == VEX_FSB){
			memtoWB->WBena = 0; //TODO : this shouldn't be necessary : WB shouldn't be enabled before

			if (extoMem.opCode == VEX_FSW){
				unsigned int value;
				memcpy(&value, &extoMem.floatRes, 4);

				stwSpec(extoMem.result, value, extoMem.funct);
			}
			else if (extoMem.opCode == VEX_FSH){
				unsigned int value;
				memcpy(&value, &extoMem.floatRes, 2);
				sthSpec(address, value, extoMem.funct);
			}
			else if (extoMem.opCode == VEX_FSW){
				unsigned int value;
				memcpy(&value, &extoMem.floatRes, 1);
				stbSpec(address, value, extoMem.funct);
			}
		}
		else if (extoMem.opCode.slc<3>(4) == 1){
			//The instruction is a memory access

			if (extoMem.opCode == VEX_PROFILE){

				if (this->profileResult[extoMem.result] != 255)
					this->profileResult[extoMem.result]++;

				memtoWB->WBena = 0;
			}
			else {




				memtoWB->WBena = 0; //TODO : this shouldn't be necessary : WB shouldn't be enabled before

				//We are on a store instruction
				ac_int<1, false> byteEna0=0, byteEna1=0, byteEna2=0, byteEna3=0, byteEna4=0, byteEna5=0, byteEna6=0, byteEna7=0;
				ac_int<8, false> byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7;
				ac_int<7, false> opcodeToSwitch = extoMem.opCode;
				ac_int<1, false> enableStore = 1;

				if (opcodeToSwitch > VEX_STD){
					opcodeToSwitch -= 4;
					enableStore = this->enable;
				}

				if (enableStore){
					switch (extoMem.opCode){
					case VEX_STD:
						this->stdSpec(address, extoMem.datac, extoMem.funct);
						break;
					case VEX_STW:

						this->stwSpec(address, extoMem.datac.slc<32>(0), extoMem.funct);

					break;
					case VEX_STH:
						//STH

						this->sthSpec(address, extoMem.datac.slc<16>(0), extoMem.funct);
					break;
					case VEX_STB:

						this->stbSpec(address, extoMem.datac.slc<8>(0), extoMem.funct);

						break;
					default:
					break;
					}
				}

}
