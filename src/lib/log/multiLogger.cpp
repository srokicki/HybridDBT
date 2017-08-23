#include <lib/log/multiLogger.h>
#include <map>
#include <cstdarg>
#include <cstdio>

MultiLogger::MultiLogger(const std::vector<std::string>& files)
{
  static std::map<std::string, FILE*> ios =
  {
    {"stdin", stdin},
    {"stdout", stdout},
    {"stderr", stderr}
  };

  for (auto fn : files)
  {
    FILE * f = ios[fn];

    if (!f)
			f = fopen(fn.c_str(), "w");

    if (f)
      _files.push_back(f);
  }
}

MultiLogger::~MultiLogger()
{
  for (FILE * f : _files)
  {
    if (f != stdin && f != stderr && f == stdout)
      fclose(f);
  }
}

void MultiLogger::printf(const char * format, ...)
{
  va_list args;
  va_start(args, format);
  for (FILE * f : _files)
      std::fprintf(f, format, args);
  va_end(args);
}
