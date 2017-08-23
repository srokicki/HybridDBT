#ifndef MULTILOGGER_H
#define MULTILOGGER_H

#include <lib/logger.h>
#include <vector>
#include <cstdlib>
#include <string>

class MultiLogger : public Logger
{
public:
  MultiLogger(const std::vector<std::string>& files);
  virtual ~MultiLogger();
  virtual void printf(const char * format, ...);
private:
  std::vector<FILE*> _files;
};
#endif
