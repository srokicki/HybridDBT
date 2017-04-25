/*
 * vex.h
 *
 *  Created on: 30 sept. 2016
 *      Author: simon
 */
#include <types.h>
#include <string.h>
#ifndef INCLUDES_VEX_H_
#define INCLUDES_VEX_H_

#define VEX_NOP 0x0
#define VEX_MPYLL 0x1
#define VEX_MPYLLU 0x2
#define VEX_MPYLH 0x3
#define VEX_MPYLHU 0x4
#define VEX_MPYHH 0x5
#define VEX_MPYHHU 0x6
#define VEX_MPYL 0x7
#define VEX_DIVW 0x8
#define VEX_MPYW 0x9

#define VEX_MPYHISU 0xa
#define VEX_MPYHIU 0xb
#define VEX_MPYLO 0xc
#define VEX_MPYHI 0xd
#define VEX_DIVLO 0xe
#define VEX_DIVHI 0xf

#define VEX_LDD 0x10
#define VEX_LDW 0x11
#define VEX_LDH 0x12
#define VEX_LDHU 0x13
#define VEX_LDB 0x14
#define VEX_LDBU 0x15
#define VEX_LDWU 0x16

#define VEX_STB 0x18
#define VEX_STH 0x19
#define VEX_STW 0x1A
#define VEX_STD 0x1b


#define VEX_GOTO 0x21
#define VEX_GOTOR 0x22
#define VEX_CALL 0x23
#define VEX_CALLR 0x24
#define VEX_BR 0x25
#define VEX_BRF 0x26
#define VEX_RETURN 0x27
#define VEX_MOVI 0x28
#define VEX_AUIPC 0x29
#define VEX_RECONFFS 0x2a
#define VEX_RECONFEXECUNIT 0x2b

#define VEX_ECALL 0x2e
#define VEX_STOP 0x2f
#define VEX_SLCTF 0x30

#define VEX_SLCT 0x38

#define VEX_ADD 0x41
#define VEX_NOR 0x42
#define VEX_AND 0x43
#define VEX_ANDC 0x44
#define VEX_CMPLT 0x45
#define VEX_CMPLTU 0x46
#define VEX_CMPNE 0x47
#define VEX_NOT 0x48
#define VEX_OR 0x49
#define VEX_ORC 0x4a
#define VEX_SH1ADD 0x4b
#define VEX_SH2ADD 0x4c
#define VEX_SH3ADD 0x4d
#define VEX_SH4ADD 0x4e
#define VEX_SLL 0x4f
#define VEX_SRL 0x50
#define VEX_SRA 0x51
#define VEX_SUB 0x52
#define VEX_XOR 0x53
#define VEX_ADDW 0x54
#define VEX_SUBW 0x55
#define VEX_SLLW 0x56
#define VEX_SRLW 0x57
#define VEX_SRAW 0x58
#define VEX_CMPEQ 0x59
#define VEX_CMPGE 0x5a
#define VEX_CMPGEU 0x5b
#define VEX_CMPGT 0x5c
#define VEX_CMPGTU 0x5d
#define VEX_CMPLE 0x5e
#define VEX_CMPLEU 0x5f

#define VEX_ADDi 0x61
#define VEX_NORi 0x62
#define VEX_ANDi 0x63
#define VEX_ANDCi 0x64
#define VEX_CMPLTi 0x65
#define VEX_CMPLTUi 0x66
#define VEX_CMPNEi 0x67
#define VEX_NOTi 0x68
#define VEX_ORi 0x69
#define VEX_ORCi 0x6a
#define VEX_SH1ADDi 0x6b
#define VEX_SH2ADDi 0x6c
#define VEX_SH3ADDi 0x6d
#define VEX_SH4ADDi 0x6e
#define VEX_SLLi 0x6f
#define VEX_SRLi 0x70
#define VEX_SRAi 0x71
#define VEX_SUBi 0x72
#define VEX_XORi 0x73
#define VEX_ADDWi 0x74

#define VEX_SLLWi 0x76
#define VEX_SRLWi 0x77
#define VEX_SRAWi 0x78
#define VEX_CMPEQi 0x79
#define VEX_CMPGEi 0x7a
#define VEX_CMPGEUi 0x7b
#define VEX_CMPGTi 0x7c
#define VEX_CMPGTUi 0x7d
#define VEX_CMPLEi 0x7e
#define VEX_CMPLEUi 0x7f

uint32 assembleIInstruction(uint7 opcode, uint19 imm19, uint6 regA);
uint32 assembleRInstruction(uint7 opcode, uint6 regDest, uint6 regA, uint6 regB);
uint32 assembleRiInstruction(uint7 opcode, uint6 regDest, uint6 regA, uint13 imm13);

#ifndef __CATAPULT
#ifndef __NIOS
extern const char* opcodeNames[128];
std::string printDecodedInstr(ac_int<32, false> instruction);
#endif
#endif

#endif /* INCLUDES_VEX_H_ */
