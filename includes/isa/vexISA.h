/*
 * vex.h
 *
 *  Created on: 30 sept. 2016
 *      Author: simon
 */
#include <types.h>
#include <string>
#ifndef INCLUDES_VEX_H_
#define INCLUDES_VEX_H_

#define VEX_NOP 0x0

#define VEX_MPYW 0x3
#define VEX_DIVW 0x4
#define VEX_DIVUW 0x5
#define VEX_REMW 0x6
#define VEX_REMUW 0x7

#define VEX_MPYH 0x8
#define VEX_MPYHSU 0x9
#define VEX_MPYHU 0xa
#define VEX_MPY 0xb
#define VEX_DIV 0xc
#define VEX_DIVU 0xd
#define VEX_REM 0xe
#define VEX_REMU 0xf

#define VEX_LDD 0x10
#define VEX_LDW 0x11
#define VEX_LDH 0x12
#define VEX_LDHU 0x13
#define VEX_LDB 0x14
#define VEX_LDBU 0x15
#define VEX_LDWU 0x16
#define VEX_PROFILE 0x17

#define VEX_STB 0x18
#define VEX_STH 0x19
#define VEX_STW 0x1A
#define VEX_STD 0x1b
#define VEX_STBc 0x1c
#define VEX_STHc 0x1d
#define VEX_STWc 0x1e
#define VEX_STDc 0x1f

#define VEX_GOTO 0x21
#define VEX_GOTOR 0x22
#define VEX_CALL 0x23
#define VEX_CALLR 0x24
#define VEX_BR 0x25
#define VEX_BRF 0x26
#define VEX_MOVUI 0x27
#define VEX_MOVI 0x28
#define VEX_BLT 0x29
#define VEX_RECONFFS 0x2a
#define VEX_BGE 0x2b
#define VEX_BLTU 0x2c
#define VEX_BGEU 0x2d
#define VEX_ECALL 0x2e
#define VEX_STOP 0x2f

#define VEX_FLW 0x30
#define VEX_FLH 0x31
#define VEX_FLB 0x32
#define VEX_FSW 0x33
#define VEX_FSH 0x34
#define VEX_FSB 0x35
#define VEX_FMADD 0x36
#define VEX_FMSUB 0x37
#define VEX_FNMSUB 0x38
#define VEX_FNMADD 0x39
#define VEX_FP 0x3a

#define VEX_FP_FADD 0
#define VEX_FP_FSUB 1
#define VEX_FP_FMUL 2
#define VEX_FP_FDIV 3
#define VEX_FP_FSQRT 4
#define VEX_FP_FSGNJ 5
#define VEX_FP_FSGNJN 6
#define VEX_FP_FSGNJNX 7
#define VEX_FP_FMIN 8
#define VEX_FP_FMAX 9
#define VEX_FP_FCVTWS 10
#define VEX_FP_FCVTWUS 11
#define VEX_FP_FMVXW 12
#define VEX_FP_FEQ 13
#define VEX_FP_FLT 14
#define VEX_FP_FLE 15
#define VEX_FP_FCLASS 16
#define VEX_FP_FCVTSW 17
#define VEX_FP_FCVTSWU 18
#define VEX_FP_FMVWX 19



#define VEX_SETc 0x40
#define VEX_ADD 0x41
#define VEX_NOR 0x42
#define VEX_AND 0x43
#define VEX_ANDC 0x44
#define VEX_CMPLT 0x45
#define VEX_CMPLTU 0x46
#define VEX_CMPNE 0x47
#define VEX_NOT 0x48
#define VEX_OR 0x49
#define VEX_SETFc 0x4a
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

/*
 * Declaration of functions that assemble vex instructions.
 * These are implemented for HW or for SW.
 *
 * Note: HW version is enabled inly with __USE_AC flag
 */

#ifndef __HW
#ifndef __SW

ac_int<32, false> assembleIInstruction(ac_int<7, false> opcode, ac_int<19, false> imm19, ac_int<6, false> regA);
ac_int<32, false> assembleRInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<6, false> regB);
ac_int<32, false> assembleRiInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<13, false> imm13);
ac_int<32, false> assembleFPInstruction(ac_int<7, false> opcode, ac_int<5, false> funct, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<6, false> regB);
ac_int<32, false> assembleRRInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<6, false> regB, ac_int<6, false> regC);
#endif
#endif

unsigned int assembleIInstruction_sw(char opcode, int imm19, char regA);
unsigned int  assembleRInstruction_sw(char opcode, char regDest, char regA, char regB);
unsigned int  assembleRiInstruction_sw(char opcode, char regDest, char regA, short imm13);
unsigned int  assembleFPInstruction_sw(char opcode, char funct, char regDest, char regA, char regB);
unsigned int  assembleRRInstruction_sw(char opcode, char regDest, char regA, char regB, char regC);


#ifndef __CATAPULT
extern const char* opcodeNames[128];
std::string printDecodedInstr(unsigned int instruction);
#endif

#endif /* INCLUDES_VEX_H_ */
