/*
 * dbtInformation.h
 *
 *  Created on: 16 janv. 2019
 *      Author: simon
 */

#ifndef INCLUDES_DBT_DBTINFORMATION_H_
#define INCLUDES_DBT_DBTINFORMATION_H_


#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************
 *   Function getBlockSize returns the size of a block schedule
 *   Arguments:
 *   	address is the address of the block currently executed
 *   	optLevel is the optimization level of the block (0 for
 *   		first pass, 1 for schedule, 2 for procedure)
 *   	timeFromSwitch is the number of cycle since the decision
 *   		of switching opt level happen
 *   	nextBlock pointer which is modified by the function
 *
 *	Returns the size of the schedule and place the address of
 *	next block in the argument nextBlock
 *************************************************************/
int getBlockSize(int address, int optLevel, int timeFromSwitch, int *nextBlock);

/*************************************************************
 * 	Function initialize will read the the elf file and instantiate
 * 	everything needed for the estimation
 *
 *************************************************************/
void initializeDBTInfo(char* fileName);
char useIndirectionTable(int address);
char getOptLevel(int address, uint64_t nb_cycle);
void verifyBranchDestination(int addressOfJump, int dest);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDES_DBT_DBTINFORMATION_H_ */
