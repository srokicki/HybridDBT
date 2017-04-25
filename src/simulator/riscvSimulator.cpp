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

ac_int<64, false> shiftMask[64];


ac_int<64, false> RiscvSimulator::doRead(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size){
	//printf("Doign read on file %x\n", file);

	int localSize = size.slc<32>(0);
	char* localBuffer = (char*) malloc(localSize*sizeof(char));
	ac_int<64, false> result;

	if (file == 0){
		result = fread(localBuffer, 1, size, stdin);
	}
	else{
		FILE* localFile = this->fileMap[file.slc<16>(0)];
		result = fread(localBuffer, 1, size, localFile);
	}

	for (int i=0; i<result; i++)
		this->stb(bufferAddr + i, localBuffer[i]);


	return result;
}


ac_int<64, false> RiscvSimulator::doWrite(ac_int<64, false> file, ac_int<64, false> bufferAddr, ac_int<64, false> size){
	int localSize = size.slc<32>(0);
	char* localBuffer = (char*) malloc(localSize*sizeof(char));
	for (int i=0; i<size; i++)
		localBuffer[i] = this->ldb(bufferAddr + i);

	if (file < 3){

		ac_int<64, false> result = fwrite(localBuffer, 1, size, stdout);
		return result;
	}
	else{

		FILE* localFile = this->fileMap[file.slc<16>(0)];


		ac_int<64, false> result = fwrite(localBuffer, 1, size, localFile);
		return result;
	}
}


ac_int<64, false> RiscvSimulator::doOpen(ac_int<64, false> path, ac_int<64, false> flags, ac_int<64, false> mode){
	int oneStringElement = this->ldb(path);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = this->ldb(path+index);
	}

	int pathSize = index+1;

	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = this->ldb(path + i);

	char* localMode;
	if (flags==0)
		localMode = "r";
	else if (flags == 577)
		localMode = "w";
	else if (flags == 1089)
		localMode = "a";
	else if (flags == O_WRONLY|O_CREAT|O_EXCL)
		localMode = "wx";
	else{
		fprintf(stderr, "Trying to open files with unknown flags... %d\n", flags);
		exit(-1);
	}

	FILE* test = fopen(localPath, localMode);
	uint64_t result = (uint64_t) test;
	ac_int<64, true> result_ac = result;

	//For some reasons, newlib only store last 16 bits of this pointer, we will then compute a hash and return that.
	//The real pointer is stored here in a hashmap

	ac_int<64, true> returnedResult = 0;
	returnedResult.set_slc(0, result_ac.slc<16>(0) ^ result_ac.slc<16>(16));

	this->fileMap[returnedResult.slc<16>(0)] = test;
	//printf("Doing open on %s, returned %x\n", localPath, returnedResult);

	return returnedResult;

}

ac_int<64, false> RiscvSimulator::doOpenat(ac_int<64, false> dir, ac_int<64, false> path, ac_int<64, false> flags, ac_int<64, false> mode){
	fprintf(stderr, "Syscall openat not implemented yet...\n");
	exit(-1);
}

ac_int<64, false> RiscvSimulator::doClose(ac_int<64, false> file){
	if (file > 2 ){
		FILE* localFile = this->fileMap[file.slc<16>(0)];
		int result = fclose(localFile);
		return result;
	}
	else
		return 0;
}

ac_int<64, false> RiscvSimulator::doLseek(ac_int<64, false> file, ac_int<64, false> ptr, ac_int<64, false> dir){
	FILE* localFile = this->fileMap[file.slc<16>(0)];
	int result = fseek(localFile, ptr, dir);
	return result;
}

ac_int<64, false> RiscvSimulator::doStat(ac_int<64, false> filename, ac_int<64, false> ptr){

	int oneStringElement = this->ldb(filename);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = this->ldb(filename+index);
	}

	int pathSize = index+1;

	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = this->ldb(filename + i);

	struct stat fileStat;
	int result = stat(localPath, &fileStat);

	//We copy the result in simulator memory
	for (int oneChar = 0; oneChar<sizeof(struct stat); oneChar++)
		this->stb(ptr+oneChar, ((char*)(&stat))[oneChar]);

	return result;
}


