#ifndef LOG_H
#define LOG_H

#include <lib/logger.h>
#include <cstdarg>

class Log
{
public:
	Log() = delete;
	Log(const Log&) = delete;

	static void Init(Logger * logger);

	static void printf(const char * format, ...)
	{
		va_list args;
		va_start(args, format);
		_logger->printf(format, args);
		va_end(args);
	}

private:
	static Logger * _logger;
};

#endif // LOG_H
