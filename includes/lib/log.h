#ifndef LOG_H
#define LOG_H

#include <cstdio>

//Definition of debug level symbols
#define LOG_SCHEDULE_BLOCK 3
#define LOG_SCHEDULE_PROC 2
#define LOG_WARNING 1
#define LOG_ERROR 0


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
      std::fprintf(stderr, format, args ...);
  }

  /**
   * @brief This method encapsulates the standard fprintf() function.
   */
  template<class ... Ts>
  static void fprintf(char verbose, FILE * f, const char * format, Ts ... args)
  {
    if (_verbose_level >= verbose)
      std::fprintf(f, format, args ...);
  }
 
private:

  static char _verbose_level;
};


#endif // LOG_H
