/*
 * mipsToVexISA.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_MIPSTOVEXISA_H_
#define INCLUDES_ISA_MIPSTOVEXISA_H_

#include <isa/vexISA.h>
#include <types.h>

#ifdef __HW_SIM
const uint7 opcodeBinding[64] = {0,           // R
                                 0,           // REGIMM
                                 VEX_GOTO,    // J
                                 VEX_CALL,    // JAL
                                 VEX_CMPEQ,   // BEQ
                                 VEX_CMPNE,   // BNE
                                 VEX_CMPLE,   // BLEZ
                                 VEX_CMPGT,   // BGTZ
                                 VEX_ADDi,    // ADDI
                                 VEX_ADDi,    // ADDIU
                                 VEX_CMPLTi,  // SLTI
                                 VEX_CMPLTUi, // SLTIU
                                 VEX_ANDi,    // ANDI
                                 VEX_ORi,     // ORI
                                 VEX_XORi,    // XORI
                                 0,           // LUI

                                 0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

                                 VEX_LDB,  // LB
                                 VEX_LDH,  // LH
                                 0,        // LWL
                                 VEX_LDW,  // LW
                                 VEX_LDBU, // LBU
                                 VEX_LDHU, // LHU
                                 0,        // LWR
                                 0,        // LWU
                                 VEX_STB,  // SB
                                 VEX_STH,  // SH
                                 0,        // SWL
                                 VEX_STW,  // SW
                                 0,        // SDL
                                 0,        // SDR
                                 0,        // SWR
                                 0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const uint7 functBinding[64] = {VEX_SLLi, // SLL
                                0,
                                VEX_SRLi, // SRL
                                VEX_SRAi, // SRA
                                VEX_SLL,  // SLLV
                                0,
                                VEX_SRL,  // SRLV
                                VEX_SRA,  // SRAV
                                VEX_GOTO, // JR
                                VEX_CALL, // JALR
                                0,        // MOVZ
                                0,        // MOVN
                                0,
                                VEX_STOP, // BREAK
                                0,          0,

                                0, //	VEX_MPYHI,		//MFHI
                                0, // MTHI
                                0, //	VEX_MPYLO,		//MFLO
                                0, // MTLO
                                0,          0, 0, 0,
                                -3, // MULT
                                -3, // MULTU
                                -2, // DIV
                                -2, // DIVU
                                0,          0, 0, 0,

                                VEX_ADD, // ADD
                                VEX_ADD, // ADDU
                                VEX_SUB, // SUB
                                VEX_SUB, // SUBU
                                VEX_AND, // AND
                                VEX_OR,  // OR
                                VEX_XOR, // XOR
                                VEX_NOR, // NOR
                                0,          0,
                                VEX_CMPLT,  // SLT
                                VEX_CMPLTU, // SLTU
                                0,          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char regimmBindings[64] = {VEX_CMPLT, VEX_CMPGE};
#endif

#ifndef __USE_AC
const char opcodeBinding[64] = {0,           // R
                                0,           // REGIMM
                                VEX_GOTO,    // J
                                VEX_CALL,    // JAL
                                VEX_CMPEQ,   // BEQ
                                VEX_CMPNE,   // BNE
                                VEX_CMPLE,   // BLEZ
                                VEX_CMPGT,   // BGTZ
                                VEX_ADDi,    // ADDI
                                VEX_ADDi,    // ADDIU
                                VEX_CMPLTi,  // SLTI
                                VEX_CMPLTUi, // SLTIU
                                VEX_ANDi,    // ANDI
                                VEX_ORi,     // ORI
                                VEX_XORi,    // XORI
                                0,           // LUI

                                0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

                                VEX_LDB,  // LB
                                VEX_LDH,  // LH
                                0,        // LWL
                                VEX_LDW,  // LW
                                VEX_LDBU, // LBU
                                VEX_LDHU, // LHU
                                0,        // LWR
                                0,        // LWU
                                VEX_STB,  // SB
                                VEX_STH,  // SH
                                0,        // SWL
                                VEX_STW,  // SW
                                0,        // SDL
                                0,        // SDR
                                0,        // SWR
                                0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char functBinding[64] = {VEX_SLLi, // SLL
                               0,
                               VEX_SRLi, // SRL
                               VEX_SRAi, // SRA
                               VEX_SLL,  // SLLV
                               0,
                               VEX_SRL,  // SRLV
                               VEX_SRA,  // SRAV
                               VEX_GOTO, // JR
                               VEX_CALL, // JALR
                               0,        // MOVZ
                               0,        // MOVN
                               0,
                               VEX_STOP, // BREAK
                               0,          0,

                               VEX_MPYHI, // MFHI
                               0,         // MTHI
                               VEX_MPYLO, // MFLO
                               0,         // MTLO
                               0,          0, 0, 0,
                               -3, // MULT
                               -3, // MULTU
                               -2, // DIV
                               -2, // DIVU
                               0,          0, 0, 0,

                               VEX_ADD, // ADD
                               VEX_ADD, // ADDU
                               VEX_SUB, // SUB
                               VEX_SUB, // SUBU
                               VEX_AND, // AND
                               VEX_OR,  // OR
                               VEX_XOR, // XOR
                               VEX_NOR, // NOR
                               0,          0,
                               VEX_CMPLT,  // SLT
                               VEX_CMPLTU, // SLTU
                               0,          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char regimmBindings[64] = {VEX_CMPLT, VEX_CMPGE};
#endif

#endif /* INCLUDES_ISA_MIPSTOVEXISA_H_ */
