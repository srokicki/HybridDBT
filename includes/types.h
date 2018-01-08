
#define MAX_ISSUE_WIDTH 8

#ifndef __TYPES
#define __TYPES

#ifdef __USE_AC

#ifdef __CATAPULT
#include <ac_int.h>
#else
#include <lib/ac_int.h>
#endif
using namespace ac_intN;

typedef ac_int<MAX_ISSUE_WIDTH * 4, false> uintIW;
typedef ac_int<128, false> uint128;
typedef ac_int<64, false> uint64;


#endif


struct uint128_struct {
	unsigned int word96;
	unsigned int word64;
	unsigned int word32;
	unsigned int word0;
};

#ifndef __USE_AC
typedef unsigned int uint32;
typedef unsigned int uint31;
typedef unsigned int uint30;
typedef unsigned int uint29;
typedef unsigned int uint28;
typedef unsigned int uint27;
typedef unsigned int uint26;
typedef unsigned int uint25;
typedef unsigned int uint24;
typedef unsigned int uint23;
typedef unsigned int uint22;
typedef unsigned int uint21;
typedef unsigned int uint20;
typedef unsigned int uint19;
typedef unsigned int uint18;
typedef unsigned int uint17;
typedef unsigned short uint16;
typedef unsigned short uint15;
typedef unsigned short uint14;
typedef unsigned short uint13;
typedef unsigned short uint12;
typedef unsigned short uint11;
typedef unsigned short uint10;
typedef unsigned short uint9;
typedef unsigned char uint8;
typedef unsigned char uint7;
typedef unsigned char uint6;
typedef unsigned char uint5;
typedef unsigned char uint4;
typedef unsigned char uint3;
typedef unsigned char uint2;
typedef unsigned char uint1;

typedef int int32;
typedef int int31;
typedef int int30;
typedef int int29;
typedef int int28;
typedef int int27;
typedef int int26;
typedef int int25;
typedef int int24;
typedef int int23;
typedef int int22;
typedef int int21;
typedef int int20;
typedef int int19;
typedef int int18;
typedef int int17;
typedef short int16;
typedef short int15;
typedef short int14;
typedef short int13;
typedef short int12;
typedef short int11;
typedef short int10;
typedef short int9;
typedef char int8;
typedef char int7;
typedef char int6;
typedef char int5;
typedef char int4;
typedef char int3;
typedef char int2;
typedef char int1;


typedef unsigned int uintIW;
#endif

#endif
