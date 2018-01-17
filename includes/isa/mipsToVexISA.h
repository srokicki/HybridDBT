/*
 * mipsToVexISA.h
 *
 *  Created on: 10 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_ISA_MIPSTOVEXISA_H_
#define INCLUDES_ISA_MIPSTOVEXISA_H_

#include <types.h>
#include <isa/vexISA.h>


#ifdef __USE_AC
const uint7 opcodeBinding[64] = {
		-1,  	//R
		-1,  	//REGIMM
		VEX_GOTO,	//J
		VEX_CALL,	//JAL
		VEX_CMPEQ,	//BEQ
		VEX_CMPNE,	//BNE
		VEX_CMPLE,	//BLEZ
		VEX_CMPGT,	//BGTZ
		VEX_ADDi,	//ADDI
		VEX_ADDi,	//ADDIU
		VEX_CMPLTi,	//SLTI
		VEX_CMPLTUi,	//SLTIU
		VEX_ANDi,	//ANDI
		VEX_ORi,	//ORI
		VEX_XORi,	//XORI
		-1,		//LUI

		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,

		VEX_LDB,	//LB
		VEX_LDH,	//LH
		-1,		//LWL
		VEX_LDW,	//LW
		VEX_LDBU,	//LBU
		VEX_LDHU,	//LHU
		-1,		//LWR
		-1,		//LWU
		VEX_STB,	//SB
		VEX_STH,	//SH
		-1,		//SWL
		VEX_STW,	//SW
		-1,		//SDL
		-1,		//SDR
		-1,		//SWR
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
		};

const uint7 functBinding[64] = {
		VEX_SLLi,	//SLL
		-1,
		VEX_SRLi,	//SRL
		VEX_SRAi,	//SRA
		VEX_SLL,	//SLLV
		-1,
		VEX_SRL,	//SRLV
		VEX_SRA,	//SRAV
		VEX_GOTO,	//JR
		VEX_CALL,	//JALR
		-1,		//MOVZ
		-1,		//MOVN
		-1,
		VEX_STOP,		//BREAK
		-1,
		-1,

	0,//	VEX_MPYHI,		//MFHI
		-1,		//MTHI
	0,//	VEX_MPYLO,		//MFLO
		-1,		//MTLO
		-1,
		-1,
		-1,
		-1,
		-3,		//MULT
		-3,		//MULTU
		-2,		//DIV
		-2,		//DIVU
		-1,
		-1,
		-1,
		-1,

		VEX_ADD,	//ADD
		VEX_ADD,	//ADDU
		VEX_SUB,	//SUB
		VEX_SUB,	//SUBU
		VEX_AND,	//AND
		VEX_OR,		//OR
		VEX_XOR,	//XOR
		VEX_NOR,	//NOR
		-1,
		-1,
		VEX_CMPLT,	//SLT
		VEX_CMPLTU,	//SLTU
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
};



const char regimmBindings[64] = {VEX_CMPLT, VEX_CMPGE};
#endif

#ifndef __USE_AC
const char opcodeBinding[64] = {
		-1,  	//R
		-1,  	//REGIMM
		VEX_GOTO,	//J
		VEX_CALL,	//JAL
		VEX_CMPEQ,	//BEQ
		VEX_CMPNE,	//BNE
		VEX_CMPLE,	//BLEZ
		VEX_CMPGT,	//BGTZ
		VEX_ADDi,	//ADDI
		VEX_ADDi,	//ADDIU
		VEX_CMPLTi,	//SLTI
		VEX_CMPLTUi,	//SLTIU
		VEX_ANDi,	//ANDI
		VEX_ORi,	//ORI
		VEX_XORi,	//XORI
		-1,		//LUI

		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,

		VEX_LDB,	//LB
		VEX_LDH,	//LH
		-1,		//LWL
		VEX_LDW,	//LW
		VEX_LDBU,	//LBU
		VEX_LDHU,	//LHU
		-1,		//LWR
		-1,		//LWU
		VEX_STB,	//SB
		VEX_STH,	//SH
		-1,		//SWL
		VEX_STW,	//SW
		-1,		//SDL
		-1,		//SDR
		-1,		//SWR
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
		};

const char functBinding[64] = {
		VEX_SLLi,	//SLL
		-1,
		VEX_SRLi,	//SRL
		VEX_SRAi,	//SRA
		VEX_SLL,	//SLLV
		-1,
		VEX_SRL,	//SRLV
		VEX_SRA,	//SRAV
		VEX_GOTO,	//JR
		VEX_CALL,	//JALR
		-1,		//MOVZ
		-1,		//MOVN
		-1,
		VEX_STOP,		//BREAK
		-1,
		-1,

		VEX_MPYHI,		//MFHI
		-1,		//MTHI
		VEX_MPYLO,		//MFLO
		-1,		//MTLO
		-1,
		-1,
		-1,
		-1,
		-3,		//MULT
		-3,		//MULTU
		-2,		//DIV
		-2,		//DIVU
		-1,
		-1,
		-1,
		-1,

		VEX_ADD,	//ADD
		VEX_ADD,	//ADDU
		VEX_SUB,	//SUB
		VEX_SUB,	//SUBU
		VEX_AND,	//AND
		VEX_OR,		//OR
		VEX_XOR,	//XOR
		VEX_NOR,	//NOR
		-1,
		-1,
		VEX_CMPLT,	//SLT
		VEX_CMPLTU,	//SLTU
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
};



const char regimmBindings[64] = {VEX_CMPLT, VEX_CMPGE};
#endif



#endif /* INCLUDES_ISA_MIPSTOVEXISA_H_ */
