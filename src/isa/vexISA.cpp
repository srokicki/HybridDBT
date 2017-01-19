#include <lib/ac_int.h>
#include <isa/vexISA.h>
#include <string.h>
#include <strings.h>
#include <iomanip>
#include <sstream>

ac_int<32, false> assembleIInstruction(ac_int<7, false> opcode, ac_int<19, true> imm19, ac_int<6, false> regA){
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


const char* opcodeNames[128] = {
		"NOP", "MPYLL", "MPYLLU", "MPYLH", "MPYLHU", "MPYHH", "MPYHHU", "MPYL", "MPYLU", "MPYH", "MPYHU", "MPYHS", "MPYLO", "MPYHI", "DIVLO", "DIVHI",
		"?", "LDW", "LDH", "LDHU", "LDB", "LDBU", "STW", "STH", "STB", "?", "?", "?", "?", "?", "?", "?",
		"?", "GOTO", "IGOTO", "CALL", "ICALL", "BR", "BRF", "RETURN", "MOVI", "AUIPC", "?", "?", "?", "?", "?", "STOP",
		"SLCTF", "?", "?", "?", "?", "?", "?", "?", "SLCT", "?", "?", "?", "?", "?", "?", "?",
		"?", "ADD", "NOR", "AND", "ANDC", "CMPLT", "CMPLTU", "CMPNE", "NOT", "OR", "ORC", "SH1ADD", "SH2ADD", "SH3ADD", "SH4ADD", "SLL",
		"SRL", "SRA", "SUB", "XOR", "SXTH", "ZXTB", "ZXTH", "SXTB", "?", "CMPEQ", "CMPGE", "CMPGEU", "CMPGT", "CMPGTU", "CMPLE", "CMPLEU",
		"?", "ADDi", "NORi", "ANDi", "ANDCi", "CMPLTi", "CMPLTUi", "CMPNEi", "NOTi", "ORi", "ORCi", "SH1ADDi", "SH2ADDi", "SH3ADDi", "SH4ADDi", "SLLi",
		"SRLi", "SRAi", "SUBi", "XORi", "SXTHi", "ZXTBi", "ZXTHi", "SXTBi", "?", "CMPEQi", "CMPGEi", "CMPGEUi", "CMPGTi", "CMPGTUi", "CMPLEi", "CMPLEUi"};


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
