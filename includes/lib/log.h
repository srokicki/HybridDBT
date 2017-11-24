#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <dbt/dbtPlateform.h>
#include <transformation/reconfigureVLIW.h>

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
  static void Init(char verbose_level, char statMode)
  {
    _verbose_level = verbose_level;
    _stat_mode = statMode;
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

  static void printStat(DBTPlateform *platform){

	  if (_stat_mode == 1){
		  float energyConsumption = 0;
		  float period = 1.4/1000000000;
		  const int lineSize = 100;
		  for (int oneConfig = 0; oneConfig<32; oneConfig++){
			  float timeInConfig = platform->vexSimulator->timeInConfig[oneConfig];
			  timeInConfig = timeInConfig / platform->vexSimulator->cycle;
			  Log::fprintf(0, stdout, "%x;", oneConfig);
			  Log::fprintf(0, stdout, "%d;%f;", timeInConfig*100, getPowerConsumption(oneConfig));
			  energyConsumption += platform->vexSimulator->timeInConfig[oneConfig] * period * getPowerConsumption(oneConfig) / 1000;
		  }

		  Log::fprintf(0, stdout, "%ld;%ld;%f;%d;%d;",	platform->vexSimulator->cycle,
				  platform->vexSimulator->nbInstr,
				  ((double) platform->vexSimulator->nbInstr)/((double) platform->vexSimulator->cycle),
				  platform->blockScheduleCounter,
				  platform->procedureOptCounter);

		  Log::fprintf(0, stdout, "%d;", platform->vliwInitialConfiguration);
		  Log::fprintf(0, stdout, "%f;", energyConsumption);
		  Log::fprintf(0, stdout, "%d;", platform->optimizationCycles);
		  Log::fprintf(0, stdout, "%f\n", platform->optimizationEnergy*period);

	  }
	  else{
		  float energyConsumption = 0;
		  float period = 1.4/1000000000;
		  const int lineSize = 100;
		  for (int oneConfig = 0; oneConfig<32; oneConfig++){
			  float timeInConfig = platform->vexSimulator->timeInConfig[oneConfig];
			  timeInConfig = timeInConfig / platform->vexSimulator->cycle;
			  Log::fprintf(0, stdout, "Conf %x\t[", oneConfig);
			  int convertToPercent = timeInConfig * lineSize;
			  for (int oneChar = 0; oneChar < convertToPercent; oneChar++){
				  Log::fprintf(0, stdout, "|");
			  }
			  for (int oneChar = convertToPercent; oneChar < lineSize; oneChar++){
				  Log::fprintf(0, stdout, " ");
			  }
			  Log::fprintf(0, stdout, "] %f  Power consumption : %f\n", timeInConfig*100, getPowerConsumption(oneConfig));
			  energyConsumption += platform->vexSimulator->timeInConfig[oneConfig] * period * getPowerConsumption(oneConfig) / 1000;
		  }

		  Log::fprintf(0, stdout, "Execution is finished...\nStatistics on the execution:\n\t Number of cycles: %ld\n\t Number of instruction executed: %ld\n\t Average IPC: %f\n\t Number of block scheduled: %d\n\t Number of procedure optimized (O2): %d\n",
				  platform->vexSimulator->cycle, platform->vexSimulator->nbInstr, ((double) platform->vexSimulator->nbInstr)/((double) platform->vexSimulator->cycle), platform->blockScheduleCounter, platform->procedureOptCounter);
		  Log::fprintf(0, stdout, "\tConfiguration used: %d\n", platform->vliwInitialConfiguration);
		  Log::fprintf(0, stdout, "\tEnergy consumed (exec): %f\n", energyConsumption);

		  Log::fprintf(0, stdout, "\tOptimization cycles: %d\n", platform->optimizationCycles);
		  Log::fprintf(0, stdout, "\tOptimization energy (mJ): %f\n", platform->optimizationEnergy*period);
	  }
  }
 
private:

  static char _verbose_level;
  static char _stat_mode;

};


#endif // LOG_H
