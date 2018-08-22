/*
 * loadStoreQueue.cpp
 *
 *  Created on: 8 juin 2018
 *      Author: simon
 */

#define LSQ_SIZE_STORE_QUEUE 16
#define LSQ_SIZE_AGE 5

#include <types.h>
#include <lib/log.h>

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

#ifdef __CATAPULT
#include <ac_int.h>
#else
#include <lib/ac_int.h>
#endif

void partitionnedLoadQueue(ac_int<64, false> pc, ac_int<64, false> address, ac_int<5, false> specId, ac_int<1, false> clear, ac_int<1, false> *rollback,
		unsigned int *speculationData, ac_int<1, false> specInit, ac_int<8, false> specParam, ac_int<64, false> *mask, ac_int<64, false> *rollback_start){

	static ac_int<64, false> addresses[4][4];
	static ac_int<1, false> ages[4][4];
	static ac_int<16, false> specCounters[4];
	static ac_int<16, false> missCounters[4];
	static ac_int<8, false> currentSpecParam[4];
	static ac_int<64, false> currentSpecMasks[4];
	static ac_int<64, false> firstLoad[4];


	ac_int<16, false> hashedAddress = address.slc<16>(3);


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
		if (firstLoad[bank] == 0)
			firstLoad[bank] = pc;

	}
	else if (clear){

		for (int oneAddress = 0; oneAddress < 4; oneAddress++){
			ages[bank][oneAddress] = 0;
		}
		specCounters[bank]++;

		if (specCounters[bank][15]){
			specCounters[bank] = specCounters[bank]>>6;
			missCounters[bank] = missCounters[bank]>>6;
		}
//		ac_int<64+32, false> paramWord = specCounters[bank];
//		paramWord.set_slc(16, missCounters[bank]);
//		paramWord.set_slc(32, currentSpecMasks[bank]);

		speculationData[4*currentSpecParam[bank]] = specCounters[bank];
		speculationData[4*currentSpecParam[bank]+1] = missCounters[bank];
		speculationData[4*currentSpecParam[bank]+2] = currentSpecMasks[bank]>>32;
		speculationData[4*currentSpecParam[bank]+3] = currentSpecMasks[bank] & 0xffffffff;

		firstLoad[bank] = 0;

	}
	else if (specInit){
		ac_int<64+32, false> param = speculationData[specParam];
		currentSpecParam[bank] = specParam;
		currentSpecMasks[bank] = speculationData[4*currentSpecParam[bank]+3];
		ac_int<32, false> hi_mask = speculationData[4*currentSpecParam[bank]+2];
		currentSpecMasks[bank].set_slc(32, hi_mask);
		specCounters[bank] = speculationData[4*currentSpecParam[bank]+0];
		missCounters[bank] = speculationData[4*currentSpecParam[bank]+1];

		firstLoad[bank] = 0;
	}
	else{
		for (int oneAddress = 0; oneAddress<4; oneAddress++){
			ac_int<64, false> storedAddress = addresses[bank][oneAddress];
			ac_int<1, false> storedAge = ages[bank][oneAddress];


			#ifndef __CATAPULT
			//We gather statistics
			for (int oneBit = 0; oneBit<64; oneBit++){
				if (((storedAddress>>oneBit) & 1) != ((address>>oneBit)&1)){
					bitDifferentiation[oneBit]++;
				}
			}

			plsq_checks++;

			if (storedAge && storedAddress.slc<61>(3) == address.slc<61>(3)){
				plsq_positive++;
			}

			if (storedAge && storedAddress.slc<16>(3) == hashedAddress && storedAddress.slc<61>(3) != address.slc<61>(3)){
				plsq_false_positive++;
			}
			#endif

			if (storedAge && storedAddress.slc<16>(3) == address.slc<16>(3)){
				*rollback |= 1;
				*mask = currentSpecMasks[bank];
				*rollback_start = firstLoad[bank];
				missCounters[bank]++;
			}

		}

	}
}
