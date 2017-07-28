/*
 * reconfigureVLIW.h
 *
 *  Created on: 9 févr. 2017
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_RECONFIGUREVLIW_H_
#define INCLUDES_TRANSFORMATION_RECONFIGUREVLIW_H_

#include <isa/irISA.h>
#include <dbt/dbtPlateform.h>
#include <types.h>
#include <simulator/vexSimulator.h>

/***********************************************************
 *  Header file for reconfigureVLIW
 ****************************************
 * This header file contains the definition of all helpers functions for
 * triggering/managing the VLIW reconfiguration.
 *
 * The current VLIW can have the following configurations:
 *
 * Registers: 32 or 64
 * Issues: 2, 4, 6 or 8
 * Memory/Mult:
 * 		For 2 issues : (1,1)
 * 		For 4 issues : (1,2) (2,1) (1,3) (2,2) (1,1)
 * 		For 6 issues : (1,2) (1,3) (2,1) (2,2) (2,3)
 * 		For 8 issues : (2,3)
 *
 * This results in a total of 24 configurations of the VLIW.
 * The configuration will be encoded using 5 bits:
 *      - the first bit corresponds to the choice between 32 registers (0) or 64 registers (1)
 *      - Next one bit correspond to the extra memory slot: (0) for a config with only one mem, (1) for a config with two mem
 *      - Next two bits corresponds to the extra mult slot: (00) for 1 mult, (01) for two, (10) for three
 *      - Last bit says if the configuration is boosted with basic instruction execution: each configuration of (mem/mult)
 *      		can be reached in two different issue widths. When this bit is at zero, it correspond to the smallest one,
 *      		when it is at one, it corresponds to the biggest.
 *
 */


char getIssueWidth(char configuration);
unsigned int getConfigurationForScheduler(char configuration);
unsigned int getReconfigurationInstruction(char configuration);
char getNbMem(char configuration);
char getNbMult(char configuration);
float getPowerConsumption(char configuration);
void setVLIWConfiguration(VexSimulator *simulator, char configuration);


#endif /* INCLUDES_TRANSFORMATION_RECONFIGUREVLIW_H_ */
