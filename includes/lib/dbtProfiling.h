/*
 * dbtProfiling.h
 *
 *  Created on: 17 janv. 2018
 *      Author: simon
 */

#ifndef INCLUDES_LIB_DBTPROFILING_H_
#define INCLUDES_LIB_DBTPROFILING_H_

#define SYS_PROFILING_START 2000
#define SYS_PROFILING_STOP 2001
#define SYS_PROFILING_GET 2002

void startProfiler(int id);
void stopProfiler(int id);
int getProfilingInfo(int id);

#endif /* INCLUDES_LIB_DBTPROFILING_H_ */
