
#define MAX_ISSUE_WIDTH 8

#ifndef __TYPES
#define __TYPES

#ifdef __HW_SIM

#ifdef __CATAPULT
#include <ac_int.h>
#else
#include <lib/ac_int.h>
#endif

#endif

#ifdef __SW_HW_SIM
#include <lib/ac_int.h>
#endif

struct uint128_struct {
  unsigned int word96;
  unsigned int word64;
  unsigned int word32;
  unsigned int word0;
};

#endif
