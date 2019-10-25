#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <iostream>
#include <ostream>
#include <dbt/dbtPlateform.h>
#include <transformation/reconfigureVLIW.h>
#include <isa/irISA.h>

//Definition of debug level symbols
#define LOG_SCHEDULE_BLOCK 6
#define LOG_SCHEDULE_PROC 5
#define LOG_MEMORY_DISAMBIGUATION 4
#define LOG_WARNING 1
#define LOG_ERROR 0


//***************************************************
//Values for statistics

//Stats on PLSQ
extern unsigned int plsq_checks, plsq_positive,plsq_false_positive, spec_loop_counter, spec_trace_counter,
cache_l1_miss, cache_l2_miss, memory_accesses;
extern unsigned short bitDifferentiation[64];


//*************************************************

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

	static std::ostream logScheduleBlocks;
	static std::ostream logScheduleProc;
	static std::ostream logMemoryDisambiguation;
	static std::ostream logWarning;
	static std::ostream logError;

  /**
   * @brief This method should be called once to setup the Log state
   */
  static void Init(char verbose_level, char statMode)
  {
    _verbose_level = verbose_level;
    _stat_mode = statMode;

	if (verbose_level >= LOG_SCHEDULE_BLOCK)
		logScheduleBlocks.rdbuf(std::clog.rdbuf());

	if (verbose_level >= LOG_SCHEDULE_PROC)
		logScheduleProc.rdbuf(std::clog.rdbuf());

	if (verbose_level >= LOG_MEMORY_DISAMBIGUATION)
		logMemoryDisambiguation.rdbuf(std::clog.rdbuf());

	if (verbose_level >= LOG_WARNING)
		logWarning.rdbuf(std::clog.rdbuf());

	if (verbose_level >= LOG_ERROR)
		logError.rdbuf(std::clog.rdbuf());

  }


