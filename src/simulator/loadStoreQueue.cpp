/*
 * loadStoreQueue.cpp
 *
 *  Created on: 8 juin 2018
 *      Author: simon
 */

#define LSQ_SIZE_STORE_QUEUE 16
#define LSQ_SIZE_AGE 5

#include <types.h>

//void loadStoreQueue(ac_int<64, false> address, ac_int<LSQ_SIZE_AGE, false> age, ac_int<64, false> value, ac_int<1, false> isStore, ac_int<1, false> isRm,
//					ac_int<1, false> *match, ac_int<1, false> *flush, ac_int<64, false> *result){
//
//	static ac_int<64+LSQ_SIZE_AGE+64+1, false> stores[LSQ_SIZE_STORE_QUEUE];
//
//
//	if (isStore){
//
//	}
//	else{
//		for (int oneStore = 0; oneStore<LSQ_SIZE_STORE_QUEUE; oneStore++){
//			ac_int<64+LSQ_SIZE_AGE+64+1, false> store = stores[oneStore];
//			ac_int<1, false> storeOccupied = store[64+LSQ_SIZE_AGE+64];
//			ac_int<64, false> storeAddress = store.slc<64>(LSQ_SIZE_AGE+64);
//			ac_int<LSQ_SIZE_AGE, false> storeAge = store.slc<LSQ_SIZE_AGE>(64);
//			ac_int<64, false> storeValue = store.slc<64>(0);
//
//		}
//
//	}
//
//
//}

#include <lib/ac_int.h>

void partitionnedLoadQueue(ac_int<64, false> address, ac_int<5, false> specId, ac_int<1, false> clear, ac_int<1, false> *rollback,
		ac_int<64+32, false> speculationData[256], ac_int<1, false> specInit, ac_int<8, false> specParam){

	static ac_int<64, false> addresses[4][4];
	static ac_int<1, false> ages[4][4];
	static ac_int<16, false> specCounters[4];
	static ac_int<16, false> missCounters[4];
	static ac_int<8, false> currentSpecParam[4];
	static ac_int<64, false> currentSpecMasks[4];


	ac_int<1, false> saveMemOp = specId[4];
	ac_int<2, false> bank = specId.slc<2>(0);
	*rollback = 0;

	if (saveMemOp){
		for (int oneAddress = 3; oneAddress > 0; oneAddress--){
			addresses[bank][oneAddress - 1] = addresses[bank][oneAddress];
			ages[bank][oneAddress - 1] = ages[bank][oneAddress];
		}
		addresses[bank][0] = address;
		ages[bank][0] = 1;

	}
	else if (clear){
		for (int oneAddress = 0; oneAddress < 4; oneAddress++){
			ages[bank][oneAddress] = 0;
		}
		specCounters[bank]++;
		ac_int<64+32, false> paramWord = specCounters[bank];
		paramWord.set_slc(16, missCounters[bank]);
		paramWord.set_slc(32, currentSpecMasks[bank]);

		speculationData[currentSpecParam[bank]] = paramWord;
	}
	else if (specInit){
		ac_int<64+32, false> param = speculationData[specParam];
		currentSpecParam[bank] = specParam;
		currentSpecMasks[bank] = param.slc<64>(32);
		specCounters[bank] = param.slc<16>(0);
		missCounters[bank] = param.slc<16>(16);
	}
	else{

		for (int oneAddress = 0; oneAddress<4; oneAddress++){
			ac_int<64, false> storedAddress = addresses[bank][oneAddress];
			ac_int<1, false> storedAge = ages[bank][oneAddress];

			if (storedAge && storedAddress == address){
				*rollback |= 1;
				missCounters[bank]++;
			}

		}

	}
}
