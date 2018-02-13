#include <simulator/functionalUnit.h>
#include <isa/cgraIsa.h>

FunctionalUnit::DIR reverse(FunctionalUnit::DIR d)
{
	switch (d)
	{
	case FunctionalUnit::LEFT: return FunctionalUnit::RIGHT;
	case FunctionalUnit::RIGHT: return FunctionalUnit::LEFT;
	case FunctionalUnit::UP: return FunctionalUnit::DOWN;
	case FunctionalUnit::DOWN: return FunctionalUnit::UP;
	default: return FunctionalUnit::LEFT;
	}
}

FunctionalUnit::FunctionalUnit(uint8_t *memory)
	: _memory(memory)
{
	_neighbours[0] = this;
	for (unsigned int i = 1; i < 5; ++i)
		_neighbours[i] = nullptr;

	_out = _result = 0;
}

void FunctionalUnit::run()
{
	uint32_t imm13, imm19, addr;
	int32_t imm13_s;
	FunctionalUnit * op1 = nullptr, * op2 = nullptr;
	uint8_t opcode, isImm, ra, rb, rc;

	ra = (_instruction >> 26) & 0x3f;
	rb = (_instruction >> 20) & 0x3f;
	rc = (_instruction >> 14) & 0x3f;
	imm13 = (_instruction >> 7) & 0x1fff;
	imm13_s = (imm13 > 4096) ? imm13 - 8192 : imm13;
	imm19 = (_instruction >> 7) & 0x7ffff;
	opcode = (_instruction >> 0) & 0x7f;
	isImm = (((opcode >> 4) & 0x7) == 2);

	addr = imm13_s + ra;

	op1 = _neighbours[ra];
	if (!isImm)
		op2 = _neighbours[rb];

	switch (opcode)
	{
	case VEX_NOP:
		break;

	// ARITHMETIC OPERATIONS
	case VEX_ADD:
		_result = op1->_out + op2->_out;
		break;
	case VEX_ADDi:
		_result = op1->_out + imm13;
		break;
	case VEX_SUB:
		_result = op1->_out - op2->_out;
		break;
	case VEX_SUBi:
		_result = op1->_out - imm13;
		break;
	case VEX_MPY:
		_result = op1->_out * op2->_out;
		break;
	case VEX_SLL:
		_result = op1->_out << op2->_out;
		break;
	case VEX_SLLi:
		_result = op1->_out << imm13;
		break;
	case VEX_SRL:
		_result = op1->_out >> op2->_out;
		break;
	case VEX_SRLi:
		_result = op1->_out >> imm13;
		break;
	case VEX_OR:
		_result = op1->_out | op2->_out;
		break;
	case VEX_ORi:
		_result = op1->_out | imm13;
		break;
	case VEX_AND:
		_result = op1->_out & op2->_out;
		break;
	case VEX_ANDi:
		_result = op1->_out & imm13;
		break;
	case VEX_XOR:
		_result = op1->_out ^ op2->_out;
		break;
	case VEX_XORi:
		_result = op1->_out ^ imm13;
		break;

	// MEMORY OPERATIONS
	case VEX_LDD:
		_result = _memory[addr] + (_memory[addr+1] << 8) + (_memory[addr+2] << 16) + (_memory[addr+3] << 24);
		break;
	case VEX_LDW:
		_result = _memory[addr] + (_memory[addr+1] << 8) + (_memory[addr+2] << 16);
		break;
	case VEX_LDH:
		_result = _memory[addr] + (_memory[addr+1] << 8);
		break;
	case VEX_LDB:
		_result = _memory[addr];
		break;

	case VEX_STD:
		_memory[addr+3] = (_result >> 24) & 0xff;
	case VEX_STW:
		_memory[addr+2] = (_result >> 16) & 0xff;
	case VEX_STH:
		_memory[addr+1] = (_result >>  8) & 0xff;
	case VEX_STB:
		_memory[addr+0] = (_result >>  0) & 0xff;
		break;

	// CGRA OPERATIONS
	case 0xFF:
		_result = op1->_out;
		break;
	default:
		break;
	}
}

void FunctionalUnit::commit()
{
	_out = _result;
}

void FunctionalUnit::setNeighbour(FunctionalUnit::DIR d, FunctionalUnit *u)
{
	_neighbours[d] = u;
}

void FunctionalUnit::setInstruction(uint32_t instr)
{
	_instruction = instr;
}

void FunctionalUnit::setMemory(uint8_t *memory)
{
	_memory = memory;
}

uint32_t FunctionalUnit::read()
{
	return _out;
}
