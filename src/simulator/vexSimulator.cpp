/**************************************************************************
 * 
                       PipeLined Four Issues Processor  CUSTOM MOV
 * 
 **************************************************************************/



/*	This is a four ways pipelined (5 stages) prossecor simulator
*		All the ways can perform common operations
*		But some ways can perform further operations :
*		->	Way 1 can perform Branch operations too
*		->	Way 2 can perform Memory access operations too
* 		->	Way 4 can perform Multiplication operations too
*		
*		The different stages are :
*		-> Fetch (F) 		: Access Instruction registers and store current instruction
*		-> Decode (DC)		: Decode the instruction and select the needed operands for the next stage (including accessing to registers)
*		-> Execute (EX) 	: Do the calculating part
*		-> Memory (M)		: Access memory if needed (Only for Way 2) 
*		-> Write Back (WB) 	: Update registers value if needed
*
*		For any questions, please contact yo.uguen@gmail.com
*/


#ifndef __VLIW

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <simulator/vexSimulator.h>

#include <lib/ac_int.h> // This is a proprietary library (Mentor Graphics - Use term agreements included in the header)
#include <map>


using namespace std;

#define MAX_NUMBER_OF_INSTRUCTIONS 65536
#define REG_NUMBER 64
#define BRANCH_UNIT_NUMBER 8
#define DATA_SIZE 65536
#define SIZE_INSTRUCTION 128
#define SIZE_MEM_ADDR 16


#define __VERBOSE
//Declaration of different structs

struct FtoDC {
	ac_int<32, false> instruction; //Instruction to execute
};

struct DCtoEx {
	ac_int<32, true> dataa; //First data from register file
	ac_int<32, true> datab; //Second data, from register file or immediate value
	ac_int<32, true> datac; //Third data used only for store instruction and corresponding to rb
	ac_int<6, false> dest;  //Register to be written
	ac_int<7, false> opCode;//OpCode of the instruction
};

struct ExtoMem {
	ac_int<32, true> result;	//Result of the EX stage
	ac_int<32, true> datac;		//Data to be stored in memory (if needed)
	ac_int<6, false> dest;		//Register to be written at WB stage
	ac_int<1, false> WBena;		//Is a WB is needed ?
	ac_int<7, false> opCode;	//OpCode of the operation
};

struct MemtoWB {
	ac_int<32, true> result;	//Result to be written back
	ac_int<6, false> dest;		//Register to be written at WB stage
	ac_int<1, false> WBena;		//Is a WB is needed ?
};

// 32 bits registers

// 1 bit registers
ac_int<1, false> BREG[BRANCH_UNIT_NUMBER];
// Link register

std::map<unsigned int, ac_int<32, false>> memory;
ac_int<128, false> RI[MAX_NUMBER_OF_INSTRUCTIONS];


// Cycle counter

typedef ac_int<32, false> acuint32;
typedef ac_int<8, false> acuint8;

void VexSimulator::stb(unsigned int addr, ac_int<8, false> value){
	if (addr == 0x10009000)
		fprintf(stdout,"%c",(char) value & 0xff);
	else
		memory[addr] = value;
}

void VexSimulator::sth(unsigned int addr, ac_int<16, false> value){
	memory[addr+1] = value.slc<8>(8);//TODO ne pas ecraser...
	memory[addr] = value.slc<8>(0);
}

void VexSimulator::stw(unsigned int addr, ac_int<32, false> value){
	memory[addr+3] = value.slc<8>(24);
	memory[addr+2] = value.slc<8>(16);
	memory[addr+1] = value.slc<8>(8);
	memory[addr+0] = value.slc<8>(0);
}

ac_int<8, false> VexSimulator::ldb(unsigned int addr){
	ac_int<8, false>  result = 0;
	if (memory.find(addr) != memory.end())
		result = memory[addr];

	return result;
}

ac_int<16, false> VexSimulator::ldh(unsigned int addr){
	ac_int<16, false> result = 0;
	result.set_slc(8, ldb(addr+1));
	result.set_slc(0, ldb(addr));
	return result;
}

ac_int<32, false> VexSimulator::ldw(unsigned int addr){
	ac_int<32, false> result = 0;
	result.set_slc(24, ldb(addr+3));
	result.set_slc(16, ldb(addr+2));
	result.set_slc(8, ldb(addr+1));
	result.set_slc(0, ldb(addr+0));
	return result;
}


void VexSimulator::doWB(struct MemtoWB memtoWB){
	if(memtoWB.WBena){
			if (debugLevel >= 4)
				fprintf(stderr,"Writing in %d %d\n", (int) memtoWB.dest, (int) memtoWB.result);

			REG[memtoWB.dest] = memtoWB.result;
	}
}

void VexSimulator::doMemNoMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB){
	memtoWB->WBena = extoMem.WBena;
	memtoWB->dest = extoMem.dest;
	memtoWB->result = extoMem.result;
}

