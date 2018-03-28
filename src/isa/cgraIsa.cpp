#include <isa/cgraIsa.h>
#include <algorithm>
#include <sstream>
#include <lib/log.h>
#include <simulator/cgraSimulator.h>
#include <iomanip>

namespace cgra
{

uint8_t regA(const uint64_t& instruction)
{
	return (instruction >> CGRA_REG1_OFFSET) & CGRA_REG_MASK;
}

uint8_t regB(const uint64_t& instruction)
{
	return (instruction >> CGRA_REG2_OFFSET) & CGRA_REG_MASK;
}

uint8_t regC(const uint64_t& instruction)
{
	return (instruction >> CGRA_REG3_OFFSET) & CGRA_REG_MASK;
}

uint16_t immShort(const uint64_t& instruction)
{
	return (instruction >> CGRA_IMM_OFFSET) & CGRA_IMM_SHORT_MASK;
}

uint32_t immLong(const uint64_t& instruction)
{
	return (instruction >> CGRA_IMM_OFFSET) & CGRA_IMM_LONG_MASK;
}

uint8_t opcode(const uint64_t& instruction)
{
	return instruction & CGRA_OPCODE_MASK;
}

uint64_t vex2cgra(uint128_struct instruction, uint8_t src1, uint8_t src2, uint16_t * read1, uint16_t * read2)
{
	uint64_t ret = (instruction.word96 >> 19) & CGRA_OPCODE_MASK;
	uint16_t imm_short = instruction.word96 & CGRA_IMM_SHORT_MASK;

	uint32_t imm_long = (instruction.word64 >> 23) & 0x1ff;
	imm_long += (instruction.word96 & 0x3ff) << 9;

	bool isImm = (instruction.word96 >> 18) & 0x1;

	uint16_t virtualRDest = ((instruction.word64>>14) & 0x1ff);
	uint16_t virtualRIn2 = ((instruction.word64>>23) & 0x1ff);
	uint16_t virtualRIn1_imm9 = ((instruction.word96>>0) & 0x1ff);
	uint16_t r1 = 0xffff, r2 = 0xffff, r3 = 0xffff;
	uint8_t opCode = ((instruction.word96>>19) & 0x7f);

	if (opCode == VEX_STB || opCode == VEX_STD || opCode == VEX_STH || opCode == VEX_STW || (opCode == VEX_BR || opCode == VEX_BRF))
	{
		r1 = virtualRIn2;
		r2 = virtualRDest;

		if (opCode == VEX_BR) ret = CGRA_RECONF_IFEQ;
		if (opCode == VEX_BRF) ret = CGRA_RECONF_IFNE;

		if (src1 != 0xff)
		{
			ret += (src1 & CGRA_REG_MASK) << CGRA_REG1_OFFSET;
		}
		else
		{
			ret += (r1 & CGRA_REG_MASK) << CGRA_REG1_OFFSET;
			if (read1)
				*read1 = r1;
		}

		if (src2 != 0xff)
		{
			ret += (src2 & CGRA_REG_MASK) << CGRA_REG2_OFFSET;
		}
		else
		{
			ret += (r2 & CGRA_REG_MASK) << CGRA_REG2_OFFSET;
			if (read2)
				*read2 = r2;
		}

		ret += imm_short << CGRA_IMM_OFFSET;
	}
	else
	{
		r1 = virtualRIn2;
		r2 = virtualRIn1_imm9;
		r3 = virtualRDest;

		if (src1 != 0xff)
		{
			ret += (src1 & CGRA_REG_MASK) << CGRA_REG1_OFFSET;
		}
		else
		{
			ret += (r1 & CGRA_REG_MASK) << CGRA_REG1_OFFSET;
			if (read1)
				*read1 = r1;
		}

		if (!isImm)
		{
			if (src2 != 0xff)
			{
				ret += (src2 & CGRA_REG_MASK) << CGRA_REG3_OFFSET;
			}
			else
			{
				ret += (r2 & CGRA_REG_MASK) << CGRA_REG3_OFFSET;
				if (read2)
					*read2 = r2;
			}
		}
		else
		{
			ret += imm_short << CGRA_IMM_OFFSET;
		}

		if (r3 >= 256)
			ret += (r3 & CGRA_REG_MASK) << CGRA_REG2_OFFSET;
	}
	return ret;
}

bool isEligible(uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4)
{
	uint8_t opCode = ((i1>>19) & 0x7f);
	static const uint8_t validOpCodes[] = {
		VEX_ADD, VEX_ADDi,
		VEX_MPY, VEX_MPYW, VEX_MPYH, VEX_LDD, VEX_STD, VEX_STB, VEX_STH, VEX_STW, VEX_LDW, VEX_LDB, VEX_LDH,
		VEX_SUB, VEX_SUBi, VEX_SLL, VEX_SLLi,
		VEX_AND, VEX_ANDi, VEX_XOR, VEX_XORi,
		VEX_OR, VEX_ORi, VEX_SRL, VEX_SRLi,
		VEX_ADDW, VEX_ADDWi, VEX_BR, VEX_BRF, VEX_CMPNE, VEX_SUBW,
		VEX_SRAW, VEX_SRAWi, VEX_SRA, VEX_SRAi, VEX_SRLW, VEX_SRLWi
	};
	return std::find(validOpCodes, validOpCodes+sizeof(validOpCodes), opCode)
			!= validOpCodes+sizeof(validOpCodes);
}

std::string toString(uint64_t instruction)
{
	std::stringstream ss;
	uint8_t op = opcode(instruction);
	auto ra = regA(instruction), rb = regB(instruction), rc = regC(instruction);

	uint16_t imm_short = cgra::immShort(instruction);
	uint16_t imm_short_signed = (imm_short >= CGRA_IMM_SHORT_MAX) ? imm_short - (CGRA_IMM_SHORT_MAX << 1): imm_short;

	char isImm = ((op >> 4) & 0x7) == 1 || ((op >> 4) & 0x7) == 6 || ((op >> 4) & 0x7) == 7;

	ss << (op == CGRA_CARRY ? "CARRY" : (op == CGRA_RECONF_IFEQ || op == CGRA_RECONF_IFNE ? "RECONF" : opcodeNames[op]));

	if (op == 0)
	{
	}
	else if (op == CGRA_CARRY)
	{
		ss << " from n" << (int)(ra-64);
	}
	else if (op == CGRA_RECONF_IFEQ)
	{
		if (ra > 63)
			ss << " n" << (int)(ra-64);
		else
			ss << " r" << (int)(ra);

		ss << "==";

		if (rb > 63)
			ss << "n" << (int)(rb-64);
		else
			ss << "r" << (int)(rb);
		ss << " PC=" << imm_short;
	}
	else if (op == CGRA_RECONF_IFNE)
	{
		if (ra > 63)
			ss << " n" << (int)(ra-64);
		else
			ss << " r" << (int)(ra);

		ss << "!=";

		if (rb > 63)
			ss << "n" << (int)(rb-64);
		else
			ss << "r" << (int)(rb);
		ss << " PC=" << imm_short;
	}
	else if (op == VEX_STB || op == VEX_STD || op == VEX_STH || op == VEX_STW)
	{
		ss << " *(";
		if (ra > 63)
			ss << "n" << (int)(ra-64);
		else
			ss << "r" << (int)ra;

		ss << "+" << imm_short_signed << ") = ";
		if (rb > 63)
			ss << "n" << (int)(rb-64);
		else
			ss << "r" << (int)rb;
	}
	else
	{
		if (rb < 64)
			ss << " r" << (int)rb;
		else
			ss << " rem";

		ss << " = ";

		if (ra > 63)
			ss << "n" << (int)(ra-64);
		else
			ss << "r" << (int)ra;

		ss << " § ";

		if (isImm)
		{
			ss << immShort(instruction);
		}
		else
		{
			if (rc > 63)
				ss << "n" << (int)(rc-64);
			else
				ss << "r" << (int)rc;
		}
	}

	std::string s = ss.str();
	ss.str("");
	ss.clear();
	ss << std::setw(25) << s;
	return ss.str();
}

void printConfig(int verbose, const uint64_t * configuration)
{
	for (int i = 0; i < CgraSimulator::height; ++i)
	{
		for (int j = 0; j < CgraSimulator::width; ++j)
		{
			Log::out(verbose) << cgra::toString(configuration[i*4+j]);
		}
		Log::fprintf(verbose, stdout, "\n");
	}
}

uint8_t direction(const cgra_node &from, const cgra_node &to)
{
	int8_t dv = from.i - to.i;
	int8_t dh = from.j - to.j;

	if (dh < 0)
		return 1;
	if (dh > 0)
		return 2;
	if (dv < 0)
		return 3;
	if (dv > 0)
		return 4;
	return 0;
}

cgra_node operator+(const cgra_node &l, const cgra_node &r)
{
	return { l.i+r.i, l.j+r.j, l.k+r.k };
}

bool operator==(const cgra_node &l, const cgra_node &r)
{
	return l.i == r.i && l.j == r.j && l.k == r.k;
}

bool operator!=(const cgra_node &l, const cgra_node &r)
{
	return !(l==r);
}

} // namespace cgra
