/*
 * firstPassTranslation.cpp
 *
 *  Created on: 4 janv. 2018
 *      Author: simon
 */


#include <transformation/firstPassTranslation.h>
#include <dbt/insertions.h>
#include <lib/endianness.h>

/**************************************************************
 *  Function firstPassTranslator will translate a piece of MIPS binaries from the memory 'code' and
 *  write the result in VLIW binaries. At the same time, it collects and solve unresolved jumps (indeed jumps
 *  destination may change because of insertions during the translation); it also keep trace of the basic block
 *  and procedures boundaries.
 *
 **************************************************************/
unsigned int firstPassTranslator(DBTPlateform *platform,
		unsigned int size,
		unsigned int sourceStartAddress,
		unsigned int sectionStartAddress,
		unsigned int placeCode){


	platform->insertions[0] = 0;
	fprintf(stderr, "translating at %d\n", placeCode);

	//We call the accelerator (or the software counterpart if no accelerator)
	#ifndef __NIOS

	ac_int<32, false> *localMipsBinaries = (ac_int<32, false>*) malloc(MEMORY_SIZE*sizeof(ac_int<32, false>));
	ac_int<32, false> *localInsertions = (ac_int<32, false>*) malloc(2048*sizeof(ac_int<32, false>));
	ac_int<128, false> *localVliwBinaries = (ac_int<128, false>*) malloc(MEMORY_SIZE*sizeof(ac_int<128, false>));
	ac_int<1, false> *localBlockBoundaries = (ac_int<1, false>*) malloc(MEMORY_SIZE*sizeof(ac_int<1, false>));
	ac_int<32, false> *localUnresolvedJumps_src = (ac_int<32, false>*) malloc(512*sizeof(ac_int<32, false>));
	ac_int<32, false> *localUnresolvedJumps_type = (ac_int<32, false>*) malloc(512*sizeof(ac_int<32, false>));
	ac_int<32, true> *localUnresolvedJumps = (ac_int<32, true>*) malloc(512*sizeof(ac_int<32, true>));

	acintMemcpy(localMipsBinaries, platform->mipsBinaries, MEMORY_SIZE*4);
	acintMemcpy(localInsertions, platform->insertions, 2048*4);
	acintMemcpy(localVliwBinaries, platform->vliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(localBlockBoundaries, platform->blockBoundaries, MEMORY_SIZE);
	acintMemcpy(localUnresolvedJumps_src, platform->unresolvedJumps_src, 512*4);
	acintMemcpy(localUnresolvedJumps_type, platform->unresolvedJumps_type, 512*4);
	acintMemcpy(localUnresolvedJumps, platform->unresolvedJumps, 512*4);

	//TODO

	int returnedValue = firstPassTranslator_riscv_hw(localMipsBinaries,
			size,
			platform->vliwInitialConfiguration,
			sourceStartAddress,
			sectionStartAddress,
			localVliwBinaries,
			placeCode,
			localInsertions,
			localBlockBoundaries,
			localUnresolvedJumps_src,
			localUnresolvedJumps_type,
			localUnresolvedJumps);

	acintMemcpy(platform->mipsBinaries, localMipsBinaries, MEMORY_SIZE*4);
	acintMemcpy(platform->insertions, localInsertions, 2048*4);
	acintMemcpy(platform->vliwBinaries, localVliwBinaries, MEMORY_SIZE*16);
	acintMemcpy(platform->blockBoundaries, localBlockBoundaries, MEMORY_SIZE);
	acintMemcpy(platform->unresolvedJumps_src, localUnresolvedJumps_src, 512*4);
	acintMemcpy(platform->unresolvedJumps_type, localUnresolvedJumps_type, 512*4);
	acintMemcpy(platform->unresolvedJumps, localUnresolvedJumps, 512*4);



	#else
		int argA = size + (placeCode<<16);
		int argB = addressStart;
		printf("Test\n");
		int returnedValue = ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0(argA, argB);

		printf("Passed first pass\n");
	#endif


	//We translate the result
	unsigned int destinationIndex = returnedValue & 0x3ffff;
	unsigned int numberUnresolvedJumps = returnedValue >> 18;



	//We copy insertions

	addInsertions((sectionStartAddress-sourceStartAddress)>>2, placeCode, platform->insertions, platform->insertions[0]);

	//Modelization of optimization time : here we needed one cycle per generated instruction
	platform->optimizationCycles += destinationIndex;
	platform->optimizationEnergy += destinationIndex*0.44;




	return placeCode + destinationIndex;
}
