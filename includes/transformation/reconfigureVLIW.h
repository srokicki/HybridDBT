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

uint32 reconfigureVLIW(DBTPlateform *platform, IRProcedure *procedure, uint32 placeCode);


#endif /* INCLUDES_TRANSFORMATION_RECONFIGUREVLIW_H_ */