void RiscvSimulator::initialize(int argc, char** argv){

	//We initialize registers
	for (int oneReg = 0; oneReg < 32; oneReg++)
		reg[oneReg] = 0;
	reg[2] = 0x70000;

	/******************************************************
	 * Argument passing:
	 * In this part of the initialization code, we will copy argc and argv into the simulator stack memory.
	 *
	 ******************************************************/

	ac_int<64, true> currentPlaceStrings = reg[2] + 8 + 8*argc;

	this->std(reg[2], argc);
	for (int oneArg = 0; oneArg<argc; oneArg++){
		this->std(reg[2] + 8*oneArg + 8, currentPlaceStrings);


		int oneCharIndex = 0;
		char oneChar = argv[oneArg][oneCharIndex];
		while (oneChar != 0){
			this->stb(currentPlaceStrings + oneCharIndex, oneChar);
			oneCharIndex++;
			oneChar = argv[oneArg][oneCharIndex];
		}
		this->stb(currentPlaceStrings + oneCharIndex, oneChar);
		oneCharIndex++;
		currentPlaceStrings += oneCharIndex;

	}

}

int RiscvSimulator::doSimulation(int start){
	long long hilo;


	ac_int<64, true> pc = start;


	ac_int<32, 0> ins, next_instr;

	//We initialize shiftmask
	ac_int<64, false> value = 0xffffffff;
	value = (value << 32) + value;
	for (int i=0; i<64; i++){
		shiftMask[i] = value;
		value = value >> 1;
	}

	while (1){
		int i;
		int n_inst;
		unsigned int ivalue;
		float fvalue;



		//We initialize instruction counter
		n_inst = 0;






		do{

			/*Fetching new instruction */
			ins = this->ldw(pc);

			if (this->debugLevel>1){
				fprintf(stderr,"%d;%x;%x", (int)n_inst, (int)pc, (int) ins);
				std::cerr << printDecodedInstrRISCV(ins);
			}

			pc = pc + 4;


			//We decode the instruction to execute
			ac_int<7, false> opcode = ins.slc<7>(0);
			ac_int<5, false> rs1 = ins.slc<5>(15);
			ac_int<5, false> rs2 = ins.slc<5>(20);
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

			//According to opcode/funct3/funct7 we perform the correct operation
			switch (opcode)
			{
			case RISCV_LUI:
				reg[rd] = imm31_12;
			break;
			case RISCV_AUIPC:
				reg[rd] = pc -4 + imm31_12;
			break;
			case RISCV_JAL:
				reg[rd] = pc;
				pc = pc - 4 + imm21_1_signed;
			break;
			case RISCV_JALR:
				reg[rd] = pc;
				pc = (reg[rs1] + imm12_I_signed) & 0xfffffffe;
			break;

			//******************************************************************************************
			//Treatment for: BRANCH INSTRUCTIONS
			case RISCV_BR:
				switch(funct3)
				{
				case RISCV_BR_BEQ:
					if (reg[rs1] == reg[rs2])
						pc = pc + (imm13_signed) - 4;
				break;
				case RISCV_BR_BNE:
					if (reg[rs1] != reg[rs2])
						pc = pc + (imm13_signed) - 4;
				break;
				case RISCV_BR_BLT:
					if (reg[rs1] < reg[rs2])
						pc = pc + (imm13_signed) - 4;
				break;
				case RISCV_BR_BGE:
					if (reg[rs1] >= reg[rs2])
						pc = pc + (imm13_signed) - 4;
				break;
				case RISCV_BR_BLTU:
					unsignedReg1.set_slc(0, reg[rs1].slc<64>(0));
					unsignedReg2.set_slc(0, reg[rs2].slc<64>(0));

					if (unsignedReg1 < unsignedReg2)
						pc = pc + (imm13_signed) - 4;
				break;
				case RISCV_BR_BGEU:
					unsignedReg1.set_slc(0, reg[rs1].slc<64>(0));
					unsignedReg2.set_slc(0, reg[rs2].slc<64>(0));

					if (unsignedReg1 >= unsignedReg2)
						pc = pc + (imm13_signed) - 4;
				break;
				default:
					printf("In BR switch case, this should never happen... Instr was %x\n", (int)ins);
				break;
				}
			break;

			//******************************************************************************************
			//Treatment for: LOAD INSTRUCTIONS
			case RISCV_LD:
				switch(funct3)
				{
				case RISCV_LD_LB:
					reg[rd] = this->ldb(reg[rs1] + imm12_I_signed);
				break;
				case RISCV_LD_LH:
					reg[rd] = this->ldh(reg[rs1] + imm12_I_signed);
				break;
				case RISCV_LD_LW:
					reg[rd] = this->ldw(reg[rs1] + imm12_I_signed);
				break;
				case RISCV_LD_LD:
					reg[rd] = this->ldd(reg[rs1] + imm12_I_signed);
				break;
				case RISCV_LD_LBU:
					valueRegA = reg[rs1];
					reg[rd] = 0;
					reg[rd].set_slc(0, this->ldb(valueRegA + imm12_I_signed));
				break;
				case RISCV_LD_LHU:
					valueRegA = reg[rs1];
					reg[rd] = 0;
					reg[rd].set_slc(0, this->ldh(valueRegA + imm12_I_signed));
				break;
				case RISCV_LD_LWU:
					valueRegA = reg[rs1];
					reg[rd] = 0;
					reg[rd].set_slc(0, this->ldw(valueRegA + imm12_I_signed));
				break;
				default:
					printf("In LD switch case, this should never happen... Instr was %x\n", (int)ins);
				break;
				}
			break;

			//******************************************************************************************
			//Treatment for: STORE INSTRUCTIONS
			case RISCV_ST:
				switch(funct3)
				{
				case RISCV_ST_STB:
					this->stb(reg[rs1] + imm12_S_signed, reg[rs2]);
				break;
				case RISCV_ST_STH:
					this->sth(reg[rs1] + imm12_S_signed, reg[rs2]);
				break;
				case RISCV_ST_STW:
					this->stw(reg[rs1] + imm12_S_signed, reg[rs2]);
				break;
				case RISCV_ST_STD:
					this->std(reg[rs1] + imm12_S_signed, reg[rs2]);
				break;
				default:
					printf("In ST switch case, this should never happen... Instr was %x\n", (int)ins);
				break;
				}
			break;

			//******************************************************************************************
			//Treatment for: OPI INSTRUCTIONS
			case RISCV_OPI:
				switch(funct3)
				{
				case RISCV_OPI_ADDI:
					reg[rd] = reg[rs1] + imm12_I_signed;
				break;
				case RISCV_OPI_SLTI:
					reg[rd] = (reg[rs1] < imm12_I_signed) ? 1 : 0;
				break;
				case RISCV_OPI_SLTIU:
					unsignedReg1.set_slc(0, reg[rs1].slc<32>(0));

					reg[rd] = (unsignedReg1 < imm12_I) ? 1 : 0;
				break;
				case RISCV_OPI_XORI:
					reg[rd] = reg[rs1] ^ imm12_I_signed;
				break;
				case RISCV_OPI_ORI:
					reg[rd] = reg[rs1] | imm12_I_signed;
				break;
				case RISCV_OPI_ANDI:
					reg[rd] = reg[rs1] & imm12_I_signed;
				break;
				case RISCV_OPI_SLLI:
					reg[rd] = reg[rs1] << shamt;
				break;
				case RISCV_OPI_SRI:
					if (funct7_smaller == RISCV_OPI_SRI_SRLI){
						reg[rd] = (reg[rs1] >> shamt) & shiftMask[shamt];
					}
					else //SRAI
						reg[rd] = reg[rs1] >> shamt;
				break;
				default:
					printf("In OPI switch case, this should never happen... Instr was %x\n", (int)ins);
				break;
				}
			break;

			//******************************************************************************************
			//Treatment for: OPIW INSTRUCTIONS
			case RISCV_OPIW:
				localDataa = reg[rs1];
				localDatab = imm12_I_signed;
				switch(funct3)
				{
				case RISCV_OPIW_ADDIW:
					localResult = localDataa + localDatab;
					reg[rd] = localResult;
				break;
				case RISCV_OPIW_SLLIW:
					localResult = localDataa << rs2;
					reg[rd] = localResult;
				break;
				case RISCV_OPIW_SRW:
					if (funct7 == RISCV_OPIW_SRW_SRLIW)
						localResult = (localDataa >> rs2) & shiftMask[32+rs2];
					else //SRAI
						localResult = localDataa >> rs2;

					reg[rd] = localResult;
				break;
				default:
					printf("In OPI switch case, this should never happen... Instr was %x\n", (int)ins);
				break;
				}
			break;

			//******************************************************************************************
			//Treatment for: OP INSTRUCTIONS
			case RISCV_OP:
				if (funct7 == 1){
					//Switch case for multiplication operations (in standard extension RV32M)
					switch (funct3)
					{
					case RISCV_OP_M_MUL:
						longResult = reg[rs1] * reg[rs2];
						reg[rd] = longResult.slc<64>(0);
					break;
					case RISCV_OP_M_MULH:
						longResult = reg[rs1] * reg[rs2];
						reg[rd] = longResult.slc<64>(64);
					break;
					case RISCV_OP_M_MULHSU:
						unsignedReg2 = reg[rs2];
						longResult = reg[rs1] * unsignedReg2;
						reg[rd] = longResult.slc<64>(64);
					break;
					case RISCV_OP_M_MULHU:
						unsignedReg1 = reg[rs1];
						unsignedReg2 = reg[rs2];
						longResult = unsignedReg1 * unsignedReg2;
						reg[rd] = longResult.slc<64>(64);
					break;
					case RISCV_OP_M_DIV:
						reg[rd] = (reg[rs1] / reg[rs2]);
					break;
					case RISCV_OP_M_DIVU:
						unsignedReg1 = reg[rs1];
						unsignedReg2 = reg[rs2];
						reg[rd] = unsignedReg1 / unsignedReg2;
					break;
					case RISCV_OP_M_REM:
						reg[rd] = (reg[rs1] % reg[rs2]);
					break;
					case RISCV_OP_M_REMU:
						unsignedReg1 = reg[rs1];
						unsignedReg2 = reg[rs2];
						reg[rd] = unsignedReg1 % unsignedReg2;
					break;
					}

				}
				else{

					//Switch case for base OP operation
					switch(funct3)
					{
					case RISCV_OP_ADD:
						if (funct7 == RISCV_OP_ADD_ADD)
							reg[rd] = reg[rs1] + reg[rs2];
						else
							reg[rd] = reg[rs1] - reg[rs2];
					break;
					case RISCV_OP_SLL:
						reg[rd] = reg[rs1] << (reg[rs2] & 0x3f);
					break;
					case RISCV_OP_SLT:
						reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0;
					break;
					case RISCV_OP_SLTU:
						unsignedReg1.set_slc(0, reg[rs1].slc<32>(0));
						unsignedReg2.set_slc(0, reg[rs2].slc<32>(0));

						reg[rd] = (unsignedReg1 < unsignedReg2) ? 1 : 0;
					break;
					case RISCV_OP_XOR:
						reg[rd] = reg[rs1] ^ reg[rs2];
					break;
					case RISCV_OP_SR:
						if (funct7 == RISCV_OP_SR_SRL)
							reg[rd] = (reg[rs1] >> (reg[rs2] & 0x3f)) & shiftMask[reg[rs2] & 0x3f];
						else //SRA
							reg[rd] = reg[rs1] >> (reg[rs2] & 0x3f);
					break;
					case RISCV_OP_OR:
						reg[rd] = reg[rs1] | reg[rs2];
					break;
					case RISCV_OP_AND:
						reg[rd] = reg[rs1] & reg[rs2];
					break;
					default:
						printf("In OP switch case, this should never happen... Instr was %x\n", (int)ins);
					break;
					}
				}
				break;

			//******************************************************************************************
			//Treatment for: OPW INSTRUCTIONS
			case RISCV_OPW:
				if (funct7 == 1){
					localDataa = reg[rs1].slc<32>(0);
					localDatab = reg[rs2].slc<32>(0);
					localDataaUnsigned = reg[rs1].slc<32>(0);
					localDatabUnsigned = reg[rs2].slc<32>(0);

					//Switch case for multiplication operations (in standard extension RV32M)
					switch (funct3)
					{
					case RISCV_OPW_M_MULW:
						localLongResult = localDataa * localDatab;
						reg[rd] = localLongResult.slc<32>(0);
					break;
					case RISCV_OPW_M_DIVW:
						reg[rd] = (localDataa / localDatab);
					break;
					case RISCV_OPW_M_DIVUW:
						reg[rd] = localDataaUnsigned / localDatabUnsigned;
					break;
					case RISCV_OPW_M_REMW:
						reg[rd] = (localDataa % localDatab);
					break;
					case RISCV_OPW_M_REMUW:
						reg[rd] = localDataaUnsigned % localDatabUnsigned;
					break;
					}

				}
				else{
					localDataa = reg[rs1].slc<32>(0);
					localDatab = reg[rs2].slc<32>(0);

					//Switch case for base OP operation
					switch(funct3)
					{
					case RISCV_OPW_ADDSUBW:
						if (funct7 == RISCV_OPW_ADDSUBW_ADDW){
							localResult = localDataa + localDatab;
							reg[rd] = localResult;
						}
						else{ //SUBW
							localResult = localDataa - localDatab;
							reg[rd] = localResult;
						}
					break;
					case RISCV_OPW_SLLW:
						localResult = localDataa << (localDatab & 0x1f);
						reg[rd] = localResult;
					break;
					case RISCV_OPW_SRW:
						if (funct7 == RISCV_OPW_SRW_SRLW){
							localResult = (localDataa >> (localDatab/* & 0x1f*/)) & shiftMask[32+localDatab/* & 0x1f*/];
							reg[rd] = localResult;
						}
						else{ //SRAW
							localResult = localDataa >> (localDatab/* & 0x1f*/);
							reg[rd] = localResult;
						}
					break;
					default:
						printf("In OPW switch case, this should never happen... Instr was %x\n", (int)ins);
					break;
					}
				}
				break;
				//END of OP treatment

			//******************************************************************************************
			//Treatment for: SYSTEM INSTRUCTIONS
			case RISCV_SYSTEM:
				if (funct3 == 0 && funct7 == 0){
					//We are on ecall
					switch (reg[17]){
					case SYS_exit:
						pc = 0; //Currently we break on ECALL
					break;
					case SYS_read:
//						fprintf(stderr, "Reading %d char\n", reg[12]);
						reg[10] = this->doRead(reg[10], reg[11], reg[12]);
					break;
					case SYS_write:
						reg[10] = doWrite(reg[10], reg[11], reg[12]);
					break;
					case SYS_brk:
						//We do nothing
					break;
					case SYS_open:
						reg[10] = this->doOpen(reg[10], reg[11], reg[12]);
					break;
					case SYS_openat:
						reg[10] = this->doOpenat(reg[13], reg[11], reg[12], reg[14]);
					break;
					case SYS_lseek:

						reg[10] = this->doLseek(reg[10], reg[11], reg[12]);
					break;
					case SYS_close:
						reg[10] = this->doClose(reg[10]);
					break;
					case SYS_fstat:
						reg[10] = 0;
					break;
					case SYS_stat:
						reg[10] = this->doStat(reg[10], reg[11]);
					break;
					default:
						printf("Unknown syscall with code %d\n", reg[17].slc<32>(0));
						exit(-1);
						reg[10] = 0;
					break;
					}


				}
			break;
			default:
				printf("In default part of switch opcode, instr %x is not handled yet\n", (int) ins);
			break;

			}
			reg[0] = 0;
			n_inst = n_inst + 1;

			if (this->debugLevel>1){
				for (int i=0; i<32; i++){
					fprintf(stderr,";%lx", (int64_t) reg[i]);
				}
				fprintf(stderr, "\n");
			}
		}
		while (pc != 0 && n_inst<1000000000); //Apparently, return from main function is always at address 0x4000074
		fprintf(stderr,"Simulation finished in %d cycles\n",n_inst);

		return 0;
	}
}

