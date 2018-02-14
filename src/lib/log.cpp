#include <lib/log.h>
#include <dbt/dbtPlateform.h>

char Log::_verbose_level = -1;
char Log::_stat_mode = -1;

LogStream& Log::out(char verbose)
{
	static LogStream out;
	out.setVerbose(verbose);
	return out;
}
