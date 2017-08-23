#include <lib/log.h>
#include <lib/log/defaultLogger.h>

Logger * Log::_logger = nullptr;
void Log::Init(Logger * logger)
{
	if (!_logger)
		_logger = logger ? logger : new DefaultLogger();
	else
		printf("Log::Init() ne peut être appelé plusieurs fois");
}
