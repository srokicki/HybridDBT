#ifndef DEFAULTLOGGER_H
#define DEFAULTLOGGER_H

#include <lib/logger.h>

class DefaultLogger : public Logger
{
public:
	DefaultLogger();
	virtual ~DefaultLogger();

	virtual void printf(const char * format, ...);
};

#endif // DEFAULTLOG_H
