/*
 * riscvToVexISA.cpp
 *
 *  Created on: 4 janv. 2018
 *      Author: simon
 */

#include <isa/vexISA.h>
#include <isa/riscvToVexISA.h>

#ifdef __USE_AC
ac_int<7, false> functBindingOP[8] = {VEX_ADD,VEX_SLL, VEX_CMPLT, VEX_CMPLTU, VEX_XOR, 0, VEX_OR, VEX_AND};
ac_int<7, false> functBindingOPI[8] = {VEX_ADDi,VEX_SLLi, VEX_CMPLTi, VEX_CMPLTUi, VEX_XORi,0, VEX_ORi, VEX_ANDi};
ac_int<7, false> functBindingLD[8] = {VEX_LDB, VEX_LDH, VEX_LDW, VEX_LDD, VEX_LDBU, VEX_LDHU, VEX_LDWU};
ac_int<7, false> functBindingST[8] = {VEX_STB, VEX_STH, VEX_STW, VEX_STD};
ac_int<7, false> functBindingBR[8] = {VEX_CMPEQ, VEX_CMPNE, 0,0, VEX_CMPLT, VEX_CMPGE, VEX_CMPLTU, VEX_CMPGEU};
ac_int<7, false> functBindingMULT[8] = {VEX_MPY,VEX_MPYH, VEX_MPYHSU, VEX_MPYHU, VEX_DIV, VEX_DIVU, VEX_REM, VEX_REMU};
ac_int<7, false> functBindingMULTW[8] = {VEX_MPYW,0, 0, 0, VEX_DIVW, VEX_DIVUW, VEX_REMW, VEX_REMUW};
#else
unsigned char functBindingOP[8] = {VEX_ADD,VEX_SLL, VEX_CMPLT, VEX_CMPLTU, VEX_XOR, 0, VEX_OR, VEX_AND};
unsigned char functBindingOPI[8] = {VEX_ADDi,VEX_SLLi, VEX_CMPLTi, VEX_CMPLTUi, VEX_XORi,0, VEX_ORi, VEX_ANDi};
unsigned char functBindingLD[8] = {VEX_LDB, VEX_LDH, VEX_LDW, VEX_LDD, VEX_LDBU, VEX_LDHU, VEX_LDWU};
unsigned char functBindingST[8] = {VEX_STB, VEX_STH, VEX_STW, VEX_STD};
unsigned char functBindingBR[8] = {VEX_CMPEQ, VEX_CMPNE, 0,0, VEX_CMPLT, VEX_CMPGE, VEX_CMPLTU, VEX_CMPGEU};
unsigned char functBindingMULT[8] = {VEX_MPY,VEX_MPYH, VEX_MPYHSU, VEX_MPYHU, VEX_DIV, VEX_DIVU, VEX_REM, VEX_REMU};
unsigned char functBindingMULTW[8] = {VEX_MPYW,0, 0, 0, VEX_DIVW, VEX_DIVUW, VEX_REMW, VEX_REMUW};
#endif