void VexSimulator::doMem(struct ExtoMem extoMem, struct MemtoWB *memtoWB){
	memtoWB->WBena = extoMem.WBena;
	memtoWB->dest = extoMem.dest;

	//IMPORTANT NOTE: In this function the value of exToMem.address is the address of the memory access to perform
	// 					and it is addressing byte. In current implementation DATA is a word array so we need to
	//					divide the address by four to have actual place inside the array

	if (extoMem.opCode.slc<3>(4) == 1){
		//The instruction is a memory access
		ac_int<32, false> address = extoMem.result;
		if (extoMem.opCode.slc<4>(0) <= 5){
			//The operation is a load instruction


			ac_int<16, false> const0_16 = 0;
			ac_int<16, false> const1_16 = 0xffff;
			ac_int<24, false> const0_24 = 0;
			ac_int<24, false> const1_24 = 0xffffff;
			ac_int<16, true> signed16Value;
			ac_int<8, true> signed8Value;


			switch(extoMem.opCode){
			case VEX_LDW:
				//ldw
				memtoWB->result = this->ldw(address);
				if (debugLevel >= 4)
					fprintf(stderr,"[Cycle %d ; PC %d] Loading %x at address %x\n",(int) cycle, (int) PC, (int) memtoWB->result, (int) address);

				break;
			case VEX_LDHU:
				//ldhu
				memtoWB->result = ldh(address);
				break;
			case VEX_LDH:
				//ldh
				signed16Value = ldh(address);
				memtoWB->result = signed16Value;
				break;
			case VEX_LDBU:
				//ldbu
				memtoWB->result = ldb(address);

				break;
			case VEX_LDB:
				//ldb
				signed8Value = ldb(address);
				memtoWB->result = signed8Value;
				break;
			default:
			break;
			}


		}
		else {
			//We are on a store instruction

			switch (extoMem.opCode){
			case 0x16:
				//STW
				stw(address, extoMem.datac);
				if (debugLevel >= 4)
					fprintf(stderr,"[Cycle %d ; PC %d] Storing %x at %x\n",(int) cycle, (int) PC, (int) extoMem.datac, (int) address);

				break;
			case 0x17:
				//STH
				sth(address, extoMem.datac.slc<16>(0));
				break;
			case 0x18:
				//STB
				stb(address, extoMem.datac.slc<8>(0));
				break;
			default:
			break;
			}

			memtoWB->WBena = 0; //TODO : this shouldn't be necessary : WB shouldn't be enabled before
		}
	}
	else
		memtoWB->result = extoMem.result;
}