static void printStat(DBTPlateform *platform, IRApplication *application){

	if (_stat_mode == 1){
	  float energyConsumption = 0;
	  float period = 1.4/1000000000;
	  for (int oneConfig = 0; oneConfig<32; oneConfig++){
		  float timeInConfig = platform->vexSimulator->timeInConfig[oneConfig];
		  timeInConfig = timeInConfig / platform->vexSimulator->cycle;
		  Log::logError << std::hex << oneConfig << ";";
		  Log::logError << timeInConfig*100 << ";" << getPowerConsumption(oneConfig) << ";";
		  energyConsumption += platform->vexSimulator->timeInConfig[oneConfig] * period * getPowerConsumption(oneConfig) / 1000;
	  }

	  Log::logError << platform->vexSimulator->cycle << ";"<< platform->vexSimulator->nbInstr << ";" << ((double) platform->vexSimulator->nbInstr)/((double) platform->vexSimulator->cycle)
			 << ";" << platform->blockScheduleCounter << ";" << platform->procedureOptCounter << ";";

	  Log::logError << platform->vliwInitialConfiguration << ";";
	  Log::logError << energyConsumption << ";";
	  Log::logError << platform->optimizationCycles << ";";
	  Log::logError << platform->optimizationEnergy*period << "\n";

	}
	else{
		float energyConsumption = 0;
		float period = 1.4/1000000000;
		const int lineSize = 100;
		for (int oneConfig = 0; oneConfig<32; oneConfig++){
		  float timeInConfig = platform->vexSimulator->timeInConfig[oneConfig];
		  timeInConfig = timeInConfig / platform->vexSimulator->cycle;
		  Log::logError << "Conf " << std::hex << oneConfig << "\t[";
		  int convertToPercent = timeInConfig * lineSize;
		  for (int oneChar = 0; oneChar < convertToPercent; oneChar++){
			 Log::logError << "|";
		  }
		  for (int oneChar = convertToPercent; oneChar < lineSize; oneChar++){
			  Log::logError << " ";
		  }
		 Log::logError << "] " << timeInConfig*100 << "  Power consumption : " << getPowerConsumption(oneConfig) << "\n";
		  energyConsumption += (platform->vexSimulator->timeInConfig[oneConfig] * period * getPowerConsumption(oneConfig));
		}

		Log::logError << "Execution is finished...\nStatistics on the execution:\n\t Number of cycles: " << platform->vexSimulator->cycle << "\n\t Number of instruction executed: " << platform->vexSimulator->nbInstr << "\n\t Average IPC: " << ((double) platform->vexSimulator->nbInstr)/((double) platform->vexSimulator->cycle) << "\n";
		Log::logError << "\t Configuration used: " << platform->vliwInitialConfiguration << "\n";
		Log::logError << "\t Energy consumed (exec, mJ): " << energyConsumption << "\n";

		Log::logError << "\t Optimization cycles: " << platform->optimizationCycles << "\n";

		Log::logError << "\t Optimization cycles: " << platform->optimizationCycles << "\n";
		Log::logError << "\t Optimization energy (mJ): " << platform->optimizationEnergy/1000000000 << "\n";

		Log::logError << "\t\t Number of cycle spent on system code : " << platform->vexSimulator->nbCycleType[3] << "\n";

		Log::logError << "\t -----------------------------------------------------\n";
		Log::logError << "\t Stats for optimization level 0\n";
		Log::logError << "\t\t Number of cycle spent one level 0 code : " << platform->vexSimulator->nbCycleType[0] << "\n";
		Log::logError << "\t\t Number of instruction scheduled: " << application->numberInstructions << "\n";


		Log::logError << "\t -----------------------------------------------------\n";
		Log::logError << "\t Stats for optimization level 1\n";
		Log::logError << "\t\t Number of cycle spent one level 1 code: " << platform->vexSimulator->nbCycleType[1] << "\n";
		Log::logError << "\t\t Number of block scheduled: " << platform->blockScheduleCounter << "\n";

		Log::logError << "\t -----------------------------------------------------\n";
		Log::logError << "\t Stats for optimization level 2\n";
		Log::logError << "\t\t Number of cycle spent one level 2 code : " << platform->vexSimulator->nbCycleType[2] << "\n";
		Log::logError << "\t\t Number of procedure optimized: " << platform->procedureOptCounter << "\n";
		Log::logError << "\t\t Number of trace construction: " << platform->traceConstructionCounter << "\n";
		Log::logError << "\t\t Number of unrolling: " << platform->unrollingCounter << "\n";
		Log::logError << "\t\t Average block size BEFORE trace/unrolling: " << platform->blockProcAverageSizeBeforeTrace << " (M2 " << platform->blockProcDistanceBeforeTrace/(platform->nbBlockProcedureBeforeTrace -1) << ")\n";
		Log::logError << "\t\t Average block size AFTER trace/unrolling: " <<  platform->blockProcAverageSize << " (M2 " << platform->blockProcDistance/(platform->nbBlockProcedure -1) << ")\n";


		Log::logError << "\t -----------------------------------------------------\n";
		Log::logError << "\t Stats for reconfiguration \n";
		for (int oneConfig = 0; oneConfig<14; oneConfig++){
		  Log::logError << "\t\t #pareto in conf " << oneConfig << ": " << platform->nbTimesInPareto[oneConfig] << "\n";
		}

		int nbMem = 0;
		int nbInstr = 0;
		for (unsigned int oneProcedure = 0; oneProcedure<application->numberProcedures; oneProcedure++){
		  IRProcedure *procedure = application->procedures[oneProcedure];
		  for (unsigned int oneBlock = 0; oneBlock<procedure->nbBlock; oneBlock++){

			  IRBlock *block = procedure->blocks[oneBlock];
			  for (unsigned int oneInstr = 0; oneInstr<block->nbInstr; oneInstr++){
				  int opcode = getOpcode(block->instructions, oneInstr);
				  if ((opcode>>4) == 1)
					  nbMem++;
			  }
			  nbInstr+=block->nbInstr;
		  }

		}
		Log::logError << "\t\t Part of procedure instr that are memory: " << 100*((float)nbMem)/((float)nbInstr) << "\%\n";

		Log::logError << "\t -----------------------------------------------------\n";
		Log::logError << "\t Stats for the Cache\n";
		Log::logError << "\t\t Number of memory accesses: " << memory_accesses << "\n";
		Log::logError << "\t\t Number of l1 miss:" << cache_l1_miss << "\n";
		Log::logError << "\t\t Number of l2 miss: " << cache_l2_miss << "\n";

		Log::logError << "\t -----------------------------------------------------\n";
		Log::logError << "\t Stats for the PLSQ\n";
		Log::logError << "\t\t Number of PLSQ checks: " << plsq_checks << "\n";
		Log::logError << "\t\t Number of PLSQ false positive: " << plsq_false_positive << "\n";
		Log::logError << "\t\t Number of PLSQ true positive: " << plsq_positive << "\n";
		Log::logError << "\t\t Number of speculation groups (loop): " << spec_loop_counter << "\n";
		Log::logError << "\t\t Number of speculation groups (trace): " << spec_trace_counter << "\n";

//		int maxDiff = 1;
//		for (int oneBit = 0; oneBit <64; oneBit++){
//			if (maxDiff<bitDifferentiation[oneBit])
//				maxDiff = bitDifferentiation[oneBit];
//		}
//
//		for (int oneBit = 0; oneBit <64; oneBit++){
//		  float value = bitDifferentiation[oneBit];
//		  value = (value / maxDiff) * 64;
//			Log::fprintf(0, stdout, "\t\t bitDiff%d: %f\n",oneBit, value);
//		}


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
	const LogStream& operator<<(const T& v)
	{
		if (Log::_verbose_level >= _verbose)
			std::cout << v;

		return *this;
	}
};

#endif // LOG_H
