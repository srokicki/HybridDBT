#include <types.h>
#include <isa/vexISA.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <strings.h>


#ifndef __SW
#ifndef __HW

ac_int<32, false> assembleIInstruction(ac_int<7, false> opcode, ac_int<19, false> imm19, ac_int<6, false> regA){
	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, imm19);
	result.set_slc(26, regA);
	return result;
}

ac_int<32, false> assembleRInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<6, false> regB){
	ac_int<32, false> result = 0;
	ac_int<1, false> const0 = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, const0); //Immediate bit is equal to zero
	result.set_slc(14, regDest);
	result.set_slc(20, regB);
	result.set_slc(26, regA);
	return result;
}

ac_int<32, false> assembleRiInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<13, false> imm13){
	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, imm13);
	result.set_slc(20, regDest);
	result.set_slc(26, regA);
	return result;
}

ac_int<32, false> assembleFPInstruction(ac_int<7, false> opcode, ac_int<5, false> funct, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<6, false> regB){
	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, funct);
	result.set_slc(14, regDest);
	result.set_slc(20, regB);
	result.set_slc(26, regA);
	return result;
}

ac_int<32, false> assembleRRInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<6, false> regB, ac_int<6, false> regC){
	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(8, regDest);
	result.set_slc(14, regC);
	result.set_slc(20, regB);
	result.set_slc(26, regA);
	return result;
}

ac_int<32, false> assembleMemoryInstruction(ac_int<7, false> opcode, ac_int<6, false> regDest, ac_int<6, false> regA, ac_int<12, false> imm12, ac_int<1, false> isSpec, ac_int<5, false> specID){

	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, isSpec);

	if (!isSpec){
		result.set_slc(8, imm12);
	}
	else {
		result.set_slc(8, specID);
		result.set_slc(13, imm12.slc<7>(0));

	}
	result.set_slc(20, regDest);
	result.set_slc(26, regA);
	return result;
}

#endif
#endif


/*
 * Same assembly functions but which do not use ac_int
 */

unsigned int  assembleFPInstruction_sw(char opcode, char funct, char regDest, char regA, char regB){
	unsigned int result = 0;
	result += opcode & 0x7f;
	result += (funct & 0x1f) << 7;
	result += (regDest & 0x3f) << 14;
	result += (regB & 0x3f) << 20;
	result += (regA & 0x3f) << 26;



	return result;
}


unsigned int assembleIInstruction_sw(char opcode, int imm19, char regA){
	unsigned int result = 0;
	result += opcode & 0x7f;
	result += (imm19 & 0x7ffff)<<7;
	result += (regA & 0x3f) << 26;

	return result;
}

unsigned int assembleRInstruction_sw(char opcode, char regDest, char regA, char regB){
	unsigned int result = 0;
	result += opcode & 0x7f;
	result += (regDest & 0x3f) << 14;
	result += (regB & 0x3f) << 20;
	result += (regA & 0x3f) << 26;

	return result;
}

unsigned int assembleRiInstruction_sw(char opcode, char regDest, char regA, short imm13){
	unsigned int result = 0;
	result += opcode & 0x7f;
	result += (imm13 & 0x1fff) << 7;
	result += (regDest & 0x3f) << 20;
	result += (regA & 0x3f) << 26;

	return result;
}

unsigned int assembleMemoryInstruction_sw(char opcode, char regDest, char regA, short imm12_7, bool isSpec, char specID){
	unsigned int result = 0;
	result += opcode & 0x7f;
	result += (isSpec ? 1 : 0) << 7;
	if (!isSpec)
		result += (imm12_7 & 0xfff) << 8;
	else{
		result += (specID & 0x1f) << 8;
		result += (imm12_7 & 0x7f) << 13;
	}
	result += (regDest & 0x3f) << 20;
	result += (regA & 0x3f) << 26;

	return result;
}


unsigned int assembleRRInstruction_sw(char opcode, char regDest, char regA, char regB, char regC){
	unsigned int result = 0;
	result += opcode & 0x7f;
	result += (regDest & 0x3f) << 8;
	result += (regC & 0x3f) << 14;
	result += (regB & 0x3f) << 20;
	result += (regA & 0x3f) << 26;

	return result;
}