void VexSimulator::doEx(struct DCtoEx dctoEx, struct ExtoMem *extoMem){

	ac_int<5, false> shiftAmount = dctoEx.datab.slc<5>(0);

	ac_int<32, true> addDataa = (dctoEx.opCode == VEX_SH1ADD) ? (dctoEx.dataa << 1) :
				(dctoEx.opCode == VEX_SH2ADD) ? (dctoEx.dataa << 2) :
				(dctoEx.opCode == VEX_SH3ADD) ? (dctoEx.dataa << 3) :
				(dctoEx.opCode == VEX_SH4ADD) ? (dctoEx.dataa << 4) :
				dctoEx.dataa ;

	ac_int<32, true> addDatab = (dctoEx.opCode == VEX_SH3bADD || dctoEx.opCode == VEX_SH3bADDi) ? (dctoEx.datab << 3) :
					dctoEx.datab ;


	ac_int<1, false> selectAdd = (dctoEx.opCode.slc<3>(4) == 0x1) //Memory instructions
			| (dctoEx.opCode == VEX_ADD) |  (dctoEx.opCode == VEX_SH1ADD) |  (dctoEx.opCode == VEX_SH2ADD)
			| (dctoEx.opCode == VEX_SH3ADD) |  (dctoEx.opCode == VEX_SH4ADD)
			| (dctoEx.opCode == VEX_ADDi) |  (dctoEx.opCode == VEX_SH1ADDi) |  (dctoEx.opCode == VEX_SH2ADDi)
			| (dctoEx.opCode == VEX_SH3ADDi) |  (dctoEx.opCode == VEX_SH4ADDi) | (dctoEx.opCode == VEX_SH3bADD) |  (dctoEx.opCode == VEX_SH3bADDi) ;

	ac_int<1, false> selectSub = (dctoEx.opCode == VEX_SUB)| (dctoEx.opCode == VEX_SUBi);
	ac_int<1, false> selectSll = (dctoEx.opCode == VEX_SLL)| (dctoEx.opCode == VEX_SLLi);
	ac_int<1, false> selectSrl = (dctoEx.opCode == VEX_SRL)| (dctoEx.opCode == VEX_SRLi);
	ac_int<1, false> selectSra = (dctoEx.opCode == VEX_SRA)| (dctoEx.opCode == VEX_SRAi);
	ac_int<1, false> selectAnd = (dctoEx.opCode == VEX_AND)| (dctoEx.opCode == VEX_ANDi);
	ac_int<1, false> selectOr = (dctoEx.opCode == VEX_OR)| (dctoEx.opCode == VEX_ORi);
	ac_int<1, false> selectNot = (dctoEx.opCode == VEX_NOT)| (dctoEx.opCode == VEX_NOTi);
	ac_int<1, false> selectXor = (dctoEx.opCode == VEX_XOR)| (dctoEx.opCode == VEX_XORi);
	ac_int<1, false> selectNor = (dctoEx.opCode == VEX_NOR)| (dctoEx.opCode == VEX_NORi);
	ac_int<1, false> selectCmp = (dctoEx.opCode == VEX_CMPLT)| (dctoEx.opCode == VEX_CMPLTi)
			| (dctoEx.opCode == VEX_CMPLTU)| (dctoEx.opCode == VEX_CMPLTUi)
			| (dctoEx.opCode == VEX_CMPNE)| (dctoEx.opCode == VEX_CMPNEi)
			| (dctoEx.opCode == VEX_CMPEQ)| (dctoEx.opCode == VEX_CMPEQi)
			| (dctoEx.opCode == VEX_CMPGE)| (dctoEx.opCode == VEX_CMPGEi)
			| (dctoEx.opCode == VEX_CMPGEU)| (dctoEx.opCode == VEX_CMPGEUi)
			| (dctoEx.opCode == VEX_CMPGT)| (dctoEx.opCode == VEX_CMPGTi)
			| (dctoEx.opCode == VEX_CMPGTU)| (dctoEx.opCode == VEX_CMPGTUi)
			| (dctoEx.opCode == VEX_CMPLE)| (dctoEx.opCode == VEX_CMPLEi)
			| (dctoEx.opCode == VEX_CMPLEU)| (dctoEx.opCode == VEX_CMPLEUi);



	ac_int<32, false> unsigned_dataa = dctoEx.dataa;
	ac_int<32, false> unsigned_datab = dctoEx.datab;

	ac_int<32, true> add_result = addDataa + addDatab;
	ac_int<32, true> sub_result = dctoEx.dataa - dctoEx.datab;
	ac_int<32, true> sl_result = dctoEx.dataa << shiftAmount;
	ac_int<32, true> srl_result = unsigned_dataa >> shiftAmount;
	ac_int<32, true> sra_result = dctoEx.dataa >> shiftAmount;

	ac_int<32, true> and_result = dctoEx.dataa & dctoEx.datab;
	ac_int<32, true> or_result = dctoEx.dataa | dctoEx.datab;
	ac_int<32, true> not_result = ~dctoEx.dataa;
	ac_int<32, true> xor_result = dctoEx.dataa ^ dctoEx.datab;
	ac_int<32, true> nor_result = ~(dctoEx.dataa | dctoEx.datab);

	ac_int<32, true> unsigned_sub_result = unsigned_dataa - unsigned_datab;

	ac_int<1, false> cmpResult_1 = ((dctoEx.opCode == VEX_CMPLT) | (dctoEx.opCode == VEX_CMPLTi)) ? sub_result < 0 :
			((dctoEx.opCode == VEX_CMPLTU)| (dctoEx.opCode == VEX_CMPLTUi)) ? unsigned_dataa < unsigned_datab :
			((dctoEx.opCode == VEX_CMPNE)| (dctoEx.opCode == VEX_CMPNEi)) ? sub_result != 0 :
			((dctoEx.opCode == VEX_CMPEQ)| (dctoEx.opCode == VEX_CMPEQi)) ? sub_result == 0 :
			((dctoEx.opCode == VEX_CMPGE)| (dctoEx.opCode == VEX_CMPGEi)) ? sub_result >= 0 :
			((dctoEx.opCode == VEX_CMPGEU)| (dctoEx.opCode == VEX_CMPGEUi)) ? unsigned_sub_result >=0 :
			((dctoEx.opCode == VEX_CMPGT)| (dctoEx.opCode == VEX_CMPGTi)) ? sub_result > 0 :
			((dctoEx.opCode == VEX_CMPGTU)| (dctoEx.opCode == VEX_CMPGTUi)) ? unsigned_sub_result > 0 :
			((dctoEx.opCode == VEX_CMPLE)| (dctoEx.opCode == VEX_CMPLEi)) ? sub_result <= 0 :
			sub_result <= 0;

	ac_int<32, true> cmpResult = cmpResult_1;

	extoMem->result = selectAdd ? add_result :
			selectSub ? sub_result :
			selectSll ? sl_result :
			selectSra ? sra_result :
			selectSrl ? srl_result :
			selectAnd ? and_result :
			selectOr ? or_result :
			selectNot ? not_result :
			selectXor ? xor_result :
			selectNor ? nor_result :
			selectCmp ? cmpResult :
			dctoEx.dataa;


	extoMem->WBena = !(dctoEx.opCode == VEX_NOP); //TODO
	extoMem->datac = dctoEx.datac;
	extoMem->dest = dctoEx.dest;
	extoMem->opCode = dctoEx.opCode;
}

