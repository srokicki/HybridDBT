/*
 * riscvISA.h
 *
 *  Created on: 2 déc. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_RISCVISA_H_
#define INCLUDES_ISA_RISCVISA_H_

#include <types.h>
#include <string.h>

#define RISCV_LUI 0x37
#define RISCV_AUIPC 0x17
#define RISCV_JAL 0x6f
#define RISCV_JALR 0x67
#define RISCV_BR 0x63
#define RISCV_LD 0x3
#define RISCV_ST 0x23
#define RISCV_OPI 0x13
#define RISCV_OP 0x33
#define RISCV_OPIW 0x1b
#define RISCV_OPW 0x3b

#define RISCV_BR_BEQ 0x0
#define RISCV_BR_BNE 0x1
#define RISCV_BR_BLT 0x4
#define RISCV_BR_BGE 0x5
#define RISCV_BR_BLTU 0x6
#define RISCV_BR_BGEU 0x7


#define RISCV_LD_LB 0x0
#define RISCV_LD_LH 0x1
#define RISCV_LD_LW 0x2
#define RISCV_LD_LD 0x3
#define RISCV_LD_LBU 0x4
#define RISCV_LD_LHU 0x5
#define RISCV_LD_LWU 0x6


#define RISCV_ST_STB 0x0
#define RISCV_ST_STH 0x1
#define RISCV_ST_STW 0x2
#define RISCV_ST_STD 0x3



#define RISCV_OPI_ADDI 0x0
#define RISCV_OPI_SLTI 0x2
#define RISCV_OPI_SLTIU 0x3
#define RISCV_OPI_XORI 0x4
#define RISCV_OPI_ORI 0x6
#define RISCV_OPI_ANDI 0x7
#define RISCV_OPI_SLLI 0x1
#define RISCV_OPI_SRI 0x5

#define RISCV_OPI_SRI_SRAI 0x20
#define RISCV_OPI_SRI_SRLI 0x0

#define RISCV_OP_ADD 0x0
#define RISCV_OP_SLL 0x1
#define RISCV_OP_SLT 0x2
#define RISCV_OP_SLTU 0x3
#define RISCV_OP_XOR 0x4
#define RISCV_OP_SR 0x5
#define RISCV_OP_OR 0x6
#define RISCV_OP_AND 0x7

#define RISCV_OP_ADD_ADD 0x0
#define RISCV_OP_ADD_SUB 0x20

#define RISCV_OP_SR_SRL 0x0
#define RISCV_OP_SR_SRA 0x20

#define RISCV_SYSTEM 0x73

#define RISCV_OPIW_ADDIW 0x0
#define RISCV_OPIW_SLLIW 0x1
#define RISCV_OPIW_SRW 0x5

#define RISCV_OPIW_SRW_SRLIW 0x0
#define RISCV_OPIW_SRW_SRAIW 0x20


#define RISCV_OPW_ADDSUBW 0x0
#define RISCV_OPW_SLLW 0x1
#define RISCV_OPW_SRW 0x5

#define RISCV_OPW_ADDSUBW_ADDW 0x0
#define RISCV_OPW_ADDSUBW_SUBW 0x20

#define RISCV_OPW_SRW_SRLW 0x0
#define RISCV_OPW_SRW_SRAW 0x20



#define RISCV_SYSTEM_ENV 0x0
#define RISCV_SYSTEM_ENV_ECALL 0x0
#define RISCV_SYSTEM_ENV_EBREAK 0x1


#define RISCV_SYSTEM_CSRRW 0x1
#define RISCV_SYSTEM_CSRRS 0x2
#define RISCV_SYSTEM_CSRRC 0x3
#define RISCV_SYSTEM_CSRRWI 0x5
#define RISCV_SYSTEM_CSRRSI 0x6
#define RISCV_SYSTEM_CSRRCI 0x7


//FIXME some special operations of the base instruction set are not yet supported. (FENCE)

/******************************************************************************************************
* Specification of the standard M extension
********************************************
* This extension brings the support for multiplication operation.
* It is composed of the RISCV_OP opcode then a dedicated value for funct7 which identify it.
* Then funct3 is used to determine which of the eight operation to use.
* Added operations are MUL, MULH, MULHSU, MLHU, DIV, DIVU, REM, REMU
*****************************************************************************************************/

#define RISCV_OP_M 0x1

#define RISCV_OP_M_MUL 0x0
#define RISCV_OP_M_MULH 0x1
#define RISCV_OP_M_MULHSU 0x2
#define RISCV_OP_M_MULHU 0x3
#define RISCV_OP_M_DIV 0x4
#define RISCV_OP_M_DIVU 0x5
#define RISCV_OP_M_REM 0x6
#define RISCV_OP_M_REMU 0x7

#define RISCV_OPW_M_MULW 0x0
#define RISCV_OPW_M_DIVW 0x4
#define RISCV_OPW_M_DIVUW 0x5
#define RISCV_OPW_M_REMW 0x6
#define RISCV_OPW_M_REMUW 0x7


#ifndef __CATAPULT
std::string printDecodedInstrRISCV(ac_int<32, false> instruction);

extern const char* riscvNamesOP[8];
extern const char* riscvNamesOPI[8];
extern const char* riscvNamesOPW[8];
extern const char* riscvNamesOPIW[8];
extern const char* riscvNamesLD[8];
extern const char* riscvNamesST[8];
extern const char* riscvNamesBR[8];
extern const char* riscvNames[8];
#endif


#endif /* INCLUDES_ISA_RISCVISA_H_ */
