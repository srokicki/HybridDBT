/*
 * dbtPlateform.cpp
 *
 *  Created on: 30 janv. 2018
 *      Author: simon
 */

#include <dbt/dbtPlateform.h>


DBTPlateform::DBTPlateform(int binarySize){
	vliwBinaries = (unsigned int*) calloc(4*MEMORY_SIZE, sizeof(unsigned int));
	mipsBinaries = (unsigned int*) calloc(4*MEMORY_SIZE,sizeof(unsigned int));
	insertions = (unsigned int*) calloc(2048, sizeof(unsigned int));
	blockBoundaries = (unsigned char*) calloc(4*MEMORY_SIZE, sizeof(unsigned char));
	bytecode = (unsigned int*) calloc(256*4, sizeof(unsigned int));
	globalVariables = (int*) calloc(128, sizeof(int));
	unresolvedJumps_src = (unsigned int*) calloc(1024, sizeof(unsigned int));
	unresolvedJumps_type = (unsigned int*) calloc(1024, sizeof(unsigned int));
	unresolvedJumps = (int*) calloc(1024, sizeof(int));
	placeOfRegisters = (unsigned char*) calloc(512, sizeof(unsigned char));
	freeRegisters = (unsigned char*) calloc(64, sizeof(unsigned char));
	placeOfInstr = (unsigned int*) calloc(256, sizeof(unsigned int));
	specInfo = (unsigned int*) calloc(4*256, sizeof(unsigned int));
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
