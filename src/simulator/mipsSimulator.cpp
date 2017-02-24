#ifndef __NIOS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <simulator/mipsSimulator.h>

#define TRACE_CFG 1

/*
 * We define values for opcodes
 */

//opcodes
#define R 0x0
#define REGIMM 0x1
#define J 0x2
#define JAL 0x3
#define BEQ 0x4
#define BNE 0x5
#define BLEZ 0x6
#define BGTZ 0x7
#define ADDI 0x8 //TODO
#define ADDIU 0x9
#define SLTI 0xa
#define SLTIU 0xb
#define ANDI 0xc
#define ORI 0xd
#define XORI 0xe
#define LUI 0xf

#define COP1 0x11
#define COP3 0x13
#define SPECIAL2 0x1c
#define SEH 0x1f


#define LB 0x20
#define LH 0x21
#define LWL 0x22
#define LW 0x23
#define LBU 0x24
#define LHU 0x25
#define LWR 0x26
#define LWU 0x27
#define SB 0x28
#define SH 0x29
#define SWL 0x2a
#define SW 0x2b
#define SDL 0x2c
#define SDR 0x2d
#define SWR 0x2e

#define LWC1 0x31
#define LDC1 0x35

#define SWC1 0x39
#define SDC1 0x3d


//Funct
#define SLL 0x0

#define SRL 0x2
#define SRA 0x3
#define SLLV 0x4

#define SRLV 0x6
#define SRAV 0x7
#define JR 0x8
#define JALR 0x9
#define MOVZ 0xa
#define MOVN 0xb
#define FUNCT_BREAK 0xd




#define MFHI 0x10
#define MTHI 0x11
#define MFLO 0x12
#define MTLO 0x13




#define MULT 0x18
#define MULTU 0x19
#define DIV 0x1a
#define DIVU 0x1b




#define ADD 0x20
#define ADDU 0x21
#define SUB 0x22
#define SUBU 0x23
#define AND 0x24
#define OR 0x25
#define XOR 0x26
#define NOR 0x27

#define TEQ 0x34


#define SLT 0x2a
#define SLTU 0x2b


//REGIMM
#define BLTZ 0
#define BGEZ 1
#define REGIMM_BAL 0x11

//SPECIAL2
#define SPECIAL2_MUL 2

//********************
//For copro1 instr

//Formats
#define COP1_SINGLE 0x10
#define COP1_DOUBLE 0x11
#define COP1_CMP_S 0x17
#define COP1_CMP_D 0x18
#define COP1_BCEQZ 0x09
#define COP1_BCNEZ 0x0D
#define COP1_MTC1 0x4
#define COP1_MTHC1 0x7
#define COP1_CVT_D_W 0x14
#define COP1_MOV_ID 0x0

//Funct
#define COP1_MADDF 0x0
#define COP1_MULF 0x2
#define COP1_MSUBF 0x1
#define COP1_TRUNCWD 0xD

#define COP1_SEL 0x10
#define COP1_MAX 0x1D
#define COP1_MIN 0x1C
#define COP1_MAXA 0x1F
#define COP1_MINA 0x1E
#define COP1_SELEQZ 0x14
#define COP1_SELNEZ 0x17
#define COP1_RINT 0x1A
#define COP1_CLASS 0x1B
#define COP1_CVT_S_D 0x20
#define COP1_CVT_D_S 0x21

#define COP1_MOV 0x06


//cond
#define COP1_CMP_F 0x0
#define COP1_CMP_UN 0x1
#define COP1_CMP_EQ 0x2
#define COP1_CMP_UEQ 0x3
#define COP1_CMP_LT 0x4
#define COP1_CMP_ULT 0x5
#define COP1_CMP_LE 0x6
#define COP1_CMP_ULE 0x7
#define COP1_CMP_SAF 0x8
#define COP1_CMP_SUN 0x9
#define COP1_CMP_SEQ 0xA
#define COP1_CMP_SUEQ 0xB
#define COP1_CMP_SLT 0xC
#define COP1_CMP_SULT 0xD
#define COP1_CMP_SLE 0xE
#define COP1_CMP_SULE 0xF