void VexSimulator::doExMult(struct DCtoEx dctoEx, struct ExtoMem *extoMem){

	ac_int<5, false> shiftAmount = dctoEx.datab.slc<5>(0);

	ac_int<32, true> addDataa = (dctoEx.opCode == VEX_SH1ADD) ? (dctoEx.dataa << 1) :
				(dctoEx.opCode == VEX_SH2ADD) ? (dctoEx.dataa << 2) :
				(dctoEx.opCode == VEX_SH3ADD) ? (dctoEx.dataa << 3) :
				(dctoEx.opCode == VEX_SH4ADD) ? (dctoEx.dataa << 4) :
				dctoEx.dataa ;

	ac_int<32, true> addDatab = (dctoEx.opCode == VEX_SH3bADD || dctoEx.opCode == VEX_SH3bADDi) ? (dctoEx.datab << 3) :
					dctoEx.datab ;


	ac_int<1, false> selectAdd = (dctoEx.opCode.slc<3>(4) == 0x1) //Memory instructions
			| (dctoEx.opCode == VEX_ADD) |  (dctoEx.opCode == VEX_SH1ADD) |  (dctoEx.opCode == VEX_SH2ADD)
			| (dctoEx.opCode == VEX_SH3ADD) |  (dctoEx.opCode == VEX_SH4ADD)
			| (dctoEx.opCode == VEX_ADDi) |  (dctoEx.opCode == VEX_SH1ADDi) |  (dctoEx.opCode == VEX_SH2ADDi)
			| (dctoEx.opCode == VEX_SH3ADDi) |  (dctoEx.opCode == VEX_SH4ADDi) | (dctoEx.opCode == VEX_SH3bADD) |  (dctoEx.opCode == VEX_SH3bADDi) ;

	ac_int<1, false> selectSub = (dctoEx.opCode == VEX_SUB)| (dctoEx.opCode == VEX_SUBi);
	ac_int<1, false> selectSll = (dctoEx.opCode == VEX_SLL)| (dctoEx.opCode == VEX_SLLi);
	ac_int<1, false> selectSrl = (dctoEx.opCode == VEX_SRL)| (dctoEx.opCode == VEX_SRLi);
	ac_int<1, false> selectSra = (dctoEx.opCode == VEX_SRA)| (dctoEx.opCode == VEX_SRAi);
	ac_int<1, false> selectAnd = (dctoEx.opCode == VEX_AND)| (dctoEx.opCode == VEX_ANDi);
	ac_int<1, false> selectOr = (dctoEx.opCode == VEX_OR)| (dctoEx.opCode == VEX_ORi);
	ac_int<1, false> selectNot = (dctoEx.opCode == VEX_NOT)| (dctoEx.opCode == VEX_NOTi);
	ac_int<1, false> selectXor = (dctoEx.opCode == VEX_XOR)| (dctoEx.opCode == VEX_XORi);
	ac_int<1, false> selectNor = (dctoEx.opCode == VEX_NOR)| (dctoEx.opCode == VEX_NORi);
	ac_int<1, false> selectCmp = (dctoEx.opCode == VEX_CMPLT)| (dctoEx.opCode == VEX_CMPLTi)
			| (dctoEx.opCode == VEX_CMPLTU)| (dctoEx.opCode == VEX_CMPLTUi)
			| (dctoEx.opCode == VEX_CMPNE)| (dctoEx.opCode == VEX_CMPNEi)
			| (dctoEx.opCode == VEX_CMPEQ)| (dctoEx.opCode == VEX_CMPEQi)
			| (dctoEx.opCode == VEX_CMPGE)| (dctoEx.opCode == VEX_CMPGEi)
			| (dctoEx.opCode == VEX_CMPGEU)| (dctoEx.opCode == VEX_CMPGEUi)
			| (dctoEx.opCode == VEX_CMPGT)| (dctoEx.opCode == VEX_CMPGTi)
			| (dctoEx.opCode == VEX_CMPGTU)| (dctoEx.opCode == VEX_CMPGTUi)
			| (dctoEx.opCode == VEX_CMPLE)| (dctoEx.opCode == VEX_CMPLEi)
			| (dctoEx.opCode == VEX_CMPLEU)| (dctoEx.opCode == VEX_CMPLEUi);



	ac_int<32, false> unsigned_dataa = dctoEx.dataa;
	ac_int<32, false> unsigned_datab = dctoEx.datab;

	ac_int<32, true> add_result = addDataa + addDatab;
	ac_int<32, true> sub_result = dctoEx.dataa - dctoEx.datab;
	ac_int<32, true> sl_result = dctoEx.dataa << shiftAmount;
	ac_int<32, true> srl_result = unsigned_dataa >> shiftAmount;
	ac_int<32, true> sra_result = dctoEx.dataa >> shiftAmount;

	ac_int<32, true> and_result = dctoEx.dataa & dctoEx.datab;
	ac_int<32, true> or_result = dctoEx.dataa | dctoEx.datab;
	ac_int<32, true> not_result = ~dctoEx.dataa;
	ac_int<32, true> xor_result = dctoEx.dataa ^ dctoEx.datab;
	ac_int<32, true> nor_result = ~(dctoEx.dataa | dctoEx.datab);

	ac_int<64, true> mul_result = dctoEx.dataa * dctoEx.datab;
	ac_int<32, true> mullo_result = mul_result.slc<32>(0);
	ac_int<32, true> mulhi_result = mul_result.slc<32>(32);

	ac_int<33, true> const0 = 0;
	ac_int<32, true> divlo_result = !dctoEx.datab ? const0 : dctoEx.dataa / dctoEx.datab;
	ac_int<32, true> divhi_result = !dctoEx.datab ? dctoEx.datab : dctoEx.dataa % dctoEx.datab;

	ac_int<32, true> unsigned_sub_result = unsigned_dataa - unsigned_datab;

	ac_int<1, false> cmpResult_1 = ((dctoEx.opCode == VEX_CMPLT) | (dctoEx.opCode == VEX_CMPLTi)) ? sub_result < 0 :
			((dctoEx.opCode == VEX_CMPLTU)| (dctoEx.opCode == VEX_CMPLTUi)) ? unsigned_sub_result < 0 :
			((dctoEx.opCode == VEX_CMPNE)| (dctoEx.opCode == VEX_CMPNEi)) ? sub_result != 0 :
			((dctoEx.opCode == VEX_CMPEQ)| (dctoEx.opCode == VEX_CMPEQi)) ? unsigned_sub_result == 0 :
			((dctoEx.opCode == VEX_CMPGE)| (dctoEx.opCode == VEX_CMPGEi)) ? sub_result >= 0 :
			((dctoEx.opCode == VEX_CMPGEU)| (dctoEx.opCode == VEX_CMPGEUi)) ? unsigned_sub_result >=0 :
			((dctoEx.opCode == VEX_CMPGT)| (dctoEx.opCode == VEX_CMPGTi)) ? sub_result > 0 :
			((dctoEx.opCode == VEX_CMPGTU)| (dctoEx.opCode == VEX_CMPGTUi)) ? unsigned_sub_result > 0 :
			((dctoEx.opCode == VEX_CMPLE)| (dctoEx.opCode == VEX_CMPLEi)) ? sub_result <= 0 :
			sub_result <= 0;

	ac_int<32, true> cmpResult = cmpResult_1;

	extoMem->result = selectAdd ? add_result :
			selectSub ? sub_result :
			selectSll ? sl_result :
			selectSra ? sra_result :
			selectSrl ? srl_result :
			selectAnd ? and_result :
			selectOr ? or_result :
			selectNot ? not_result :
			selectXor ? xor_result :
			selectNor ? nor_result :
			selectCmp ? cmpResult :
			(dctoEx.opCode == VEX_MPYLO) ? mullo_result :
			(dctoEx.opCode == VEX_MPYHI) ? mulhi_result :
			(dctoEx.opCode == VEX_DIVLO) ? divlo_result :
			(dctoEx.opCode == VEX_DIVHI) ? divhi_result :
			dctoEx.dataa;


	extoMem->WBena = !(dctoEx.opCode == VEX_NOP); //TODO
	extoMem->datac = dctoEx.datac;
	extoMem->dest = dctoEx.dest;
	extoMem->opCode = dctoEx.opCode;
}

