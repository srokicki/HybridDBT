#include <parameters.h>
#include <types.h>
#include <dbt/dbtPlateform.h>


#ifndef __IR_SCHEDULER
#define __IR_SCHEDULER

int irScheduler(DBTPlateform *platform, uint1 optLevel, uint8 basicBlockSize, uint16 addressInBinaries,
		int6 numberFreeRegister, uint4 issue_width,
		uintIW way_specialisation);

#endif
