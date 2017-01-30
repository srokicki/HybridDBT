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

unsigned int insertCodeForProfiling(ac_int<128, false> *binaries, int start, unsigned int startAddress);
void profileBlock(IRBlock *oneBlock, DBTPlateform &platform);
void returnProfilingInformation(DBTPlateform &platform);


#endif /* INCLUDES_DBT_PROFILING_H_ */
