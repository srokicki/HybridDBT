/*
 * insertions.h
 *
 *  Created on: 13 janv. 2017
 *      Author: simon
 */

#ifndef INCLUDES_DBT_INSERTIONS_H_
#define INCLUDES_DBT_INSERTIONS_H_

#include <dbt/dbtPlateform.h>
#include <types.h>

// Values for unresolved jumps
#define UNRESOLVED_JUMP_RELATIVE 1
#define UNRESOLVED_JUMP_ABSOLUTE 0

extern int insertionsArray[65536];
extern int unresolvedJumpsArray[65536];
extern int unresolvedJumpsTypeArray[65536];
extern int unresolvedJumpsSourceArray[65536];

void addInsertions(unsigned int blockStartAddressInSources, unsigned int blockStartAddressInVLIW,
                   unsigned int* insertionsToInsert, unsigned int numberInsertions);
int solveUnresolvedJump(DBTPlateform* platform, unsigned int initialDestination);
unsigned int insertCodeForInsertions(DBTPlateform* platform, int start, unsigned int startAddress);
void initializeInsertionsMemory(int sizeSourceCode);
int getInsertionList(int mipsStartAddress, int** result);

#endif /* INCLUDES_DBT_INSERTIONS_H_ */
