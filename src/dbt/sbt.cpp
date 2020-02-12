#include <cstring>
#include <dbt/dbtPlateform.h>
#include <dbt/insertions.h>
#include <dbt/profiling.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lib/endianness.h>

#include <transformation/irGenerator.h>
#include <transformation/irScheduler.h>

#include <transformation/buildControlFlow.h>
#include <transformation/buildTraces.h>
#include <transformation/firstPassTranslation.h>
#include <transformation/optimizeBasicBlock.h>
#include <transformation/reconfigureVLIW.h>
#include <transformation/rescheduleProcedure.h>

#include <isa/riscvISA.h>
#include <isa/vexISA.h>
#include <lib/dbtProfiling.h>
#include <lib/log.h>

void printStats(unsigned int size, short* blockBoundaries)
{

  float numberBlocks = 0;

  for (int oneInstruction = 0; oneInstruction < size; oneInstruction++) {
    if (blockBoundaries[oneInstruction] == 1)
      numberBlocks++;
  }

  Log::printf(0, "\n* Statistics on used binaries:\n");
  Log::printf(0, "* \tThere is %d instructions.\n", size);
  Log::printf(0, "* \tThere is %d blocks.\n", (int)numberBlocks);
  Log::printf(0, "* \tBlocks mean size is %f.\n\n", size / numberBlocks);
}

int translateOneSection(DBTPlateform& dbtPlateform, unsigned int placeCode, int sourceStartAddress,
                        int sectionStartAddress, int sectionEndAddress)
{
  int previousPlaceCode = placeCode;
  unsigned int size     = (sectionEndAddress - sectionStartAddress) >> 2;
  placeCode             = firstPassTranslator(&dbtPlateform, size, sourceStartAddress, sectionStartAddress, placeCode);

  return placeCode;
}

void readSourceBinaries(char* path, unsigned char*& code, unsigned int& addressStart, unsigned int& size,
                        unsigned int& pcStart, DBTPlateform* platform)
{

  //	//We open the elf file and search for the section that is of interest for us
  //	ElfFile elfFile(path);
  //
  //
  //	for (unsigned int sectionCounter = 0; sectionCounter<elfFile.sectionTable->size(); sectionCounter++){
  //		ElfSection *section = elfFile.sectionTable->at(sectionCounter);
  //
  //		//The section we are looking for is called .text
  //		if (!section->getName().compare(".text")){
  //
  //			code = section->getSectionCode();//0x3c
  //			addressStart = section->address + 0;
  //			size = section->size/4 - 0;
  //
  //			if (size > MEMORY_SIZE){
  //				Log::fprintf(0, stderr, "Error: binary file has %d instructions, we currently handle a maximum size
  //of %d\n", size, MEMORY_SIZE); 				exit(-1);
  //			}
  //
  //
  //		}
  //	}
  //
  //	for (int oneSymbol = 0; oneSymbol < elfFile.symbols->size(); oneSymbol++){
  //		ElfSymbol *symbol = elfFile.symbols->at(oneSymbol);
  //		const char* name = (const char*)
  //&(elfFile.sectionTable->at(elfFile.indexOfSymbolNameSection)->getSectionCode()[symbol->name]);
  //
  //		if (strcmp(name, "_start") == 0){
  //			pcStart = symbol->offset;
  //
  //		}
  //	}

  pcStart = 0;
}

#include <binaries.h>

