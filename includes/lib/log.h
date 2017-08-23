#ifndef LOG_H
#define LOG_H

#include <cstdio>

/**
 * @brief This static class is used to encapsulate logging functions. To change
 * logging behaviour, just change this class members.
 */
class Log
{
public:
	Log() = delete;
	Log(const Log&) = delete;

  /**
   * @brief This method chould be called once to setup the Log state
   */
  static void Init(char verbose_level)
  {
    _verbose_level = verbose_level;
  }

  /**
   * @brief This method encapsulates the standard printf() function.
   */
  template<class ... Ts>
  static void printf(char verbose, const char * format, Ts ... args)
  {
    if (_verbose_level >= verbose)
      std::printf(format, args ...);
  }
  
private:
  static char _verbose_level;
};

char Log::_verbose_level = 0;

#endif // LOG_H
