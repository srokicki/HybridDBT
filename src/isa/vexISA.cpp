#include <types.h>
#include <isa/vexISA.h>
#include <string.h>
#include <strings.h>
#include <iomanip>
#include <sstream>

#ifndef __NIOS
uint32 assembleIInstruction(uint7 opcode, uint19 imm19, uint6 regA){
	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, imm19);
	result.set_slc(26, regA);
	return result;
}

uint32 assembleRInstruction(uint7 opcode, uint6 regDest, uint6 regA, uint6 regB){
	ac_int<32, false> result = 0;
	ac_int<1, false> const0 = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, const0); //Immediate bit is equal to zero
	result.set_slc(14, regDest);
	result.set_slc(20, regB);
	result.set_slc(26, regA);
	return result;
}

uint32 assembleRiInstruction(uint7 opcode, uint6 regDest, uint6 regA, uint13 imm13){
	ac_int<32, false> result = 0;
	result.set_slc(0, opcode);
	result.set_slc(7, imm13);
	result.set_slc(20, regDest);
	result.set_slc(26, regA);
	return result;
}
#else

uint32 assembleIInstruction(uint7 opcode, uint19 imm19, uint6 regA){
	uint32 result = 0;
	result += opcode & 0x7f;
	result += (imm19 & 0x7ffff)<<7;
	result += (regA & 0x3f) << 26;
	return result;
}

uint32 assembleRInstruction(uint7 opcode, uint6 regDest, uint6 regA, uint6 regB){
	ac_int<32, false> result = 0;
	ac_int<1, false> const0 = 0;
	result += opcode & 0x7f;
	result += (regDest & 0x3f) << 14;
	result += (regB & 0x3f) << 20;
	result += (regA & 0x3f) << 26;
	return result;
}

uint32 assembleRiInstruction(uint7 opcode, uint6 regDest, uint6 regA, uint13 imm13){
	ac_int<32, false> result = 0;
	result += opcode & 0x7f;
	result += (imm13 & 0x1fff) << 7;
	result += (regDest & 0x3f) << 20;
	result += (regA & 0x3f) << 26;
	return result;
}

#endif

const char* opcodeNames[128] = {
		"NOP", "MPYLL", "MPYLLU", "MPYLH", "MPYLHU", "MPYHH", "MPYHHU", "MPYL", "DIVW", "MPYW", "MPYHU", "MPYHS", "MPYLO", "MPYHI", "DIVLO", "DIVHI",
		"LDD", "LDW", "LDH", "LDHU", "LDB", "LDBU","LDWU", "?", "STB", "STH", "STW", "STD", "?", "?", "?", "?",
		"?", "GOTO", "IGOTO", "CALL", "ICALL", "BR", "BRF", "RETURN", "MOVI", "AUIPC", "RECONFFS", "RECONFEXECUNIT", "?", "?", "?", "STOP",
		"SLCTF", "?", "?", "?", "?", "?", "?", "?", "SLCT", "?", "?", "?", "?", "?", "?", "?",
		"?", "ADD", "NOR", "AND", "ANDC", "CMPLT", "CMPLTU", "CMPNE", "NOT", "OR", "ORC", "SH1ADD", "SH2ADD", "SH3ADD", "SH4ADD", "SLL",
		"SRL", "SRA", "SUB", "XOR", "ADDW", "SUBW", "SLLW", "SRLW", "SRAW", "CMPEQ", "CMPGE", "CMPGEU", "CMPGT", "CMPGTU", "CMPLE", "CMPLEU",
		"?", "ADDi", "NORi", "ANDi", "ANDCi", "CMPLTi", "CMPLTUi", "CMPNEi", "NOTi", "ORi", "ORCi", "SH1ADDi", "SH2ADDi", "SH3ADDi", "SH4ADDi", "SLLi",
		"SRLi", "SRAi", "SUBi", "XORi", "ADDWi", "?", "SLLWi", "SRLWi", "SRAWi", "CMPEQi", "CMPGEi", "CMPGEUi", "CMPGTi", "CMPGTUi", "CMPLEi", "CMPLEUi"};


std::string printDecodedInstr(ac_int<32, false> instruction){
	ac_int<6, false> RA = instruction.slc<6>(26);
	ac_int<6, false> RB = instruction.slc<6>(20);
	ac_int<6, false> RC = instruction.slc<6>(14);
	ac_int<19, true> IMM19 = instruction.slc<19>(7);
	ac_int<13, false> IMM13 = instruction.slc<13>(7);
	ac_int<13, true> IMM13_signed = instruction.slc<13>(7);

	ac_int<7, false> OP = instruction.slc<7>(0);
	ac_int<3, false> BEXT = instruction.slc<3>(8);
	ac_int<9, false> IMM9 = instruction.slc<9>(11);

	ac_int<1, false> isIType = (OP.slc<3>(4) == 2);
	ac_int<1, false> isImm = OP.slc<3>(4) == 1 || OP.slc<3>(4) == 6 || OP.slc<3>(4) == 7;

	std::stringstream stream;

	stream << opcodeNames[OP];
	if (OP == 0){
	}
	else if (isIType)
		stream << " r" + std::to_string(RA) + ", " + std::to_string(IMM19);
	else if (isImm){
		stream << " r" + std::to_string(RB) + "  = " + std::to_string(RA) + " 0x";
		stream << std::hex << IMM13_signed;
	}
	else
		stream << " r" + std::to_string(RC) + "  = " + std::to_string(RA) + " " + std::to_string(RB);

	std::string result(stream.str());
	for (int addedSpace = result.size(); addedSpace < 20; addedSpace++)
		result.append(" ");

	return result;
}