int main(int argc, char* argv[])
{

  /*Parsing arguments of the commant
   *
   */

  int CONFIGURATION = 2;

  int VERBOSE  = 0;
  int STATMODE = 0;

  Log::Init(VERBOSE, STATMODE);

  int OPTLEVEL     = 2;
  char* binaryFile = argv[2];

  int MAX_SCHEDULE_COUNT = -1;
  int MAX_PROC_COUNT     = -1;

  // the application has an argc equal to the number of options + 1 for its path
  // cfg stores all application arguments in the "--" option
  //
  // as neither the C nor the POSIX standard ensures that the arguments are stored
  // in contiguous space, we can use cfg's c_str() as pointers

  /***********************************
   *  Initialization of the DBT platform
   ***********************************
   * In the linux implementation, this is done by reading an elf file and copying binary instructions
   * in the corresponding memory.
   * In a real platform, this may require no memory initialization as the binaries would already be stored in the
   * system memory.
   ************************************/

  // Definition of objects used for DBT process
  DBTPlateform dbtPlateform(MEMORY_SIZE);

  dbtPlateform.vliwInitialConfiguration = CONFIGURATION;
  dbtPlateform.vliwInitialIssueWidth    = getIssueWidth(dbtPlateform.vliwInitialConfiguration);

  // Preparation of required memories
  for (int oneFreeRegister = 33; oneFreeRegister < 63; oneFreeRegister++)
    dbtPlateform.freeRegisters[oneFreeRegister - 33] = oneFreeRegister;

  for (int oneFreeRegister = 63 - 33; oneFreeRegister < 63; oneFreeRegister++)
    dbtPlateform.freeRegisters[oneFreeRegister] = 0;

  for (int onePlaceOfRegister = 0; onePlaceOfRegister < 64; onePlaceOfRegister++)
    dbtPlateform.placeOfRegisters[256 + onePlaceOfRegister] = onePlaceOfRegister;
  // same for FP registers
  for (int onePlaceOfRegister = 0; onePlaceOfRegister < 64; onePlaceOfRegister++)
    dbtPlateform.placeOfRegisters[256 + 64 + onePlaceOfRegister] = onePlaceOfRegister;

  dbtPlateform.vexSimulator = new VexSimulator(dbtPlateform.vliwBinaries);

  unsigned int pcStart = 0;

  if (size > MEMORY_SIZE) {
    Log::printf(LOG_ERROR,
                "ERROR: Size of source binaries is %d. Current implementation only accept size lower then %d\n", size,
                MEMORY_SIZE);
    exit(-1);
  }

  int numberOfSections      = 1 + (size >> 10);
  IRApplication application = IRApplication(numberOfSections);
  Profiler profiler         = Profiler(&dbtPlateform);

  // we copy the binaries in the corresponding memory
  for (int oneInstruction = 0; oneInstruction < size; oneInstruction++)
    dbtPlateform.mipsBinaries[oneInstruction] = ((unsigned int*)code)[oneInstruction];

  // We declare the variable in charge of keeping a track of where we are writing
  unsigned int placeCode = 0; // As 4 instruction bundle

  /********************************************************
   * First part of DBT: generating the first pass translation of binaries
   *******************************************************
   * TODO: description
   *
   *
   ********************************************************/
  unsigned int nbInstrFirstPass = 0;
  unsigned int nbInstrIRGen     = 0;
  unsigned int nbInstrSchedule  = 45;

  unsigned int timeHwFirstPass = 0;
  unsigned int timeHwIRGen     = 0;
  unsigned int timeHwSchedule  = 0;

  for (int oneSection = 0; oneSection < (size >> 10) + 1; oneSection++) {

    int startAddressSource = addressStart + oneSection * 1024 * 4;
    int endAddressSource   = startAddressSource + 1024 * 4;
    if (endAddressSource > addressStart + size * 4)
      endAddressSource = addressStart + (size << 2);

    int effectiveSize = (endAddressSource - startAddressSource) >> 2;
    for (int j = 0; j < effectiveSize; j++) {
      dbtPlateform.mipsBinaries[j] = ((unsigned int*)code)[j + oneSection * 1024];
    }
    int oldPlaceCode = placeCode;

    placeCode = translateOneSection(dbtPlateform, placeCode, addressStart, startAddressSource, endAddressSource);
    nbInstrFirstPass += (endAddressSource - startAddressSource) / 4;
    timeHwFirstPass += timeTakenFirstPass;

    buildBasicControlFlow(&dbtPlateform, oneSection, addressStart, startAddressSource, oldPlaceCode, placeCode,
                          &application, &profiler);
  }

  float coef = 0;

  // We perform aggressive level 1 optimization: if a block takes more than 8 cycle we schedule it.
  // If it has a backward loop, we also profile it.
  for (int oneSection = 0; oneSection < numberOfSections; oneSection++) {
    for (int oneBlock = 0; oneBlock < application.numbersBlockInSections[oneSection]; oneBlock++) {
      IRBlock* block = application.blocksInSections[oneSection][oneBlock];

      if (block != NULL) {
        if (block->sourceEndAddress - block->sourceStartAddress > 16) {

          optimizeBasicBlock(block, &dbtPlateform, &application, placeCode);
          timeHwIRGen += timeTakenIRGeneration;
          timeHwSchedule += timeTakenIRScheduler;

          nbInstrIRGen += block->nbInstr;
          nbInstrSchedule += block->nbInstr;
        }
      }
    }
  }

#ifdef __SW
  printf("\tFirstPass cycle: %d\n", getProfilingInfo(0));
  printf("\tFirstPass instr: %d\n", nbInstrFirstPass);

  printf("\tIRGeneration cycle: %d\n", getProfilingInfo(1));
  printf("\tIRGeneration instr: %d\n", nbInstrIRGen);

  printf("\tIRScheduler cycle: %d\n", getProfilingInfo(2));
  printf("\tIRScheduler instr: %d\n", nbInstrSchedule);
#else
  printf("\tFirstPass cycle: %d\n", timeHwFirstPass);
  printf("\tFirstPass instr: %d\n", nbInstrFirstPass);

  printf("\tIRGeneration cycle: %d\n", timeHwIRGen);
  printf("\tIRGeneration instr: %d\n", nbInstrIRGen);

  printf("\tIRScheduler cycle: %d\n", timeHwSchedule);
  printf("\tIRScheduler instr: %d\n", nbInstrSchedule);

#endif

  return 0;
}
