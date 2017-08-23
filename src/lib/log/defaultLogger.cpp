#include <lib/log/defaultLogger.h>

#include <cstdio>
#include <stdarg.h>
#include <iostream>

DefaultLogger::DefaultLogger()
{

}

DefaultLogger::~DefaultLogger()
{

}

void DefaultLogger::printf(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	std::printf(format, args);
	va_end(args);
}