const char* opcodeNames[128] = {
		"NOP", "-", "-", "MPYW", "DIVW", "DIVUW", "REMW", "REMUW", "MPYH", "MPYHSU", "MPYHU", "MPY", "DIV", "DIVU", "REM", "REMU",
		"LDD", "LDW", "LDH", "LDHU", "LDB", "LDBU","LDWU", "?", "STB", "STH", "STW", "STD", "?", "?", "?", "?",
		"?", "GOTO", "IGOTO", "CALL", "ICALL", "BR", "BRF", "RETURN", "MOVI", "BLT", "RECONFFS", "BGE", "BLTU", "BGEU", "ECALL", "STOP",
		"FLB", "FLH", "FLW", "FSB", "FSH", "FSW", "FMADD", "FMSUB", "FNMSUB", "FNMADD", "FP", "?", "?", "?", "?", "?",
		"SET", "ADD", "NOR", "AND", "ANDC", "CMPLT", "CMPLTU", "CMPNE", "NOT", "OR", "SETF", "SH1ADD", "SH2ADD", "SH3ADD", "SH4ADD", "SLL",
		"SRL", "SRA", "SUB", "XOR", "ADDW", "SUBW", "SLLW", "SRLW", "SRAW", "CMPEQ", "CMPGE", "CMPGEU", "CMPGT", "CMPGTU", "CMPLE", "CMPLEU",
		"?", "ADDi", "NORi", "ANDi", "ANDCi", "CMPLTi", "CMPLTUi", "CMPNEi", "NOTi", "ORi", "ORCi", "SH1ADDi", "SH2ADDi", "SH3ADDi", "SH4ADDi", "SLLi",
		"SRLi", "SRAi", "SUBi", "XORi", "ADDWi", "?", "SLLWi", "SRLWi", "SRAWi", "CMPEQi", "CMPGEi", "CMPGEUi", "CMPGTi", "CMPGTUi", "CMPLEi", "CMPLEUi"};


const char* fpNames[32] = {
		"FADD", "FSUB","FMUL","FDIV","FSQRT","FSGNJ","FSGNJN","FSGNJNX","FMIN","FMAX",
"FCVTWS","FCVTWUS","FMVXW","FEQ","FLT","FLE","FCLASS","FCVTSW","FCVTSWU","FMVWX"};


std::string printDecodedInstr(unsigned int instruction){


	int RA = (instruction >> 26) & 0x3f;
	int RB = (instruction >> 20) & 0x3f;
	int RC = (instruction >> 14) & 0x3f;
	int IMM19 = (instruction >> 7) & 0x7ffff;
	short IMM13 = (instruction >> 7) & 0x1fff;
	short IMM13_signed = (IMM13 > 4095) ? IMM13 - 8192 : IMM13;
	short IMM12 = (instruction >> 8) & 0xfff;
	short IMM12_signed = (IMM13 > 2047) ? IMM12 - 4196 : IMM12;

	int funct = (instruction >> 7) & 0x1f;
	int OP = (instruction & 0x7f);
	int BEXT = (instruction >> 8) & 0x7;
	short IMM9 = (instruction >> 11) & 0x1ff;
	char isSpec = (instruction>>7) & 0x1;


	char isIType = (((OP >> 4) & 0x7) == 2) && (OP != VEX_BR) && (OP != VEX_BR) && (OP != VEX_BRF) && (OP != VEX_BGE) && (OP != VEX_BGEU) && (OP != VEX_BLT) && (OP != VEX_BLTU) ;
	char isImm = ((OP >> 4) & 0x7) == 1 || ((OP >> 4) & 0x7) == 6 || ((OP >> 4) & 0x7) == 7 || (((OP >> 4) & 0x7) == 2);

	std::stringstream stream;

	stream << opcodeNames[OP];
	if (OP == VEX_FP){
			stream << " " << fpNames[funct];
	}
	else if (OP>>4 == 0x1){
		//We are a memory op
		IMM13 = IMM12;
		IMM13_signed == IMM12_signed;
		if (isSpec){
			stream << " spec ";
		}
	}

	if (OP == 0){
	}
	else if (isIType)
		stream << " r" << RA << ", "  << IMM19;
	else if (isImm){
		stream << " r" << RB << "  = r" << RA << " 0x";
		stream << std::hex << IMM13_signed;
	}
	else
		stream << " r" << RC << "  = r" << RA << " r" << RB;

	std::string result(stream.str());
	for (int addedSpace = result.size(); addedSpace < 20; addedSpace++)
		result.append(" ");

	return result;
}