void VexSimulator::doDC(struct FtoDC ftoDC, struct DCtoEx *dctoEx){
	ac_int<6, false> RA = ftoDC.instruction.slc<6>(26);
	ac_int<6, false> RB = ftoDC.instruction.slc<6>(20);
	ac_int<6, false> RC = ftoDC.instruction.slc<6>(14);
	ac_int<19, false> IMM19 = ftoDC.instruction.slc<19>(7);
	ac_int<13, false> IMM13 = ftoDC.instruction.slc<13>(7);
	ac_int<7, false> OP = ftoDC.instruction.slc<7>(0);
	ac_int<3, false> BEXT = ftoDC.instruction.slc<3>(8);
	ac_int<9, false> IMM9 = ftoDC.instruction.slc<9>(11);

	ac_int<1, false> isIType = (OP.slc<3>(4) == 2);
	ac_int<1, false> isImm = OP.slc<3>(4) == 1 || OP.slc<3>(4) == 6 || OP.slc<3>(4) == 7;

	ac_int<1, false> isUnsigned = (OP == VEX_MPYLLU) | (OP == VEX_MPYLHU) | (OP == VEX_MPYHHU) | (OP == VEX_MPYLU)
			| (OP == VEX_MPYHU) | (OP == VEX_CMPLTU) | (OP == VEX_CMPLTUi) | (OP == VEX_CMPGEU) | (OP == VEX_CMPGEUi)
			| (OP == VEX_ZXTB)  | (OP == VEX_ZXTBi) | (OP == VEX_ZXTH) | (OP == VEX_ZXTHi) | (OP == VEX_CMPGTU)
			| (OP == VEX_CMPGTUi) | (OP == VEX_CMPLEU) | (OP == VEX_CMPLEUi) | (OP == VEX_ORi)
			| (OP == VEX_ANDi)| (OP == VEX_XORi)| (OP == VEX_NOTi)| (OP == VEX_NORi) ;


	ac_int<23, false> const0_23 = 0;
	ac_int<23, false> const1_23 = 0x7fffff;
	ac_int<19, false> const0_19 = 0;
	ac_int<19, false> const1_19 = 0x7ffff;
	ac_int<13, false> const0_13 = 0;
	ac_int<13, false> const1_13 = 0x1fff;

	ac_int<6, false> secondRegAccess = RB;


	ac_int<32, true> regValueA = REG[RA];
	ac_int<32, true> regValueB = REG[secondRegAccess];

	dctoEx->opCode = OP;
	dctoEx->datac = regValueB; //For store instructions

	if (isIType){
		//The instruction is I type
		dctoEx->dest = RA;
		dctoEx->dataa.set_slc(0, IMM19);
		if (IMM19[18])
			dctoEx->dataa.set_slc(19, const1_13);
		else
			dctoEx->dataa.set_slc(19, const0_13);
	}
	else{
		//The instruction is R type
		dctoEx->dataa = regValueA;

		if (isImm){
			dctoEx->dest = RB;
			dctoEx->datab.set_slc(0, IMM13);
			if (IMM13[12] && !isUnsigned)
				dctoEx->datab.set_slc(13, const1_19);
			else
				dctoEx->datab.set_slc(13, const0_19);
		}
		else{
			dctoEx->dest = RC;
			dctoEx->datab = regValueB;
		}
	}
}

