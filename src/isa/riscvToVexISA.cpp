/*
 * riscvToVexISA.cpp
 *
 *  Created on: 4 janv. 2018
 *      Author: simon
 */

#include <isa/vexISA.h>
#include <isa/riscvToVexISA.h>

uint7 functBindingOP[8] = {VEX_ADD,VEX_SLL, VEX_CMPLT, VEX_CMPLTU, VEX_XOR, 0, VEX_OR, VEX_AND};
uint7 functBindingOPI[8] = {VEX_ADDi,VEX_SLLi, VEX_CMPLTi, VEX_CMPLTUi, VEX_XORi,0, VEX_ORi, VEX_ANDi};
uint7 functBindingLD[8] = {VEX_LDB, VEX_LDH, VEX_LDW, VEX_LDD, VEX_LDBU, VEX_LDHU, VEX_LDWU};
uint7 functBindingST[8] = {VEX_STB, VEX_STH, VEX_STW, VEX_STD};
uint7 functBindingBR[8] = {VEX_CMPEQ, VEX_CMPNE, 0,0, VEX_CMPLT, VEX_CMPGE, VEX_CMPLTU, VEX_CMPGEU};
uint7 functBindingMULT[8] = {VEX_MPY,VEX_MPYH, VEX_MPYHSU, VEX_MPYHU, VEX_DIV, VEX_DIVU, VEX_REM, VEX_REMU};
uint7 functBindingMULTW[8] = {VEX_MPYW,0, 0, 0, VEX_DIVW, VEX_DIVUW, VEX_REMW, VEX_REMUW};

