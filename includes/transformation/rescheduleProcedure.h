/*
 * rescheduleProcedure.h
 *
 *  Created on: 30 mai 2017
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_RESCHEDULEPROCEDURE_H_
#define INCLUDES_TRANSFORMATION_RESCHEDULEPROCEDURE_H_


int rescheduleProcedure(DBTPlateform *platform, IRProcedure *procedure,int writePlace);

IRProcedure* rescheduleProcedure_schedule(DBTPlateform *platform, IRProcedure *procedure,int writePlace);
int rescheduleProcedure_commit(DBTPlateform *platform, IRProcedure *procedure,int writePlace, IRProcedure *schedulePlaces);


#endif /* INCLUDES_TRANSFORMATION_RESCHEDULEPROCEDURE_H_ */
