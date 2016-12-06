/*
 * riscvISA.h
 *
 *  Created on: 2 déc. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_RISCVISA_H_
#define INCLUDES_ISA_RISCVISA_H_

#define RISCV_LUI 0x37
#define RISCV_AUIPC 0x17
#define RISCV_JAL 0x6f
#define RISCV_JALR 0x67
#define RISCV_BR 0x63
#define RISCV_LD 0x3
#define RISCV_ST 0x23
#define RISCV_OPI 0x13
#define RISCV_OP 0x33


#define RISCV_BR_BEQ 0x0
#define RISCV_BR_BNE 0x1
#define RISCV_BR_BLT 0x4
#define RISCV_BR_BGE 0x5
#define RISCV_BR_BLTU 0x6
#define RISCV_BR_BGEU 0x7


#define RISCV_LD_LB 0x0
#define RISCV_LD_LH 0x1
#define RISCV_LD_LW 0x2
#define RISCV_LD_LBU 0x4
#define RISCV_LD_LHU 0x5

#define RISCV_ST_STB 0x0
#define RISCV_ST_STH 0x1
#define RISCV_ST_STW 0x2


#define RISCV_OPI_ADDI 0x0
#define RISCV_OPI_SLTI 0x2
#define RISCV_OPI_SLTIU 0x3
#define RISCV_OPI_XORI 0x4
#define RISCV_OPI_ORI 0x6
#define RISCV_OPI_ANDI 0x7
#define RISCV_OPI_SLLI 0x1
#define RISCV_OPI_SRI 0x5

#define RISCV_OPI_SRI_SRAI 0x0
#define RISCV_OPI_SRI_SRLI 0x20

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


#endif /* INCLUDES_ISA_RISCVISA_H_ */
