/*
 * riscvSimulator.cpp
 *
 *  Created on: 2 déc. 2016
 *      Author: Simon Rokicki
 */

#ifndef __NIOS

#include <isa/riscvISA.h>
#include <simulator/riscvSimulator.h>

#include <types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <map>

ac_int<64, false> shiftMask[64];
float regf[32];
#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)


int RiscvSimulator::doSimulation(int nbkCycle){
	//We initialize shiftmask
	ac_int<64, false> value = 0xffffffff;
	value = (value << 32) + value;
	for (int i=0; i<64; i++){
		shiftMask[i] = value;
		value = value >> 1;
	}

	//We initialize instruction counter
	n_inst = 0;

	do{
		this->doStep();
	}
	while (stop != 1 && this->cycle<20000000000);

	if (this->stop)
		return 0;
	else
		return -1;

}

void RiscvSimulator::doStep(){


	int storedVerbose = this->debugLevel;

	/*Fetching new instruction */
	ac_int<32, false> ins = this->ldw(pc);




	//Ignoring cache
	ac_int<8, true> result0 = 0;
	if (this->memory.find(pc) != this->memory.end())
		result0 = this->memory[pc];
	else
		result0= 0;

	ins.set_slc(0, result0);


	if (this->memory.find(pc+1) != this->memory.end())
		result0 = this->memory[pc+1];
	else
		result0= 0;

	ins.set_slc(8, result0);

	if (this->memory.find(pc+2) != this->memory.end())
		result0 = this->memory[pc+2];
	else
		result0= 0;

	ins.set_slc(16, result0);


	if (this->memory.find(pc+3) != this->memory.end())
		result0 = this->memory[pc+3];
	else
		result0= 0;


	ins.set_slc(24, result0);



	if (this->debugLevel>1){
		fprintf(stderr,"%d;%x;%x", (int)cycle, (int)pc, (int) ins);
		std::cerr << printDecodedInstrRISCV(ins);
	}

	pc = pc + 4;


	//We decode the instruction to execute
	ac_int<7, false> opcode = ins.slc<7>(0);
	ac_int<5, false> rs1 = ins.slc<5>(15);
	ac_int<5, false> rs2 = ins.slc<5>(20);
	ac_int<5, false> rs3 = ins.slc<5>(27);

	ac_int<5, false> rd = ins.slc<5>(7);
	ac_int<7, false> funct7 = ins.slc<7>(25);
	ac_int<7, false> funct7_smaller = 0;
	funct7_smaller.set_slc(1, ins.slc<6>(26));

	ac_int<3, false> funct3 = ins.slc<3>(12);
	ac_int<12, false> imm12_I = ins.slc<12>(20);
	ac_int<12, false> imm12_S = 0;
	imm12_S.set_slc(5, ins.slc<7>(25));
	imm12_S.set_slc(0, ins.slc<5>(7));

	ac_int<12, true> imm12_I_signed = ins.slc<12>(20);
	ac_int<12, true> imm12_S_signed = 0;
	imm12_S_signed.set_slc(0, imm12_S.slc<12>(0));


	ac_int<13, false> imm13 = 0;
	imm13[12] = ins[31];
	imm13.set_slc(5, ins.slc<6>(25));
	imm13.set_slc(1, ins.slc<4>(8));
	imm13[11] = ins[7];

	ac_int<13, true> imm13_signed = 0;
	imm13_signed.set_slc(0, imm13);

	ac_int<32, true> imm31_12 = 0;
	imm31_12.set_slc(12, ins.slc<20>(12));

	ac_int<21, false> imm21_1 = 0;
	imm21_1.set_slc(12, ins.slc<8>(12));
	imm21_1[11] = ins[20];
	imm21_1.set_slc(1, ins.slc<10>(21));
	imm21_1[20] = ins[31];

	ac_int<21, true> imm21_1_signed = 0;
	imm21_1_signed.set_slc(0, imm21_1);

	ac_int<6, false> shamt = ins.slc<6>(20);


	ac_int<64, false> unsignedReg1 = 0;
	ac_int<64, false> unsignedReg2 = 0;

	ac_int<64, true> valueRegA;

	ac_int<128, true> longResult;


	ac_int<32, true> localDataa, localDatab;
	ac_int<64, true> localLongResult;
	ac_int<32, false> localDataaUnsigned, localDatabUnsigned;
	ac_int<32, true> localResult;
	unsigned int localInt;

	float localFloat;

	this->lastWrittenRegister = -1;
	this->lastIsLoad = false;

	//According to opcode/funct3/funct7 we perform the correct operation
	switch (opcode)
	{
	case RISCV_LUI:
		this->lastWrittenRegister = rd;
		REG[rd] = imm31_12;
	break;
	case RISCV_AUIPC:
		this->lastWrittenRegister = rd;
		REG[rd] = pc -4 + imm31_12;
	break;
	case RISCV_JAL:
		this->lastWrittenRegister = rd;

		REG[rd] = pc;
		pc = pc - 4 + imm21_1_signed;
		this->cycle += LOSS_INCORRECT_BRANCH;
	break;
	case RISCV_JALR:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}

		localResult = pc;
		pc = (REG[rs1] + imm12_I_signed) & 0xfffffffe;
		REG[rd] = localResult;
		this->cycle += LOSS_INCORRECT_BRANCH;
	break;

	//******************************************************************************************
	//Treatment for: BRANCH INSTRUCTIONS
	case RISCV_BR:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister || rs2 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}

		switch(funct3)
		{
		case RISCV_BR_BEQ:
			if (REG[rs1] == REG[rs2]){
				pc = pc + (imm13_signed) - 4;
				this->cycle += LOSS_INCORRECT_BRANCH;
			}
		break;
		case RISCV_BR_BNE:
			if (REG[rs1] != REG[rs2]){
				pc = pc + (imm13_signed) - 4;
				this->cycle += LOSS_INCORRECT_BRANCH;
			}
		break;
		case RISCV_BR_BLT:
			if (REG[rs1] < REG[rs2]){
				pc = pc + (imm13_signed) - 4;
				this->cycle += LOSS_INCORRECT_BRANCH;
			}
		break;
		case RISCV_BR_BGE:
			if (REG[rs1] >= REG[rs2]){
				pc = pc + (imm13_signed) - 4;
				this->cycle += LOSS_INCORRECT_BRANCH;
			}
		break;
		case RISCV_BR_BLTU:
			unsignedReg1.set_slc(0, REG[rs1].slc<64>(0));
			unsignedReg2.set_slc(0, REG[rs2].slc<64>(0));

			if (unsignedReg1 < unsignedReg2){
				pc = pc + (imm13_signed) - 4;
				this->cycle += LOSS_INCORRECT_BRANCH;
			}
		break;
		case RISCV_BR_BGEU:
			unsignedReg1.set_slc(0, REG[rs1].slc<64>(0));
			unsignedReg2.set_slc(0, REG[rs2].slc<64>(0));

			if (unsignedReg1 >= unsignedReg2){
				pc = pc + (imm13_signed) - 4;
				this->cycle += LOSS_INCORRECT_BRANCH;
			}
		break;
		default:
			printf("In BR switch case, this should never happen... Instr was %x\n", (int)ins);
			exit(-1);
		break;
		}
	break;

	//******************************************************************************************
	//Treatment for: LOAD INSTRUCTIONS
	case RISCV_LD:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister || rs2 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastIsLoad = true;
		this->lastWrittenRegister = rd;


		switch(funct3)
		{
		case RISCV_LD_LB:
			REG[rd] = this->ldb(REG[rs1] + imm12_I_signed);
		break;
		case RISCV_LD_LH:
			REG[rd] = this->ldh(REG[rs1] + imm12_I_signed);
		break;
		case RISCV_LD_LW:
			REG[rd] = this->ldw(REG[rs1] + imm12_I_signed);
		break;
		case RISCV_LD_LD:
			REG[rd] = this->ldd(REG[rs1] + imm12_I_signed);
		break;
		case RISCV_LD_LBU:
			valueRegA = REG[rs1];
			REG[rd] = 0;
			REG[rd].set_slc(0, this->ldb(valueRegA + imm12_I_signed));
		break;
		case RISCV_LD_LHU:
			valueRegA = REG[rs1];
			REG[rd] = 0;
			REG[rd].set_slc(0, this->ldh(valueRegA + imm12_I_signed));
		break;
		case RISCV_LD_LWU:
			valueRegA = REG[rs1];
			REG[rd] = 0;
			REG[rd].set_slc(0, this->ldw(valueRegA + imm12_I_signed));
		break;
		default:
			printf("In LD switch case, this should never happen... Instr was %x\n", (int)ins);
			exit(-1);
		break;
		}
	break;

	//******************************************************************************************
	//Treatment for: STORE INSTRUCTIONS
	case RISCV_ST:
		switch(funct3)
		{
		case RISCV_ST_STB:
			this->stb(REG[rs1] + imm12_S_signed, REG[rs2]);
		break;
		case RISCV_ST_STH:
			this->sth(REG[rs1] + imm12_S_signed, REG[rs2]);
		break;
		case RISCV_ST_STW:
			this->stw(REG[rs1] + imm12_S_signed, REG[rs2]);
		break;
		case RISCV_ST_STD:
			this->std(REG[rs1] + imm12_S_signed, REG[rs2]);
		break;
		default:
			printf("In ST switch case, this should never happen... Instr was %x\n", (int)ins);
			exit(-1);
		break;
		}
	break;

	//******************************************************************************************
	//Treatment for: OPI INSTRUCTIONS
	case RISCV_OPI:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd;

		switch(funct3)
		{
		case RISCV_OPI_ADDI:
			REG[rd] = REG[rs1] + imm12_I_signed;
		break;
		case RISCV_OPI_SLTI:
			REG[rd] = (REG[rs1] < imm12_I_signed) ? 1 : 0;
		break;
		case RISCV_OPI_SLTIU:
			unsignedReg1.set_slc(0, REG[rs1].slc<32>(0));

			REG[rd] = (unsignedReg1 < imm12_I) ? 1 : 0;
		break;
		case RISCV_OPI_XORI:
			REG[rd] = REG[rs1] ^ imm12_I_signed;
		break;
		case RISCV_OPI_ORI:
			REG[rd] = REG[rs1] | imm12_I_signed;
		break;
		case RISCV_OPI_ANDI:
			REG[rd] = REG[rs1] & imm12_I_signed;
		break;
		case RISCV_OPI_SLLI:
			REG[rd] = REG[rs1] << shamt;
		break;
		case RISCV_OPI_SRI:
			if (funct7_smaller == RISCV_OPI_SRI_SRLI){
				REG[rd] = (REG[rs1] >> shamt) & shiftMask[shamt];
			}
			else //SRAI
				REG[rd] = REG[rs1] >> shamt;
		break;
		default:
			printf("In OPI switch case, this should never happen... Instr was %x\n", (int)ins);
			exit(-1);
		break;
		}
	break;

	//******************************************************************************************
	//Treatment for: OPIW INSTRUCTIONS
	case RISCV_OPIW:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd;

		localDataa = REG[rs1];
		localDatab = imm12_I_signed;
		switch(funct3)
		{
		case RISCV_OPIW_ADDIW:
			localResult = localDataa + localDatab;
			REG[rd] = localResult;
		break;
		case RISCV_OPIW_SLLIW:
			localResult = localDataa << rs2;
			REG[rd] = localResult;
		break;
		case RISCV_OPIW_SRW:
			if (funct7 == RISCV_OPIW_SRW_SRLIW)
				localResult = (localDataa >> rs2) & shiftMask[32+rs2];
			else //SRAI
				localResult = localDataa >> rs2;

			REG[rd] = localResult;
		break;
		default:
			printf("In OPI switch case, this should never happen... Instr was %x\n", (int)ins);
			exit(-1);
		break;
		}
	break;

	//******************************************************************************************
	//Treatment for: OP INSTRUCTIONS
	case RISCV_OP:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister || rs2 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd;

		if (funct7 == 1){
			//Switch case for multiplication operations (in standard extension RV32M)
			switch (funct3)
			{
			case RISCV_OP_M_MUL:
				longResult = REG[rs1] * REG[rs2];
				REG[rd] = longResult.slc<64>(0);
			break;
			case RISCV_OP_M_MULH:
				longResult = REG[rs1] * REG[rs2];
				REG[rd] = longResult.slc<64>(64);
			break;
			case RISCV_OP_M_MULHSU:
				unsignedReg2 = REG[rs2];
				longResult = REG[rs1] * unsignedReg2;
				REG[rd] = longResult.slc<64>(64);
			break;
			case RISCV_OP_M_MULHU:
				unsignedReg1 = REG[rs1];
				unsignedReg2 = REG[rs2];
				longResult = unsignedReg1 * unsignedReg2;
				REG[rd] = longResult.slc<64>(64);
			break;
			case RISCV_OP_M_DIV:
				REG[rd] = (REG[rs1] / REG[rs2]);
			break;
			case RISCV_OP_M_DIVU:
				unsignedReg1 = REG[rs1];
				unsignedReg2 = REG[rs2];
				REG[rd] = unsignedReg1 / unsignedReg2;
			break;
			case RISCV_OP_M_REM:
				REG[rd] = (REG[rs1] % REG[rs2]);
			break;
			case RISCV_OP_M_REMU:
				unsignedReg1 = REG[rs1];
				unsignedReg2 = REG[rs2];
				REG[rd] = unsignedReg1 % unsignedReg2;
			break;
			}

		}
		else{

			//Switch case for base OP operation
			switch(funct3)
			{
			case RISCV_OP_ADD:
				if (funct7 == RISCV_OP_ADD_ADD)
					REG[rd] = REG[rs1] + REG[rs2];
				else
					REG[rd] = REG[rs1] - REG[rs2];
			break;
			case RISCV_OP_SLL:
				REG[rd] = REG[rs1] << (REG[rs2] & 0x3f);
			break;
			case RISCV_OP_SLT:
				REG[rd] = (REG[rs1] < REG[rs2]) ? 1 : 0;
			break;
			case RISCV_OP_SLTU:
				unsignedReg1.set_slc(0, REG[rs1].slc<64>(0));
				unsignedReg2.set_slc(0, REG[rs2].slc<64>(0));

				REG[rd] = (unsignedReg1 < unsignedReg2) ? 1 : 0;
			break;
			case RISCV_OP_XOR:
				REG[rd] = REG[rs1] ^ REG[rs2];
			break;
			case RISCV_OP_SR:
				if (funct7 == RISCV_OP_SR_SRL){
					int shiftAmount = REG[rs2];
					REG[rd] = (REG[rs1] >> (REG[rs2] & 0x3f)) & shiftMask[(shiftAmount & 0x3f)];
				}
				else //SRA
					REG[rd] = REG[rs1] >> (REG[rs2] & 0x3f);
			break;
			case RISCV_OP_OR:
				REG[rd] = REG[rs1] | REG[rs2];
			break;
			case RISCV_OP_AND:
				REG[rd] = REG[rs1] & REG[rs2];
			break;
			default:
				printf("In OP switch case, this should never happen... Instr was %x\n", (int)ins);
				exit(-1);
			break;
			}
		}
		break;

	//******************************************************************************************
	//Treatment for: OPW INSTRUCTIONS
	case RISCV_OPW:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister || rs2 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd;

		if (funct7 == 1){
			localDataa = REG[rs1].slc<32>(0);
			localDatab = REG[rs2].slc<32>(0);
			localDataaUnsigned = REG[rs1].slc<32>(0);
			localDatabUnsigned = REG[rs2].slc<32>(0);

			//Switch case for multiplication operations (in standard extension RV32M)
			switch (funct3)
			{
			case RISCV_OPW_M_MULW:
				localLongResult = localDataa * localDatab;
				REG[rd] = localLongResult.slc<32>(0);
			break;
			case RISCV_OPW_M_DIVW:
				REG[rd] = (localDataa / localDatab);
			break;
			case RISCV_OPW_M_DIVUW:
				REG[rd] = localDataaUnsigned / localDatabUnsigned;
			break;
			case RISCV_OPW_M_REMW:
				REG[rd] = (localDataa % localDatab);
			break;
			case RISCV_OPW_M_REMUW:
				REG[rd] = localDataaUnsigned % localDatabUnsigned;
			break;
			}

		}
		else{
			localDataa = REG[rs1].slc<32>(0);
			localDatab = REG[rs2].slc<32>(0);

			//Switch case for base OP operation
			switch(funct3)
			{
			case RISCV_OPW_ADDSUBW:
				if (funct7 == RISCV_OPW_ADDSUBW_ADDW){
					localResult = localDataa + localDatab;
					REG[rd] = localResult;
				}
				else{ //SUBW
					localResult = localDataa - localDatab;
					REG[rd] = localResult;
				}
			break;
			case RISCV_OPW_SLLW:
				localResult = localDataa << (localDatab & 0x1f);
				REG[rd] = localResult;
			break;
			case RISCV_OPW_SRW:
				if (funct7 == RISCV_OPW_SRW_SRLW){
					localResult = (localDataa >> (localDatab/* & 0x1f*/)) & shiftMask[32+localDatab/* & 0x1f*/];
					REG[rd] = localResult;
				}
				else{ //SRAW
					localResult = localDataa >> (localDatab/* & 0x1f*/);
					REG[rd] = localResult;
				}
			break;
			default:
				printf("In OPW switch case, this should never happen... Instr was %x\n", (int)ins);
				exit(-1);
			break;
			}
		}
		break;
		//END of OP treatment

	//******************************************************************************************
	//Treatment for: SYSTEM INSTRUCTIONS
	case RISCV_SYSTEM:

		//For modelling pipeline performances
		if (17 == this->lastWrittenRegister || 10 == this->lastWrittenRegister || 11 == this->lastWrittenRegister || 12 == this->lastWrittenRegister || 13 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd;

		if (funct3 == 0 && funct7 == 0){
			REG[10] = solveSyscall(REG[17], REG[10], REG[11], REG[12], REG[13]);
		}
		else {
//			fprintf(stderr, "Unresolved system instr funct3 = %d funct7 = %d\n", funct3, funct7);
		}
	break;
	//******************************************************************************************
	//Treatment for: floating point operations
	case RISCV_FLW:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd+64;
		this->lastIsLoad = true;


		localInt = 0;
		localInt = ldw(REG[rs1] + imm12_I_signed);
		memcpy(&(regf[rd]), &localInt, 4);
		break;
	case RISCV_FSW:

		//For modelling pipeline performances
		if (rs1 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = 0;

		localInt = 0;
		memcpy(&localInt, &(regf[rs2]), 4);

		stw(REG[rs1] + imm12_S_signed, localInt);

		break;
	case RISCV_FMADD:
		//For modelling pipeline performances
		if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister || rs3 + 64 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd+64;

		regf[rd] = regf[rs1] * regf[rs2] + regf[rs3];
		break;
	case RISCV_FMSUB:

		//For modelling pipeline performances
		if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister || rs3 + 64 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd+64;

		regf[rd] = regf[rs1] * regf[rs2] - regf[rs3];
		break;
	case RISCV_FNMSUB:

		//For modelling pipeline performances
		if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister || rs3 + 64 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd+64;

		regf[rd] = -regf[rs1] * regf[rs2] + regf[rs3];
		break;
	case RISCV_FNMADD:

		//For modelling pipeline performances
		if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister || rs3 + 64 == this->lastWrittenRegister){
			if (this->lastIsLoad)
				this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
			else
				this->cycle += LOSS_PIPELINE_HAZARD;
		}
		this->lastWrittenRegister = rd+64;

		regf[rd] = -regf[rs1] * regf[rs2] - regf[rs3];
		break;
	case RISCV_FP:
		switch (funct7)
		{
			case  RISCV_FP_ADD:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				regf[rd] = regf[rs1] + regf[rs2];
				break;
			case  RISCV_FP_SUB:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				regf[rd] = regf[rs1] - regf[rs2];
				break;
			case  RISCV_FP_MUL:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				regf[rd] = regf[rs1] * regf[rs2];
				break;
			case  RISCV_FP_DIV:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				regf[rd] = regf[rs1] / regf[rs2];
				break;
			case  RISCV_FP_SQRT:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				regf[rd] = sqrt(regf[rs1]);
				break;
			case  RISCV_FP_FSGN:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				localFloat = fabs(regf[rs1]);
				if (funct3 == RISCV_FP_FSGN_J){
					if (regf[rs2]<0){
						regf[rd] = -localFloat;
					}
					else{
						regf[rd] = localFloat;
					}
				}
				else if (funct3 == RISCV_FP_FSGN_JN){
					if (regf[rs2]<0){
						regf[rd] = localFloat;
					}
					else{
						regf[rd] = -localFloat;
					}
				}
				else{ //JX
					if ((regf[rs2]<0 && regf[rs1]>=0) || (regf[rs2]>=0 && regf[rs1]<0)){
						regf[rd] = -localFloat;
					}
					else{
						regf[rd] = localFloat;
					}
				}

				break;
			case  RISCV_FP_MINMAX:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				if (funct3 == RISCV_FP_MINMAX_MIN)
					regf[rd] = MIN(regf[rs1], regf[rs2]);
				else
					regf[rd] = MAX(regf[rs1], regf[rs2]);
				break;
			case  RISCV_FP_FCVTW:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd;

				if (rs2 == RISCV_FP_FCVTW_W){
					REG[rd] = regf[rs1];
				}
				else{
					REG[rd] = (unsigned int) regf[rs1];
				}
				break;
			case  RISCV_FP_FMVXFCLASS:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd;

				if (funct3 == RISCV_FP_FMVXFCLASS_FMVX){
					memcpy(&localInt, &(regf[rs1]), 4);
					REG[rd] = localInt;
				}
				else{
					fprintf(stderr, "Fclass instruction is not handled in riscv simulator\n");
				}
				break;
			case  RISCV_FP_FCMP:

				//For modelling pipeline performances
				if (rs1 + 64 == this->lastWrittenRegister || rs2 + 64 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd;

				if (funct3 == RISCV_FP_FCMP_FEQ)
					REG[rd] = regf[rs1] == regf[rs2];
				else if (funct3 == RISCV_FP_FCMP_FLT)
					REG[rd] = regf[rs1] < regf[rs2];
				else
					REG[rd] = regf[rs1] <= regf[rs2];
				break;
			case  RISCV_FP_FCVTS:

				//For modelling pipeline performances
				if (rs1 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				if (rs2 == RISCV_FP_FCVTS_W){
					regf[rd] = REG[rs1];
				}
				else{
					regf[rd] = (unsigned int) REG[rs1];
				}
				break;
			case  RISCV_FP_FMVW:

				//For modelling pipeline performances
				if (rs1 == this->lastWrittenRegister){
					if (this->lastIsLoad)
						this->cycle += LOSS_PIPELINE_HAZARD_FORWARDED;
					else
						this->cycle += LOSS_PIPELINE_HAZARD;
				}
				this->lastWrittenRegister = rd+64;

				localInt = REG[rs1];
				memcpy(&(regf[rd]),&localInt,  4);
				break;

			default:
				printf("In FP part of switch opcode, instr %x is not handled yet(%x)  pC is %x\n", (int) ins, this->heapAddress, (int) this->cycle);
			break;
		}


		break;

	default:
//		printf("In default part of switch opcode, instr %x is not handled yet(%x)  pC is %x\n", (int) ins, this->heapAddress, this->cycle);
	break;

	}
	REG[0] = 0;
	n_inst = n_inst + 1;
	this->cycle++;


	if (storedVerbose>1){
		for (int i=0; i<32; i++){
			fprintf(stderr,";%lx", (int64_t) REG[i]);
		}
		fprintf(stderr, "--");

		for (int i=0; i<32; i++){
			fprintf(stderr,";%f", regf[i]);
		}
		fprintf(stderr, "\n");
	}
}

#endif