#define IADDR(x)	(((x)&0x0000ffff)>>2)



const unsigned int shiftMask[32] = { 0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
							0xfffffff, 	0x7ffffff, 	0x3ffffff, 	0x1ffffff,
							0xffffff, 	0x7fffff, 	0x3fffff, 	0x1fffff,
							0xfffff, 	0x7ffff, 	0x3ffff, 	0x1ffff,
							0xffff, 	0x7fff, 	0x3fff, 	0x1fff,
							0xfff, 		0x7ff, 		0x3ff, 		0x1ff,
							0xff, 		0x7f, 		0x3f, 		0x1f,
							0xf, 		0x7, 		0x3, 		0x1};



//Mips simulator
int Simulator::doSimulation(int start){
	long long hilo;
	int reg[32];
	float copro1_reg[32];


	int Hi = 0;
	int Lo = 0;
	unsigned int pc = start;


	unsigned int ins, next_instr;
	int op;
	int rs;
	int rt;
	int regimm;
	int rd;
	int shamt;
	int funct;
	short address;
	int tgtadr;

	int cop1_format, ft, fs, fd, cop1_cond;

	while (1){
		int i;
		int n_inst;
		unsigned int ivalue;
		float fvalue;



		n_inst = 0;

		//Register initialization
		for (i = 0; i < 32; i++){
			reg[i] = 0;
			copro1_reg[i] = 0.0;
		}
		reg[29] = 0x7fffeffc;


		next_instr = this->ldw(pc);

		do{
			ins = next_instr;
			next_instr = this->ldw(pc);
			fprintf(stderr,"%d;%x", n_inst, pc);


			pc = pc + 4;

			//Applying xor to every fields of the function
			op = (ins >> 26);

			funct = (ins & 0x3f);
			shamt = ((ins >> 6) & 0x1f);
			rd = ((ins >> 11) & 0x1f);
			rt = ((ins >> 16) & 0x1f);
			rs = ((ins >> 21) & 0x1f);
			regimm = rt;
			address = (ins & 0xffff);

			tgtadr = (ins & 0x3ffffff);

			//For cop1 instructions
			cop1_cond = (ins & 0x1f);
			fd = ((ins >> 6) & 0x1f);
			fs = ((ins >> 11) & 0x1f);
			ft = ((ins >> 16) & 0x1f);
			cop1_format = ((ins >> 21) & 0x1f);


			switch (op)
			{
			case R:

				switch (funct)
				{

				case ADDU:
					reg[rd] = reg[rs] +  reg[rt];
					break;

				case SUBU:
					reg[rd] = reg[rs] - reg[rt];
					break;

				case MULT:
					hilo = (long long) reg[rs] * (long long) reg[rt];
					Lo = hilo & 0x00000000ffffffffULL;
					Hi = ((int) (hilo >> 32)) & 0xffffffffUL;
					break;

				case MULTU:
					hilo = (unsigned long long) reg[rs] * (unsigned long long) reg[rt];
					Lo = hilo & 0x00000000ffffffffULL;
					Hi = ((int) (hilo >> 32)) & 0xffffffffUL;
					break;

				case DIVU:
					Lo = (unsigned int) reg[rs] / (unsigned int) reg[rt];
					Hi = (unsigned int) reg[rs] % (unsigned int) reg[rt];
					break;

				case DIV:
					Lo = reg[rs] / reg[rt];
					Hi = reg[rs] % reg[rt];
					break;

				case MFHI:
					reg[rd] = Hi;
					break;

				case MFLO:
					reg[rd] = Lo;
					break;

				case AND:
					reg[rd] = reg[rs] & reg[rt];
					break;

				case OR:
					reg[rd] = reg[rs] | reg[rt];
					break;

				case NOR:
					reg[rd] = ~(reg[rs] | reg[rt]);
					break;


				case XOR:
					reg[rd] = reg[rs] ^ reg[rt];
					break;

				case SLL:
					reg[rd] = reg[rt] << shamt;
					break;

				case SRL:
					reg[rd] = (reg[rt] >> shamt) & shiftMask[shamt];
					break;

				case SRA:
						reg[rd] = reg[rt] >> shamt;
					break;

				case SRAV:
						reg[rd] = reg[rt] >> reg[rs];
					break;

				case SLLV:
					reg[rd] = reg[rt] << reg[rs];
					break;

				case SRLV:
					reg[rd] = (reg[rt] >> reg[rs]) & shiftMask[reg[rs]];
					break;

				case SLT:
					reg[rd] = reg[rs] < reg[rt];
					break;

				case SLTU:
					reg[rd] = (unsigned int) reg[rs] < (unsigned int) reg[rt];
					break;

				case JR:
					pc = reg[rs];
					break;

				case JALR:
					reg[31] = pc;
					pc = reg[rs];
					break;

				case MOVN:
					if (reg[rt] != 0)
						reg[rd] = reg[rs];
					break;

				case MOVZ:
					if (reg[rt] == 0)
						reg[rd] = reg[rs];
					break;

				case TEQ: //Trap if equal
					if (reg[rt] == reg[rs])
						pc=0;
					break;

				case FUNCT_BREAK: //Trap if equal
					pc=0;
					break;

				default:
					fprintf(stderr, "Encountering an unknown opcode %x and funct %x at address %x = %x\n", op, funct, (pc-0xa0020040)/4, ins);
					pc = 0;	// error
					break;
				}
				break;

				case J:
					pc = (pc & 0xfc000000) | (tgtadr << 2);
					break;

				case JAL:
					reg[31] = pc;
					pc = (pc & 0xfc000000) | (tgtadr << 2);
					break;

				case REGIMM:
					switch (regimm){
					case BLTZ:
						if (reg[rs] < 0){
							pc = pc + (address << 2) - 4;
						}

						break;
					case BGEZ:
						if (reg[rs] >= 0){
							pc = pc + (address << 2) - 4;
						}

						break;
					case REGIMM_BAL:
						reg[31] = pc;
						pc = pc + (address << 2) - 4;
						break;
					default:
						fprintf(stderr, "Encountering an unknown opcode on REGIMM %x at address %x = %x\n",regimm, pc, ins);
						pc = 0;
						break;
					}
					break;

				case SPECIAL2:
					switch (funct){
					case SPECIAL2_MUL:
						reg[rd] = reg[rs] + reg[rt];
						break;
					default:
						fprintf(stderr, "Encountering an unknown opcode on SPECIAL2 %x at address %x = %x\n",funct, pc, ins);
						pc = 0;
						break;
					}
					break;


				default:
					switch (op)
					{
					case ADDIU:
						reg[rt] = reg[rs] + address;
						break;

					case ANDI:
						reg[rt] = reg[rs] & (unsigned short) address;
						break;

					case ORI:
						reg[rt] = reg[rs] | (unsigned short) address;
						break;

					case XORI:
						reg[rt] = reg[rs] ^ (unsigned short) address;
						break;

					case LW:
						reg[rt] = this->ldw(reg[rs] + address);
						break;

					case LWL:
						ivalue = this->ldh((reg[rs] + address));
						if (ivalue >= 32768) //handle the signed case
							ivalue = ivalue - 2*32768;

						reg[rt] = (reg[rt] & 0xffff) | (ivalue)<<16;
						break;

					case LWR:
						ivalue = this->ldh((reg[rs] + address));
						if (ivalue >= 32768) //handle the signed case
							ivalue = ivalue - 2*32768;

						reg[rt] = (reg[rt] & 0xffff0000) | ivalue;
						break;

					case LHU:
						reg[rt] = this->ldh((reg[rs] + address));
						break;

					case LH:
						ivalue =this->ldh((reg[rs] + address));
						if (ivalue >= 32768) //handle the signed case
							ivalue = ivalue - 2*32768;

						reg[rt] = ivalue;
						break;

					case LBU:
						reg[rt] = this->ldb((reg[rs] + address));
						break;

					case LB:
						ivalue = this->ldb((reg[rs] + address));
						if (ivalue > 127) //handle the signed case
							ivalue = ivalue - 256;

						reg[rt] =  ivalue;

						break;


					case SB:
						this->stb((reg[rs] + address), reg[rt]);
						break;

					case SH:
						this->sth((reg[rs] + address), reg[rt]);
						break;

					case SW:
						this->stw((reg[rs] + address), reg[rt]);
						break;

					case SWL://we store left part as a halfword
						this->sth((reg[rs] + address), (reg[rt] >> 16)&0xffff);
						break;

					case SWR: //we store right part as a halfword
						this->sth((reg[rs] + address), reg[rt]&0xffff);
						break;

					case LUI:
						reg[rt] = address << 16;
						break;

					case BEQ:
						if (reg[rs] == reg[rt]){
							pc = pc + (address << 2) - 4;
						}

						break;

					case BNE:
						ivalue = address;
						if (reg[rs] != reg[rt]){
							pc = pc + (ivalue << 2) - 4;
						}
						break;

					case BGTZ:
						ivalue = address;
						if (reg[rs] > 0){
							pc = pc + (ivalue << 2) - 4;
						}

						break;

					case BLEZ:
						ivalue = address;
						if (reg[rs] <= 0){
							pc = pc + (ivalue << 2) - 4;
						}
						break;

					case SLTI:
						reg[rt] = reg[rs] < address;
						break;

					case SLTIU:
						reg[rt] = (unsigned int) reg[rs] < (unsigned short) address;
						break;

					//For coprocessor
					case LWC1:
						ivalue = this->ldw((reg[rs] + address));
						memcpy(&fvalue, &ivalue, sizeof(float));
						copro1_reg[rt] = fvalue;
						break;

					case LDC1:
						ivalue = this->ldw((reg[rs] + address));
						memcpy(&fvalue, &ivalue, sizeof(float));
						copro1_reg[rt] = fvalue;
						ivalue = this->ldw((reg[rs] + address+4));
						memcpy(&fvalue, &ivalue, sizeof(float));
						copro1_reg[rt+1] = fvalue;
						break;

					//For coprocessor
					case SWC1:
						fvalue = copro1_reg[rt];
						memcpy(&ivalue, &fvalue, sizeof(float));
						this->stw(reg[rs] + address, ivalue);
						break;

					//For coprocessor
					case SDC1:
						fvalue = copro1_reg[rt];
						memcpy(&ivalue, &fvalue, sizeof(float));
						this->stw(reg[rs] + address, ivalue);
						fvalue = copro1_reg[rt+1];
						memcpy(&ivalue, &fvalue, sizeof(float));
						this->stw(reg[rs] + address+4, ivalue);
						break;
					break;

					case COP1:
						switch(cop1_format){
						case COP1_SINGLE:
						case COP1_DOUBLE:
							switch (funct){
							case COP1_MADDF:
								copro1_reg[fd] = copro1_reg[ft] + copro1_reg[fs];
							break;
							case COP1_MSUBF:
								copro1_reg[fd] = copro1_reg[ft] - copro1_reg[fs];
							break;
							case COP1_MULF:
								copro1_reg[fd] = copro1_reg[ft] * copro1_reg[fs];
							break;
							case COP1_TRUNCWD:
								copro1_reg[fd] = trunc(copro1_reg[fs]);
							break;
							case COP1_SEL:
								fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
								pc=0;
							break;
							case COP1_MAX:
								if (copro1_reg[ft] > copro1_reg[fs])
									copro1_reg[fd] = copro1_reg[ft];
								else
									copro1_reg[fd] = copro1_reg[fs];

							break;
							case COP1_MIN:
								if (copro1_reg[ft] < copro1_reg[fs])
									copro1_reg[fd] = copro1_reg[ft];
								else
									copro1_reg[fd] = copro1_reg[fs];
							break;
							case COP1_MAXA:
								if (copro1_reg[ft] > copro1_reg[fs])
									copro1_reg[fd] = copro1_reg[ft];
								else
									copro1_reg[fd] = copro1_reg[fs];
							break;
							case COP1_MINA:
								if (copro1_reg[ft] < copro1_reg[fs])
									copro1_reg[fd] = copro1_reg[ft];
								else
									copro1_reg[fd] = copro1_reg[fs];
							break;
							case COP1_SELEQZ:
								fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
								pc=0;
							break;
							case COP1_SELNEZ:
								fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
								pc=0;
							break;
							case COP1_RINT:
								fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
								pc=0;
							break;
							case COP1_CLASS:
								fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
								pc=0;
							break;
							case COP1_CVT_S_D:
							case COP1_CVT_D_S:

								copro1_reg[fd] = copro1_reg[fs];
							break;
							case COP1_MOV:
								copro1_reg[fd] = copro1_reg[fs];
							break;


							default:
								fprintf(stderr, "Encountering an unknown opcode in cop1 funct %x at address %x = %x\n", funct, pc, ins);
								pc=0;
							break;

							}
						break;
						case COP1_CMP_S:
						case COP1_CMP_D:
							switch (cop1_cond){
								case COP1_CMP_F:
									fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
									pc=0;
								break;
								case COP1_CMP_UN:
									copro1_reg[fd] = copro1_reg[ft] != copro1_reg[fs];
								break;
								case COP1_CMP_EQ:
									copro1_reg[fd] = copro1_reg[ft] == copro1_reg[fs];
								break;
								case COP1_CMP_UEQ:
									copro1_reg[fd] = copro1_reg[ft] == copro1_reg[fs];
								break;
								case COP1_CMP_LT:
									copro1_reg[fd] = copro1_reg[ft] < copro1_reg[fs];
								break;
								case COP1_CMP_ULT:
									copro1_reg[fd] = copro1_reg[ft] < copro1_reg[fs];
								break;
								case COP1_CMP_LE:
									copro1_reg[fd] = copro1_reg[ft] <= copro1_reg[fs];
								break;
								case COP1_CMP_ULE:
									copro1_reg[fd] = copro1_reg[ft] <= copro1_reg[fs];
								break;
								case COP1_CMP_SAF:
									fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
									pc=0;
								break;
								case COP1_CMP_SUN:
									copro1_reg[fd] = copro1_reg[ft] != copro1_reg[fs];
								break;
								case COP1_CMP_SEQ:
									copro1_reg[fd] = copro1_reg[ft] == copro1_reg[fs];
								break;
								case COP1_CMP_SUEQ:
									copro1_reg[fd] = copro1_reg[ft] == copro1_reg[fs];
								break;
								case COP1_CMP_SLT:
									copro1_reg[fd] = copro1_reg[ft] < copro1_reg[fs];
								break;
								case COP1_CMP_SULT:
									copro1_reg[fd] = copro1_reg[ft] < copro1_reg[fs];
								break;
								case COP1_CMP_SLE:
									copro1_reg[fd] = copro1_reg[ft] <= copro1_reg[fs];
								break;
								case COP1_CMP_SULE:
									copro1_reg[fd] = copro1_reg[ft] <= copro1_reg[fs];
								break;
								default:
									fprintf(stderr, "Encountering an unknown opcode in cop1_cmp funct %x at address %x = %x\n", cop1_cond, pc, ins);
								break;

								}
						break;
						case COP1_BCEQZ:
							fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
							pc=0;
						break;
						case COP1_BCNEZ:
							fprintf(stderr, "copop not handled yet %x = %x\n",  pc, ins);
							pc=0;
						break;
						case COP1_MTC1:
							ivalue = reg[ft];
							memcpy(&fvalue, &ivalue, sizeof(float));
							copro1_reg[fd] = fvalue;
						break;
						case COP1_MTHC1:
							ivalue = reg[ft] & 0xffff;
							memcpy(&fvalue, &ivalue, sizeof(float));
							copro1_reg[fd] = fvalue;
						break;
						case COP1_CVT_D_W:
							fvalue = copro1_reg[fs];
							memcpy(&ivalue, &fvalue, sizeof(float));

							copro1_reg[fd] = (float) ivalue;
						break;
						case COP1_MOV_ID:
							fvalue = copro1_reg[fs];
							memcpy(&ivalue, &fvalue, sizeof(float));

							copro1_reg[ft] = ivalue;
						break;
						default:
							fprintf(stderr, "Encountering an unknown opcode in cop1 format %x at address %x = %x\n", cop1_format, pc, ins);
							pc=0;
						break;

						}
						break;

					case COP3:
						//TODO
						break;

					case SEH:
						if (ivalue >= 32768) //handle the signed case
							ivalue = ivalue - 2*32768;
						else
							reg[rt] = reg[rs];
						break;


					default:
						fprintf(stderr, "Encountering an unknown opcode %x and funct %x at address %x = %x\n", op, funct, pc, ins);
						pc=0;
						break;
					}
					break;
			}
			reg[0] = 0;
			n_inst = n_inst + 1;
			for (int i=0; i<32; i++){
				fprintf(stderr,";%x", reg[i]);
			}
			fprintf(stderr, "\n");

		}
		while (pc != 0 && pc != 0xa00200a8 && n_inst<100000000); //Apparently, return from main function is always at address 0x4000074
		fprintf(stderr,"Simulation finished in %d cycles\n",n_inst);

		return 0;
	}
}

