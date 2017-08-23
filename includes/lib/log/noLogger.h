#ifndef NOLOG_H
#define NOLOG_H

#include <lib/logger.h>

class NoLogger : public Logger
{
public:
	NoLogger();
	virtual ~NoLogger();
	virtual void printf(const char * format, ...);
};

#endif // NOLOG_H
