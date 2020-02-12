/*
 * rescheduleProcedure.h
 *
 *  Created on: 30 mai 2017
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_RESCHEDULEPROCEDURE_H_
#define INCLUDES_TRANSFORMATION_RESCHEDULEPROCEDURE_H_

#include <dbt/dbtPlateform.h>
#include <isa/irISA.h>

int rescheduleProcedure(DBTPlateform* platform, IRApplication* application, IRProcedure* procedure,
                        unsigned int writePlace);

IRProcedure* rescheduleProcedure_schedule(DBTPlateform* platform, IRApplication* application, IRProcedure* procedure,
                                          unsigned int writePlace);
int rescheduleProcedure_commit(DBTPlateform* platform, IRApplication* application, IRProcedure* procedure,
                               unsigned int writePlace, IRProcedure* schedulePlaces);
void inPlaceBlockReschedule(IRBlock* block, DBTPlateform* platform, IRApplication* application,
                            unsigned int writePlace);

#endif /* INCLUDES_TRANSFORMATION_RESCHEDULEPROCEDURE_H_ */
