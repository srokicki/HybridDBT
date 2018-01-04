/*
 * firstPassTranslation.cpp
 *
 *  Created on: 4 janv. 2018
 *      Author: simon
 */


#include <transformation/firstPassTranslation.h>
#include <dbt/insertions.h>

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

	//We call the accelerator (or the software counterpart if no accelerator)
	#ifndef __NIOS
	int returnedValue = firstPassTranslator_riscv_hw(platform->mipsBinaries,
			size,
			platform->vliwInitialConfiguration,
			sourceStartAddress,
			sectionStartAddress,
			platform->vliwBinaries,
			placeCode,
			platform->insertions,
			platform->blockBoundaries,
			platform->unresolvedJumps_src,
			platform->unresolvedJumps_type,
			platform->unresolvedJumps);
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