void VexSimulator::doDCBr(struct FtoDC ftoDC, struct DCtoEx *dctoEx){
	ac_int<6, false> RA = ftoDC.instruction.slc<6>(26);
	ac_int<6, false> RB = ftoDC.instruction.slc<6>(20);
	ac_int<6, false> RC = ftoDC.instruction.slc<6>(14);
	ac_int<19, false> IMM19 = ftoDC.instruction.slc<19>(7);
	ac_int<13, false> IMM13 = ftoDC.instruction.slc<13>(7);
	ac_int<7, false> OP = ftoDC.instruction.slc<7>(0);
	ac_int<3, false> BEXT = ftoDC.instruction.slc<3>(8);
	ac_int<9, false> IMM9 = ftoDC.instruction.slc<9>(11);

	ac_int<1, false> isIType = (OP.slc<3>(4) == 2);
	ac_int<1, false> isImm = OP.slc<3>(4) == 1 || OP.slc<3>(4) == 6 || OP.slc<3>(4) == 7;

	ac_int<1, false> isUnsigned = (OP == VEX_MPYLLU) | (OP == VEX_MPYLHU) | (OP == VEX_MPYHHU) | (OP == VEX_MPYLU)
			| (OP == VEX_MPYHU) | (OP == VEX_CMPLTU) | (OP == VEX_CMPLTUi) | (OP == VEX_CMPGEU) | (OP == VEX_CMPGEUi)
			| (OP == VEX_ZXTB)  | (OP == VEX_ZXTBi) | (OP == VEX_ZXTH) | (OP == VEX_ZXTHi) | (OP == VEX_CMPGTU)
			| (OP == VEX_CMPGTUi) | (OP == VEX_CMPLEU) | (OP == VEX_CMPLEUi) | (OP == VEX_ORi)
			| (OP == VEX_ANDi)| (OP == VEX_XORi)| (OP == VEX_NOTi)| (OP == VEX_NORi) ;


	ac_int<23, false> const0_23 = 0;
	ac_int<23, false> const1_23 = 0x7fffff;
	ac_int<19, false> const0_19 = 0;
	ac_int<19, false> const1_19 = 0x7ffff;
	ac_int<13, false> const0_13 = 0;
	ac_int<13, false> const1_13 = 0x1fff;

	ac_int<6, false> secondRegAccess = RB;


	ac_int<32, true> regValueA = REG[RA];
	ac_int<32, true> regValueB = REG[secondRegAccess];

	dctoEx->opCode = OP;
	dctoEx->datac = regValueB;

	if (isIType){
		//The instruction is I type
		dctoEx->dest = RA;
		dctoEx->dataa.set_slc(0, IMM19);
		if (IMM19[18])
			dctoEx->dataa.set_slc(19, const1_13);
		else
			dctoEx->dataa.set_slc(19, const0_13);
	}
	else{
		//The instruction is R type
		dctoEx->dataa = regValueA;

		if (isImm){
			dctoEx->dest = RB;
			dctoEx->datab.set_slc(0, IMM13);
			if (IMM13[12] && !isUnsigned)
				dctoEx->datab.set_slc(13, const1_19);
			else
				dctoEx->datab.set_slc(13, const0_19);
		}
		else{
			dctoEx->dest = RC;
			dctoEx->datab = regValueB;
		}
	}
	switch(OP){ // Select the right operation and place the right values into the right operands
			case VEX_GOTO:
				dctoEx->opCode = 0;
				NEXT_PC = IMM19;
				break;	// GOTO1

			case VEX_CALL:
				dctoEx->opCode = 0;
				REG[63] = PC + 1;
				NEXT_PC = IMM19;

				if (debugLevel >= 3){
					fprintf(stderr, "[Cycle %d / PC %d] Call to %d (around %x in MIPS)\n", cycle, PC, (int) IMM19,(int) IMM19*4+0xa0020040);
				}
				break; // CALL

			case VEX_ICALL :
				dctoEx->opCode = 0;
				REG[63] = PC + 1;
				NEXT_PC = regValueA;
				break; // ICALL

			case VEX_IGOTO :
				dctoEx->opCode = 0;
				NEXT_PC = regValueA;
				if (debugLevel >= 3)
					fprintf(stderr, "[Cycle %d / PC %d] Returning...\n", cycle, PC);
				break; //IGOTO

			case VEX_BR :
				dctoEx->opCode = 0;
				if(regValueA)
					NEXT_PC = PC + dctoEx->dataa -1;

				if (debugLevel >= 4)
					fprintf(stderr,"nextpc %d (with %d and %d)\n", (int) NEXT_PC, (int) PC, (int) dctoEx->dataa);
				break;	// BR

			case VEX_BRF :
				dctoEx->opCode = 0;
				if(!regValueA)
					NEXT_PC = PC + dctoEx->dataa -1;
				break;	// BRF

			case VEX_RETURN :
				dctoEx->opCode = 0;
				REG[1] = REG[1] + IMM19;
				NEXT_PC = REG[63];
				if (debugLevel >= 4)
					fprintf(stderr,"[Cycle %d] Returning (PC=%d)!  returned value is %x\n", (int) cycle, (int) NEXT_PC, (int) REG[9]);
				break; // RETURN
			default:
				break;
			}
}



