/*
 * BuildControlFlow.h
 *
 *  Created on: 29 nov. 2016
 *      Author: Simon Rokicki
 */

#ifndef INCLUDES_TRANSFORMATION_BUILDCONTROLFLOW_H_
#define INCLUDES_TRANSFORMATION_BUILDCONTROLFLOW_H_

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>

/********************************************************************
 * Declaration a procedure made to build the basic control flow
 * ******************************************************************
 *
 * Given a start address and an end address, the procedure will use information stored in "insertions",
 * "blockBoundaries" and "procedureBoundaries" to build the basic control flow of binaries just translated.
 *
 * IMPORTANT: This procedure may be run before these information are removes (so before another translation is done).
 *
 * By basic, we mean to represent only procedure and block boundaries, without representing blocks successors or
 * call graphs. This is meant for another more advances (and more expensive) transformation.
 * We just want precise boundaries to determine where to perform further optimizations.
 *
 *******************************************************************/

int buildBasicControlFlow(DBTPlateform dbtPlateform, int startAddress, int endAddress, IRProcedure** result);



#endif /* INCLUDES_TRANSFORMATION_BUILDCONTROLFLOW_H_ */
