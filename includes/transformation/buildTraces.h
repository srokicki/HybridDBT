/*
 * buildTraces.h
 *
 *  Created on: 22 mai 2017
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_BUILDTRACES_H_
#define INCLUDES_TRANSFORMATION_BUILDTRACES_H_


void buildTraces(DBTPlateform *platform, IRProcedure *procedure, int optLevel);
void memoryDisambiguation(DBTPlateform *platform, IRBlock *block, IRBlock **predecessors, int nbPred);

#endif /* INCLUDES_TRANSFORMATION_BUILDTRACES_H_ */
