#ifndef LOGGER_H
#define LOGGER_H

class Logger
{
public:
	virtual ~Logger() {}
	virtual void printf(const char * format, ...) = 0;
};

#endif // LOGGER_H
