#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <iostream>
#include <dbt/dbtPlateform.h>
#include <transformation/reconfigureVLIW.h>
#include <isa/irISA.h>

//Definition of debug level symbols
#define LOG_SCHEDULE_BLOCK 3
#define LOG_SCHEDULE_PROC 2
#define LOG_WARNING 1
#define LOG_ERROR 0


class LogStream;

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

  static void printStat(DBTPlateform *platform, IRApplication *application){

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
			  energyConsumption += (platform->vexSimulator->timeInConfig[oneConfig] * period * getPowerConsumption(oneConfig));
		  }

		  Log::fprintf(0, stdout, "Execution is finished...\nStatistics on the execution:\n\t Number of cycles: %ld\n\t Number of instruction executed: %ld\n\t Average IPC: %f\n",
				  platform->vexSimulator->cycle, platform->vexSimulator->nbInstr, ((double) platform->vexSimulator->nbInstr)/((double) platform->vexSimulator->cycle));
		  Log::fprintf(0, stdout, "\t Configuration used: %d\n", platform->vliwInitialConfiguration);
		  Log::fprintf(0, stdout, "\t Energy consumed (exec, mJ): %f\n", energyConsumption);

		  Log::fprintf(0, stdout, "\t Optimization cycles: %d\n", platform->optimizationCycles);

		  Log::fprintf(0, stdout, "\t Optimization cycles: %d\n", platform->optimizationCycles);
		  Log::fprintf(0, stdout, "\t Optimization energy (mJ): %f\n", platform->optimizationEnergy/1000000000);

		  Log::fprintf(0, stdout, "\t\t Number of cycle spent on system code : %d\n", platform->vexSimulator->nbCycleType[3]);

		  Log::fprintf(0, stdout, "\t -----------------------------------------------------\n");
		  Log::fprintf(0, stdout, "\t Stats for optimization level 0:\n");
		  Log::fprintf(0, stdout, "\t\t Number of cycle spent one level 0 code : %d\n", platform->vexSimulator->nbCycleType[0]);

		  Log::fprintf(0, stdout, "\t -----------------------------------------------------\n");
		  Log::fprintf(0, stdout, "\t Stats for optimization level 1:\n");
		  Log::fprintf(0, stdout, "\t\t Number of cycle spent one level 1 code: %d\n", platform->vexSimulator->nbCycleType[1]);
		  Log::fprintf(0, stdout, "\t\t Number of block scheduled: %d\n", platform->blockScheduleCounter);

		  Log::fprintf(0, stdout, "\t -----------------------------------------------------\n");
		  Log::fprintf(0, stdout, "\t Stats for optimization level 2:\n");
		  Log::fprintf(0, stdout, "\t\t Number of cycle spent one level 2 code : %d\n", platform->vexSimulator->nbCycleType[2]);
		  Log::fprintf(0, stdout, "\t\t Number of procedure optimized: %d\n", platform->procedureOptCounter);
		  Log::fprintf(0, stdout, "\t\t Number of trace construction: %d\n", platform->traceConstructionCounter);
		  Log::fprintf(0, stdout, "\t\t Number of unrolling: %d\n", platform->unrollingCounter);
		  Log::fprintf(0, stdout, "\t\t Average block size BEFORE trace/unrolling: %f (M2 %f)\n", platform->blockProcAverageSizeBeforeTrace, platform->blockProcDistanceBeforeTrace/(platform->nbBlockProcedureBeforeTrace -1));
		  Log::fprintf(0, stdout, "\t\t Average block size AFTER trace/unrolling: %f (M2 %f)\n", platform->blockProcAverageSize, platform->blockProcDistance/(platform->nbBlockProcedure -1));

		  int nbMem = 0;
		  int nbInstr = 0;
		  for (int oneProcedure = 0; oneProcedure<application->numberProcedures; oneProcedure++){
			  IRProcedure *procedure = application->procedures[oneProcedure];
			  for (int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){

				  IRBlock *block = procedure->blocks[oneBlock];
				  for (int oneInstr = 0; oneInstr<block->nbInstr; oneInstr++){
					  int opcode = getOpcode(block->instructions, oneInstr);
					  if ((opcode>>4) == 1)
						  nbMem++;
				  }
				  nbInstr+=block->nbInstr;
			  }

		  }
		  Log::fprintf(0, stdout, "\t\t Part of procedure instr that are memory: %f \%\n", 100*((float)nbMem)/((float)nbInstr));



	  }
  }

	static LogStream& out(char verbose);

private:

  static char _verbose_level;
  static char _stat_mode;

	friend class LogStream;
};

class LogStream
{
private:
	char _verbose;
	LogStream() : _verbose(0) {}

	void setVerbose(char verbose) { _verbose = verbose; }
	friend class Log;

public:

	template<class T>
	LogStream& operator<<(T v)
	{
		if (Log::_verbose_level >= _verbose)
			std::cout << v;

		return *this;
	}
};

#endif // LOG_H
