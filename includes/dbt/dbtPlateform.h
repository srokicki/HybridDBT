#ifndef __DBTPLATEFORM
#define __DBTPLATEFORM

/********************************************************************
 * This file describe the class dbtPlatform.
 *
 * This class represent different memories that are used in the actual hardware platform. Those memories will be used by
 * hardware accelerators to store different information collected during the DBT process.
 *
 * dbtPlatform has the following types:
 *  -> vliwBinaries is a memory containing 128-bits words, each of them encoding a single VLIW syllabus. This memory is
 *  used by different accelerators to store their generated binaries but also by the VLIW as instruction memory.
 *  -> mipsBinaries is initialized with the MIPS binaries to translate
 *  -> insertions, insertions_type, insertions_src is the list of all places where firstPassTranslator had to insert an instruction to handle MIPS ISA
 *  correctly. TODO: describe how it is generated and how to use it
 *  ->
 *
 ********************************************************************/

#define MEMORY_SIZE 2300000
#define MAX_INSERTION_PER_SECTION 2048
#define SHIFT_FOR_INSERTION_SECTION 13 //Should be equal to log2(MAX_INSERTION_PER_SECTION) + 2


#define DBT_TYPE_HW 0
#define DBT_TYPE_SW 1

#include <simulator/vexSimulator.h>
#include <simulator/riscvSimulator.h>


#include <types.h>

#ifndef __NIOS
class DBTPlateform
{

public:

	/*
	ac_int<128, false> vliwBinaries[MEMORY_SIZE];
	ac_int<32, false> mipsBinaries[MEMORY_SIZE];
	ac_int<32, false> insertions[2048];
	ac_int<1, false> blockBoundaries[MEMORY_SIZE];
	ac_int<128, false> bytecode[256];
	ac_int<32, true> globalVariables[64];
	ac_int<32, false> unresolvedJumps_src[512];
	ac_int<32, false> unresolvedJumps_type[512];
	ac_int<32, true> unresolvedJumps[512];
	ac_int<6, false> placeOfRegisters[512];
	ac_int<6, false> freeRegisters[64];
	ac_int<32, false> placeOfInstr[256];
	*/

	unsigned int *vliwBinaries;
	unsigned int *mipsBinaries;
	unsigned int *insertions;
	unsigned char *blockBoundaries;
	unsigned int *bytecode;
	int *globalVariables;
	unsigned int *unresolvedJumps_src;
	unsigned int *unresolvedJumps_type;
	int *unresolvedJumps;
	unsigned char *placeOfRegisters;
	unsigned char *freeRegisters;
	unsigned int *placeOfInstr;
	unsigned int *specInfo;
	VexSimulator* vexSimulator;
	RiscvSimulator* riscvSimulator;

	int vliwInitialConfiguration;
	char vliwInitialIssueWidth;
	char debugLevel;

	//For simulating the optimization time in the platform
	long int optimizationCycles=0;
	double optimizationEnergy=0;
	int blockScheduleCounter=0;
	int procedureOptCounter=0;
	int unrollingCounter=0;
	int traceConstructionCounter=0;
	double blockProcAverageSize = 0;
	double blockProcDistance = 0;
	int nbBlockProcedure = 0;

	double blockProcAverageSizeBeforeTrace = 0;
	double blockProcDistanceBeforeTrace = 0;
	int nbBlockProcedureBeforeTrace = 0;
	int nbTimesInPareto[14] = {0};
	char dbtType = DBT_TYPE_HW;

	DBTPlateform(int binarySize);
	~DBTPlateform();

};
#endif


#ifdef __NIOS
class DBTPlateform
{

public:
	unsigned int *vliwBinaries = (unsigned int*) (0x20000000 + 0x80000000);
	unsigned int *mipsBinaries = (unsigned int*) (0x10000000 + 0x80000000);
	unsigned int *insertions = (unsigned int*) (0x30000000 + 0x80000000);
	char *blockBoundaries = (char*)(0x40000000 + 0x80000000);
	short *procedureBoundaries = (short*) (0x50000000 + 0x80000000);
	unsigned int *bytecode = (unsigned int*) (0x70000000 + 0x80000000);
	int *globalVariables = (int*) (0x63000000 + 0x80000000);
	int *unresolvedJumps_src = (int*) (0x60000000 + 0x80000000);
	char *unresolvedJumps_type = (char*) (0x61000000 + 0x80000000);
	int *unresolvedJumps = (int*) (0x62000000 + 0x80000000);
	char *placeOfRegisters = (char) (0x64000000 + 0x80000000);
	char *freeRegisters = (char*) (0x65000000 + 0x80000000);
	int *placeOfInstr = (int*) (0x66000000 + 0x80000000);
	int *vliwDataMemory = (int*) (0x0000 + 0x80000000);


};


#endif

unsigned int getInitCode(DBTPlateform *platform, int start, unsigned int startAddress);


#endif
