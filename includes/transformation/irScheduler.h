#include <dbt/dbtPlateform.h>
#include <parameters.h>
#include <types.h>

#ifndef __IR_SCHEDULER
#define __IR_SCHEDULER

#define MAX_ISSUE_WIDTH 8

#ifndef __SW
#ifndef __HW

ac_int<32, false> irScheduler_list_hw(ac_int<1, false> optLevel, ac_int<8, false> basicBlockSize,
                                      ac_int<128, false> bytecode[256], ac_int<128, false> binaries[65536],
                                      ac_int<32, false> addressInBinaries, ac_int<6, false> placeOfRegisters[512],
                                      ac_int<6, false> numberFreeRegister, ac_int<6, false> freeRegisters[64],
                                      ac_int<4, false> issue_width,
                                      ac_int<MAX_ISSUE_WIDTH * 4, false> way_specialisation,
                                      ac_int<32, false> placeOfInstr[256]);

ac_int<32, false> irScheduler_scoreboard_hw(ac_int<1, false> optLevel, ac_int<8, false> basicBlockSize,
                                            ac_int<128, false> bytecode[256], ac_int<128, false> binaries[1024],
                                            ac_int<32, false> addressInBinaries, ac_int<6, false> placeOfRegisters[512],
                                            ac_int<6, false> numberFreeRegister, ac_int<6, false> freeRegisters[64],
                                            ac_int<8, false> issue_width, // TODO: change to 1 boolean per issue
                                            ac_int<MAX_ISSUE_WIDTH * 4, false> way_specialisation,
                                            ac_int<32, false> placeOfInstr[256]);

#endif
#endif

unsigned int irScheduler_scoreboard_sw(bool optLevel, unsigned char basicBlockSize, unsigned int bytecode[256 * 4],
                                       unsigned int binaries[1024 * 4], unsigned int addressInBinaries,
                                       unsigned char placeOfRegisters[512], unsigned char numberFreeRegister,
                                       unsigned char freeRegisters[64],
                                       unsigned char issue_width, // TODO: change to 1 boolean per issue
                                       unsigned int way_specialisation, unsigned int placeOfInstr[256]);

#ifndef __USE_AC

unsigned int irScheduler_list_sw(unsigned char optLevel, unsigned char basicBlockSize, unsigned int bytecode[1024],
                                 unsigned int binaries[1024], unsigned char placeOfRegisters[512],
                                 unsigned char numberFreeRegister, unsigned char freeRegisters[64],
                                 unsigned char issue_width, unsigned int way_specialisation,
                                 unsigned int placeOfInstr[256]);
#endif

extern ac_int<128, false> maskVal[16];

int irScheduler(DBTPlateform* platform, bool optLevel, unsigned char basicBlockSize, unsigned int addressInBinaries,
                unsigned char numberFreeRegister, char configuration);

#ifndef __CATAPULT
// Performance simulation
extern int timeTakenIRScheduler;
#endif

#endif
