
#define MAX_ISSUE_WIDTH 8

#ifndef __TYPES
#define __TYPES

#ifdef __USE_AC
#include <lib/ac_int.h>
using namespace ac_intN;

typedef ac_int<MAX_ISSUE_WIDTH * 2, false> uintIW;
typedef ac_int<128, false> uint128;
typedef ac_int<64, false> uint64;


#endif


#ifndef __USE_AC
typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned char uint6;
typedef unsigned char uint4;
typedef unsigned char uint3;
typedef unsigned char uint1;

typedef unsigned short uintIW;
#endif

#endif