void RiscvSimulator::stb(ac_int<64, false> addr, ac_int<8, true> value){
	if (addr == 0x10009000){
		printf("%c",(char) value&0xff);

	}
	else
		this->memory[addr] = value & 0xff;

//	fprintf(stderr, "memwrite %x %x\n", addr, value);

}



void RiscvSimulator::sth(ac_int<64, false> addr, ac_int<16, true> value){
	this->stb(addr+1, value.slc<8>(8));
	this->stb(addr+0, value.slc<8>(0));
}

void RiscvSimulator::stw(ac_int<64, false> addr, ac_int<32, true> value){
	this->stb(addr+3, value.slc<8>(24));
	this->stb(addr+2, value.slc<8>(16));
	this->stb(addr+1, value.slc<8>(8));
	this->stb(addr+0, value.slc<8>(0));
}

void RiscvSimulator::std(ac_int<64, false> addr, ac_int<64, true> value){
	this->stb(addr+7, value.slc<8>(56));
	this->stb(addr+6, value.slc<8>(48));
	this->stb(addr+5, value.slc<8>(40));
	this->stb(addr+4, value.slc<8>(32));
	this->stb(addr+3, value.slc<8>(24));
	this->stb(addr+2, value.slc<8>(16));
	this->stb(addr+1, value.slc<8>(8));
	this->stb(addr+0, value.slc<8>(0));
}


