/*
 * loadStoreQueue.cpp
 *
 *  Created on: 8 juin 2018
 *      Author: simon
 */

#define LSQ_SIZE_STORE_QUEUE 16
#define LSQ_SIZE_AGE 5


void loadStoreQueue(ac_int<64, false> address, ac_int<LSQ_SIZE_AGE, false> age, ac_int<64, false> value, ac_int<1, false> isStore, ac_int<1, false> isRm,
					ac_int<1, false> *match, ac_int<1, false> *flush, ac_int<64, false> *result){

	static ac_int<64+LSQ_SIZE_AGE+64+1, false> stores[LSQ_SIZE_STORE_QUEUE];


	if (isStore){

	}
	else{
		for (int oneStore = 0; oneStore<LSQ_SIZE_STORE_QUEUE; oneStore++){
			ac_int<64+LSQ_SIZE_AGE+64+1, false> store = stores[oneStore];
			ac_int<1, false> storeOccupied = store[64+LSQ_SIZE_AGE+64];
			ac_int<64, false> storeAddress = store.slc<64>(LSQ_SIZE_AGE+64);
			ac_int<LSQ_SIZE_AGE, false> storeAge = store.slc<LSQ_SIZE_AGE>(64);
			ac_int<64, false> storeValue = store.slc<64>(0);

		}

	}


}

