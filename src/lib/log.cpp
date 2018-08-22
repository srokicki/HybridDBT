#include <lib/log.h>
#include <dbt/dbtPlateform.h>

char Log::_verbose_level = -1;
char Log::_stat_mode = -1;

unsigned int plsq_checks, plsq_positive,plsq_false_positive, spec_loop_counter, spec_trace_counter;
unsigned short bitDifferentiation[64];

LogStream& Log::out(char verbose)
{
	static LogStream out;
	out.setVerbose(verbose);
	return out;
}
