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

#define MEMORY_SIZE 65536


#include <simulator/vexSimulator.h>
#include <types.h>

#ifndef __NIOS
class DBTPlateform
{

public:
	ac_int<128, false> vliwBinaries[MEMORY_SIZE];
	ac_int<32, false> mipsBinaries[MEMORY_SIZE];
	ac_int<32, false> insertions[MEMORY_SIZE];
	ac_int<1, false> blockBoundaries[MEMORY_SIZE];
	ac_int<16, true> procedureBoundaries[MEMORY_SIZE];
	ac_int<128, false> bytecode[256];
	ac_int<32, true> globalVariables[64];
	ac_int<32, false> unresolvedJumps_src[512];
	ac_int<8, false> unresolvedJumps_type[512];
	ac_int<32, true> unresolvedJumps[512];
	ac_int<6, false> placeOfRegisters[512];
	ac_int<6, false> freeRegisters[64];
	ac_int<32, false> placeOfInstr[256];

	VexSimulator* vexSimulator;
};
#endif


#ifdef __NIOS
class DBTPlateform
{

public:
	unsigned int *vliwBinaries = (unsigned int*) 0x20000000;
	unsigned int *mipsBinaries = (unsigned int*) 0x10000000;
	unsigned int *insertions = (unsigned int*) 0x30000000;
	char *blockBoundaries = (char*) 0x40000000;
	short *procedureBoundaries = (short*) 0x50000000;
	unsigned int *bytecode = (unsigned int*) 0x70000000;
	int *globalVariables = (int*) 0x63000000;
	int *unresolvedJumps_src = (int*) 0x60000000;
	char *unresolvedJumps_type = (char*) 0x610000000;
	int *unresolvedJumps = (int*) 0x620000000;
	char *placeOfRegisters = (char) 0x640000000;
	char *freeRegisters = (char*) 0x650000000;
	int *placeOfInstr = (int*) 0x660000000;

};

#define ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0_N,(A),(B))
#define ALT_CI_COMPONENT_FIRSTPASSTRANSLATORRISCV_HW_0_N 0x0
#define ALT_CI_COMPONENT_IRGENERATOR_HW_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_IRGENERATOR_HW_0_N,(A),(B))
#define ALT_CI_COMPONENT_IRGENERATOR_HW_0_N 0x1
#define ALT_CI_COMPONENT_SCHEDULING_0(A,B) __builtin_custom_inii(ALT_CI_COMPONENT_SCHEDULING_0_N,(A),(B))
#define ALT_CI_COMPONENT_SCHEDULING_0_N 0x2

#endif

unsigned int getInitCode(DBTPlateform *platform, int start, unsigned int startAddress);


#endif
