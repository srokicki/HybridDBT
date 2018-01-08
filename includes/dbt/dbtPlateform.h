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

#define MEMORY_SIZE 190000


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

	unsigned int vliwBinaries[4*MEMORY_SIZE];
	unsigned int mipsBinaries[4*MEMORY_SIZE];
	unsigned int insertions[2048];
	unsigned char blockBoundaries[MEMORY_SIZE];
	unsigned int bytecode[256*4];
	int globalVariables[128];
	unsigned int unresolvedJumps_src[512];
	unsigned int unresolvedJumps_type[512];
	int unresolvedJumps[512];
	unsigned char placeOfRegisters[512];
	unsigned char freeRegisters[64];
	unsigned int placeOfInstr[256];


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

//#define ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0_N,(A),(B))
//#define ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0_N 0x0
//#define ALT_CI_COMPONENT_IRGENERATOR_HW_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_IRGENERATOR_HW_0_N,(A),(B))
//#define ALT_CI_COMPONENT_IRGENERATOR_HW_0_N 0x1
//#define ALT_CI_COMPONENT_SCHEDULING_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_SCHEDULING_0_N,(A),(B))
//#define ALT_CI_COMPONENT_SCHEDULING_0_N 0x2

#include <system.h>

#endif

unsigned int getInitCode(DBTPlateform *platform, int start, unsigned int startAddress);


#endif
