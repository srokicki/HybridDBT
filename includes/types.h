
#define MAX_ISSUE_WIDTH 8

#ifndef __TYPES
#define __TYPES

#ifdef __USE_AC
#include <lib/ac_int.h>

typedef ac_int<128, false> uint128;
typedef ac_int<128, true> int128;

typedef ac_int<32, false> uint32;
typedef ac_int<32, true> int32;


typedef ac_int<16, false> uint16;
typedef ac_int<16, true> int16;



typedef ac_int<8, false> uint8;
typedef ac_int<6, false> uint6;
typedef ac_int<4, false> uint4;
typedef ac_int<3, false> uint3;
typedef ac_int<1, false> uint1;


typedef ac_int<MAX_ISSUE_WIDTH * 2, false> uintIW;
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