int VexSimulator::run(int mainPc){

	// Initialise program counter
	PC = mainPc;

	/*********************************************************************
	 * Definition and initialization of all pipeline registers
	 *********************************************************************/
	struct MemtoWB memtoWB1;	struct MemtoWB memtoWB2;	struct MemtoWB memtoWB3;	struct MemtoWB memtoWB4;	struct MemtoWB memtoWB5; 	struct MemtoWB memtoWB6;	struct MemtoWB memtoWB7;	struct MemtoWB memtoWB8;
	memtoWB1.WBena = 0;	memtoWB2.WBena = 0;	memtoWB3.WBena = 0;	memtoWB4.WBena = 0;	memtoWB5.WBena = 0;	memtoWB6.WBena = 0;	memtoWB7.WBena = 0;	memtoWB8.WBena = 0;

	struct ExtoMem extoMem1;	struct ExtoMem extoMem2;	struct ExtoMem extoMem3;	struct ExtoMem extoMem4;	struct ExtoMem extoMem5;	struct ExtoMem extoMem6;	struct ExtoMem extoMem7;	struct ExtoMem extoMem8;
	extoMem1.WBena = 0;	extoMem2.WBena = 0;	extoMem3.WBena = 0;	extoMem4.WBena = 0;	extoMem5.WBena = 0;	extoMem6.WBena = 0;	extoMem7.WBena = 0;	extoMem8.WBena = 0;
	extoMem1.opCode = 0; extoMem2.opCode = 0; extoMem3.opCode = 0; extoMem4.opCode = 0; extoMem5.opCode = 0; extoMem6.opCode = 0; extoMem7.opCode = 0; extoMem8.opCode = 0;

	struct DCtoEx dctoEx1; struct DCtoEx dctoEx2;	struct DCtoEx dctoEx3;	struct DCtoEx dctoEx4;	struct DCtoEx dctoEx5;	struct DCtoEx dctoEx6;	struct DCtoEx dctoEx7;	struct DCtoEx dctoEx8;
	dctoEx1.opCode = 0;	dctoEx2.opCode = 0;	dctoEx3.opCode = 0;	dctoEx4.opCode = 0;	dctoEx5.opCode = 0;	dctoEx6.opCode = 0;	dctoEx7.opCode = 0;	dctoEx8.opCode = 0;

	struct FtoDC ftoDC1;	struct FtoDC ftoDC2;	struct FtoDC ftoDC3;	struct FtoDC ftoDC4;	struct FtoDC ftoDC5;	struct FtoDC ftoDC6;	struct FtoDC ftoDC7;	struct FtoDC ftoDC8;
	ftoDC1.instruction = 0;	ftoDC2.instruction = 0;	ftoDC3.instruction = 0;	ftoDC4.instruction = 0;	ftoDC5.instruction = 0;	ftoDC6.instruction = 0;	ftoDC7.instruction = 0;	ftoDC8.instruction = 0;



	int stop = 0;


	// Entering the processor loop
	// Stages are reversed so that it does not erase earlier content

	while (PC < MAX_NUMBER_OF_INSTRUCTIONS){


		///////////////////////////////////////////////////////
		//													 //
		//                         EX                        //
		//													 //
		///////////////////////////////////////////////////////


		doEx(dctoEx4, &extoMem4);
		doEx(dctoEx1, &extoMem1);
		doExMult(dctoEx3, &extoMem3);
		doEx(dctoEx2, &extoMem2);
//		doEx(dctoEx5, &extoMem5);
//		doExMem(dctoEx6, &extoMem6);
//		doEx(dctoEx7, &extoMem7);
//		doExMult(dctoEx8, &extoMem8);



		///////////////////////////////////////////////////////
		//													 //
		//                       M                           //
		//													 //
		///////////////////////////////////////////////////////




		// If the operation code is 0x2f then the processor stops
		if(stop == 1){
			return PC;
		}


		doMemNoMem(extoMem1, &memtoWB1);
		doMemNoMem(extoMem3, &memtoWB3);
		doMemNoMem(extoMem4, &memtoWB4);
//		doMemNoMem(extoMem5, &memtoWB5);
//		doMemNoMem(extoMem7, &memtoWB7);
//		doMemNoMem(extoMem8, &memtoWB8);

		doMem(extoMem2, &memtoWB2);
//		doMem(extoMem6, &memtoWB6, DATA0, DATA1, DATA2, DATA3);



		///////////////////////////////////////////////////////
		//													 //
		//                       WB                          //
		//  												 //
		///////////////////////////////////////////////////////

		doWB(memtoWB1);
		doWB(memtoWB2);
		doWB(memtoWB3);
		doWB(memtoWB4);
//		doWB(memtoWB5);
//		doWB(memtoWB6);
//		doWB(memtoWB7);
//		doWB(memtoWB8);

		///////////////////////////////////////////////////////
		//													 //
		//                       DC                          //
		//													 //
		///////////////////////////////////////////////////////

		NEXT_PC = PC+1;
		doDCBr(ftoDC1, &dctoEx1);
		doDC(ftoDC2, &dctoEx2);
		doDC(ftoDC3, &dctoEx3);
		doDC(ftoDC4, &dctoEx4);
//		doDC(ftoDC5, &dctoEx5);
//		doDC(ftoDC6, &dctoEx6);
//		doDC(ftoDC7, &dctoEx7);
//		doDC(ftoDC8, &dctoEx8);

		ac_int<7, false> OP1 = ftoDC1.instruction.slc<7>(0);

		// If the operation code is 0x2f then the processor stops
		if(OP1 == 0x2F){
			stop = 1;

			#ifdef __VERBOSE
			fprintf(stderr,"Simulation finished in %d cycles \n", cycle);
			#endif
		}


		///////////////////////////////////////////////////////
		//                       F                           //
		///////////////////////////////////////////////////////


		// Retrieving new instruction
		ac_int<SIZE_INSTRUCTION, false> vliw = RI[PC];

		// Redirect instructions to thier own ways
		ftoDC1.instruction = vliw.slc<32>(96);
		ftoDC2.instruction = vliw.slc<32>(64);
		ftoDC3.instruction = vliw.slc<32>(32);
		ftoDC4.instruction = vliw.slc<32>(0);
//		ftoDC5.instruction = vliw.slc<32>(96);
//		ftoDC6.instruction = vliw.slc<32>(64);
//		ftoDC7.instruction = vliw.slc<32>(32);
//		ftoDC8.instruction = vliw.slc<32>(0);

		int pcValueForDebug = PC;
		// Next instruction
		PC = NEXT_PC;
		cycle++;

		// DISPLAY


		if (debugLevel >= 1){
			fprintf(stderr, "%d;%d; ", (int) cycle, (int) pcValueForDebug);
			printDecodedInstr(ftoDC1.instruction); fprintf(stderr, " ");
			printDecodedInstr(ftoDC2.instruction); fprintf(stderr, " ");
			printDecodedInstr(ftoDC3.instruction); fprintf(stderr, " ");
			printDecodedInstr(ftoDC4.instruction); fprintf(stderr, " ");
		}

		if (debugLevel >= 1){
			for (int i = 0; i<34; i++)
				fprintf(stderr, ";%x", (int) REG[i]);

			fprintf(stderr, "\n");
		}
	}

	return PC;
}