ac_int<8, true> RiscvSimulator::ldb(ac_int<64, false> addr){

	ac_int<8, true> result = 0;
	if (addr == 0x10009000){
		result = getchar();
	}
	else if (this->memory.find(addr) != this->memory.end())
		result = this->memory[addr];
	else
		result= 0;

//	fprintf(stderr, "memread %x %x\n", addr, result);

	return result;
}


//Little endian version
ac_int<16, true> RiscvSimulator::ldh(ac_int<64, false> addr){

	ac_int<16, true> result = 0;
	result.set_slc(8, this->ldb(addr+1));
	result.set_slc(0, this->ldb(addr));
	return result;
}

ac_int<32, true> RiscvSimulator::ldw(ac_int<64, false> addr){

	ac_int<32, true> result = 0;
	result.set_slc(24, this->ldb(addr+3));
	result.set_slc(16, this->ldb(addr+2));
	result.set_slc(8, this->ldb(addr+1));
	result.set_slc(0, this->ldb(addr));
	return result;
}

ac_int<64, false> RiscvSimulator::ldd(ac_int<64, false> addr){

	ac_int<64, false> result = 0;
	result.set_slc(56, this->ldb(addr+7));
	result.set_slc(48, this->ldb(addr+6));
	result.set_slc(40, this->ldb(addr+5));
	result.set_slc(32, this->ldb(addr+4));
	result.set_slc(24, this->ldb(addr+3));
	result.set_slc(16, this->ldb(addr+2));
	result.set_slc(8, this->ldb(addr+1));
	result.set_slc(0, this->ldb(addr));
	return result;
}

void RiscvSimulator::doStep(){

}

#endif