void Simulator::stb(int addr, unsigned int value){
	if (addr == 0x10009000)
		printf("%c", value&0xff);
	else
		this->memory[addr] = value & 0xff;
}

////Big endian version
//void Simulator::sth(int addr, unsigned int value){
//	this->memory[addr] = (value>>8) & 0xff;
//	this->memory[addr+1] = (value) & 0xff;
//}
//
//void Simulator::stw(int addr, unsigned int value){
//	this->memory[addr] = (value>>24) & 0xff;
//	this->memory[addr+1] = (value>>16) & 0xff;
//	this->memory[addr+2] = (value>>8) & 0xff;
//	this->memory[addr+3] = (value) & 0xff;
//}

//Little endian version
void Simulator::sth(int addr, unsigned int value){
	this->memory[addr+1] = (value>>8) & 0xff;
	this->memory[addr+0] = (value) & 0xff;
}

void Simulator::stw(int addr, unsigned int value){
	this->memory[addr+3] = (value>>24) & 0xff;
	this->memory[addr+2] = (value>>16) & 0xff;
	this->memory[addr+1] = (value>>8) & 0xff;
	this->memory[addr+0] = (value) & 0xff;
}


unsigned int Simulator::ldb(int addr){

	unsigned int result = 0;
	if (addr == 0x10009000){
		result = getchar();
	}
	else if (this->memory.find(addr) != this->memory.end())
		result = this->memory[addr];
	else
		result= 0;


	return result;
}

////Big endian version
//unsigned int Simulator::ldh(int addr){
//	unsigned int result = 0;
//	result += (this->ldb(addr) & 0xff) << 8;
//	result += this->ldb(addr+1) & 0xff;
//	return result;
//}
//
//unsigned int Simulator::ldw(int addr){
//	unsigned int result = 0;
//	result += (this->ldb(addr) & 0xff) << 24;
//	result += (this->ldb(addr+1) & 0xff) << 16;
//	result += (this->ldb(addr+2) & 0xff) << 8;
//	result += this->ldb(addr+3) & 0xff;
//	return result;
//}

//Little endian version
unsigned int Simulator::ldh(int addr){

	unsigned int result = 0;
	result += (this->ldb(addr+1) & 0xff) << 8;
	result += this->ldb(addr) & 0xff;
	return result;
}

unsigned int Simulator::ldw(int addr){

	unsigned int result = 0;
	result += (this->ldb(addr+3) & 0xff) << 24;
	result += (this->ldb(addr+2) & 0xff) << 16;
	result += (this->ldb(addr+1) & 0xff) << 8;
	result += this->ldb(addr+0) & 0xff;
	return result;
}

#endif
