/*
 * profiling.h
 *
 *  Created on: 26 janv. 2017
 *      Author: simon
 */

#ifndef INCLUDES_DBT_PROFILING_H_
#define INCLUDES_DBT_PROFILING_H_

#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>



class Profiler{
public:
	DBTPlateform *platform;		  //Pointer to the DBT platform
	int destinationCallProfiling; //This is the vliw address of the start of the profiling procedure in VLIW memory
	int numberProfiledBlocks = 0; //Number of profiled blocks
	IRBlock* profiledBlocks[64];// list of profiled blocks

	Profiler(DBTPlateform *platform);

	int getNumberProfiledBlocks();
	int getProfilingInformation(int ID);
	IRBlock* getBlock(int ID);

	/* Procedure insertProfilingProcedure will add in VLIW instruction memory binaries corresponding to the definition of
	 * a procedure in charge of profiling a given block.
	 * This inserted procedure can be called when a block need to be profiled.
	 */
	unsigned int insertProfilingProcedure(int start, unsigned int startAddress);


	/* Procedure profileBlock will add necessary instructions to profile a block in VLIW memory
	 * It can insert these instructions into empty slots of the binaries or insert a call instruction
	 * to the dedicated procedure, which was inserted by procedure insertProfilingProcedure
	 */
	void profileBlock(IRBlock *oneBlock);
	void unprofileBlock(int ID);

};


#endif /* INCLUDES_DBT_PROFILING_H_ */
