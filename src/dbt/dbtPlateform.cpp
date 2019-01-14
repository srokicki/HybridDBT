/*
 * dbtPlateform.cpp
 *
 *  Created on: 30 janv. 2018
 *      Author: simon
 */

#include <dbt/dbtPlateform.h>


DBTPlateform::DBTPlateform(int binarySize){
	vliwBinaries = (unsigned int*) malloc(4*MEMORY_SIZE*sizeof(unsigned int));
	mipsBinaries = (unsigned int*) malloc(4*MEMORY_SIZE*sizeof(unsigned int));
	insertions = (unsigned int*) malloc(2048*sizeof(unsigned int));
	blockBoundaries = (unsigned char*) malloc(MEMORY_SIZE*sizeof(unsigned char));
	bytecode = (unsigned int*) malloc(256*4*sizeof(unsigned int));
	globalVariables = (int*) malloc(128*sizeof(int));
	unresolvedJumps_src = (unsigned int*) malloc(512*sizeof(unsigned int));
	unresolvedJumps_type = (unsigned int*) malloc(512*sizeof(unsigned int));
	unresolvedJumps = (int*) malloc(512*sizeof(int));
	placeOfRegisters = (unsigned char*) malloc(512*sizeof(unsigned char));
	freeRegisters = (unsigned char*) malloc(64*sizeof(unsigned char));
	placeOfInstr = (unsigned int*) malloc(256*sizeof(unsigned int));
	specInfo = (unsigned int*) malloc(4*256*sizeof(unsigned int));
}

DBTPlateform::~DBTPlateform(){
	free(vliwBinaries);
	free(mipsBinaries);
	free(insertions);
	free(blockBoundaries);
	free(bytecode);
	free(globalVariables);
	free(unresolvedJumps_src);
	free(unresolvedJumps_type);
	free(unresolvedJumps);
	free(placeOfRegisters);
	free(freeRegisters);
	free(placeOfInstr);
	free(specInfo);
}
