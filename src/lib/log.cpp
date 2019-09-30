#include <lib/log.h>
#include <dbt/dbtPlateform.h>

char Log::_verbose_level = -1;
char Log::_stat_mode = -1;
std::ostream Log::logScheduleBlocks(NULL);
std::ostream Log::logScheduleProc(NULL);
std::ostream Log::logMemoryDisambiguation(NULL);
std::ostream Log::logWarning(NULL);
std::ostream Log::logError(NULL);

unsigned int plsq_checks, plsq_positive,plsq_false_positive, spec_loop_counter, spec_trace_counter,
cache_l1_miss, cache_l2_miss, memory_accesses;
unsigned short bitDifferentiation[64];

LogStream& Log::out(char verbose)
{
	static LogStream out;
	out.setVerbose(verbose);
	return out;
}
