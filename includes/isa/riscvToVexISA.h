/*
 * riscvToVexISA.h
 *
 *  Created on: 2 déc. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_RISCVTOVEXISA_H_
#define INCLUDES_ISA_RISCVTOVEXISA_H_

ac_int<7, false> functBindingOP[8] = {VEX_ADD,VEX_SLL, VEX_CMPLT, VEX_CMPLTU, VEX_XOR, 0, VEX_OR, VEX_AND};
ac_int<7, false> functBindingOPI[8] = {VEX_ADDi,VEX_SLLi, VEX_CMPLTi, VEX_CMPLTUi, VEX_XORi,0, VEX_ORi, VEX_ANDi};
ac_int<7, false> functBindingLD[8] = {VEX_LDB, VEX_LDH, VEX_LDW, 0, VEX_LDBU, VEX_LDHU};
ac_int<7, false> functBindingST[8] = {VEX_STB, VEX_STH, VEX_STW};
ac_int<7, false> functBindingBR[8] = {VEX_CMPEQ, VEX_CMPNE, 0,0, VEX_CMPLT, VEX_CMPGE, VEX_CMPLTU, VEX_CMPGEU};
ac_int<7, false> functBindingMULT[8] = {VEX_MPYLO,VEX_MPYHI, VEX_MPYHI, VEX_MPYHI, VEX_DIVHI, VEX_DIVHI, VEX_DIVLO, VEX_DIVLO};

//FIXME: unsigned mult operations are not handled correctly



#endif /* INCLUDES_ISA_RISCVTOVEXISA_H_ */
