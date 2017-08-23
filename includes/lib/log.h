#ifndef LOG_H
#define LOG_H

#include <cstdio>

class Log
{
public:
	Log() = delete;
	Log(const Log&) = delete;

  static void Init(char verbose_level)
  {
    _verbose_level = verbose_level;
  }

  template<class ... Ts>
  static void printf(char verbose_level, const char * format, Ts ... args)
  {
    if (_verbose_level >= verbose)
      std::printf(format, args ...);
  }
  
private:
  static char _verbose_level;
};

char Log::_verbose_level = 0;

#endif // LOG_H