void VexSimulator::initializeDataMemory(unsigned char* content, unsigned int size, unsigned int start){
	for (int i = 0; i<size; i++){
		ac_int<8, false> oneByte = content[i];
		stb(i+start, oneByte);
	}
}

void VexSimulator::initializeDataMemory(ac_int<32, false>* content, unsigned int size, unsigned int start){
	for (int i = 0; i<size; i+=4){
		stw(i+start, content[i>>2]);
	}
}

void VexSimulator::initializeCodeMemory(unsigned char* content, unsigned int size, unsigned int start){
	for (int i = 0; i<size/16; i++){
		ac_int<8, false> instr1a = content[16*i];
		ac_int<8, false> instr1b = content[16*i+1];
		ac_int<8, false> instr1c = content[16*i+2];
		ac_int<8, false> instr1d = content[16*i+3];

		ac_int<32, false> instr1 = 0;
		instr1.set_slc(24, instr1d);
		instr1.set_slc(16, instr1c);
		instr1.set_slc(8, instr1b);
		instr1.set_slc(0, instr1a);

		//*********** Instr2
		ac_int<8, false> instr2a = content[16*i+4];
		ac_int<8, false> instr2b = content[16*i+5];
		ac_int<8, false> instr2c = content[16*i+6];
		ac_int<8, false> instr2d = content[16*i+7];

		ac_int<32, false> instr2 = 0;
		instr2.set_slc(24, instr2d);
		instr2.set_slc(16, instr2c);
		instr2.set_slc(8, instr2b);
		instr2.set_slc(0, instr2a);

		ac_int<8, false> instr3a = content[16*i+8];
		ac_int<8, false> instr3b = content[16*i+9];
		ac_int<8, false> instr3c = content[16*i+10];
		ac_int<8, false> instr3d = content[16*i+11];

		ac_int<32, false> instr3 = 0;
		instr3.set_slc(24, instr3d);
		instr3.set_slc(16, instr3c);
		instr3.set_slc(8, instr3b);
		instr3.set_slc(0, instr3a);

		ac_int<8, false> instr4a = content[16*i+12];
		ac_int<8, false> instr4b = content[16*i+13];
		ac_int<8, false> instr4c = content[16*i+14];
		ac_int<8, false> instr4d = content[16*i+15];

		ac_int<32, false> instr4 = 0;
		instr4.set_slc(24, instr4d);
		instr4.set_slc(16, instr4c);
		instr4.set_slc(8, instr4b);
		instr4.set_slc(0, instr4a);

		RI[i+start].set_slc(96, instr1);
		RI[i+start].set_slc(64, instr2);
		RI[i+start].set_slc(32, instr3);
		RI[i+start].set_slc(0, instr4);

		if (this->debugLevel >= 1){
			fprintf(stderr, "objdump;%d; ", (int) i);
			printDecodedInstr(instr1); fprintf(stderr, " ");
			printDecodedInstr(instr2); fprintf(stderr, " ");
			printDecodedInstr(instr3); fprintf(stderr, " ");
			printDecodedInstr(instr4); fprintf(stderr, "\n");
		}

	}
}

void VexSimulator::initializeCodeMemory(ac_int<128, false>* content, unsigned int size, unsigned int start){
	for (int i = 0; i<size/16; i++){

		RI[i+start]= content[i];



		if (this->debugLevel >= 1){
			fprintf(stderr, "objdump;%d; ", (int) i);
			printDecodedInstr(RI[i+start].slc<32>(0)); fprintf(stderr, " ");
			printDecodedInstr(RI[i+start].slc<32>(32)); fprintf(stderr, " ");
			printDecodedInstr(RI[i+start].slc<32>(64)); fprintf(stderr, " ");
			printDecodedInstr(RI[i+start].slc<32>(96)); fprintf(stderr, "\n");
		}

	}
}


#endif
