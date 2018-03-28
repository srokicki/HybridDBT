#include <simulator/functionalUnit.h>
#include <isa/cgraIsa.h>
#include <lib/log.h>
#include <simulator/cgraSimulator.h>

FunctionalUnit::DIR reverse(FunctionalUnit::DIR d)
{
	switch (d)
	{
	case FunctionalUnit::LEFT: return FunctionalUnit::RIGHT;
	case FunctionalUnit::RIGHT: return FunctionalUnit::LEFT;
	case FunctionalUnit::UP: return FunctionalUnit::DOWN;
	case FunctionalUnit::DOWN: return FunctionalUnit::UP;
	default: exit(-1);
	}
}

FunctionalUnit::FunctionalUnit()
	: _memory(nullptr), _reg(nullptr), _features(0), _out(0), _result(0), _reg_write(false), _sim(nullptr)
{
	_neighbours[0] = this;
	for (unsigned int i = 1; i < 5; ++i)
		_neighbours[i] = nullptr;
}

void FunctionalUnit::run()
{
	uint32_t imm_short, imm_long, addr;
	int32_t imm_short_signed;
	int64_t va, vb, vc;
	uint8_t opcode, isImm, ra, rb, rc;

	ra = cgra::regA(_instruction);
	rb = cgra::regB(_instruction);
	rc = cgra::regC(_instruction);

	imm_short = cgra::immShort(_instruction);
	imm_short_signed = (imm_short >= CGRA_IMM_SHORT_MAX) ? imm_short - (CGRA_IMM_SHORT_MAX << 1): imm_short;
	imm_long = cgra::immLong(_instruction);

	opcode = cgra::opcode(_instruction);

	isImm = (((opcode >> 4) & 0x7) == 2);

	_reg_write = false;

	if (opcode == VEX_NOP)
		return;

	if (ra > 63)
	{
		if (_neighbours[ra-64])
		{
			va = _neighbours[ra-64]->_out;
		}
		else
		{
			Log::out(2) << "FunctionalUnit::doStep() " << opcodeNames[opcode] << " wants to access unknown neighbour\n";
			return;
		}
	}
	else if (_reg)
	{
		va = _reg[ra];
	}
	else
	{
		Log::out(2) << "FunctionalUnit::doStep() " << opcodeNames[opcode] << " wants to access register file but can't\n";
		return;
	}

	if (!isImm && opcode != CGRA_CARRY)
	{
		if (rc > 63 && _neighbours[rc-64])
			vc = _neighbours[rc-64]->_out;
		else if (_reg)
		{
			vc = _reg[rc];
		}
		else
		{
			Log::out(2) << "FunctionalUnit::doStep() " << opcodeNames[opcode] << " wants to access register file but can't\n";
			return;
		}
	}

	if (opcode == VEX_STB || opcode == VEX_STH || opcode == VEX_STW || opcode == VEX_STD || opcode == CGRA_RECONF_IFEQ || opcode == CGRA_RECONF_IFNE)
	{
		if (rb > 63 && _neighbours[rb-64])
			vb = _neighbours[rb-64]->_out;
		else if (_reg)
		{
			vb = _reg[rb];
		}
		else
		{
			Log::out(2) << "FunctionalUnit::doStep() " << opcodeNames[opcode] << " wants to access register file but can't\n";
			return;
		}
	}

	addr = imm_short_signed + va;
	switch (opcode)
	{
	case VEX_NOP:
		break;

	// ARITHMETIC OPERATIONS
	case VEX_ADD:
	case VEX_ADDW:
		_result = va + vc;
		_reg_write = true;
		break;
	case VEX_ADDi:
	case VEX_ADDWi:
		_result = va + imm_short;
		_reg_write = true;
		break;
	case VEX_SUB:
	case VEX_SUBW:
		_result = va- vc;
		_reg_write = true;
		break;
	case VEX_SUBi:
		_result = va- imm_short;
		_reg_write = true;
		break;
	case VEX_MPY:
	case VEX_MPYH:
	case VEX_MPYW:
		if (this->_features & FEATURE_MULT)
		{
			_result = va * vc;
		}
		_reg_write = true;
		break;
	case VEX_SLL:
		_result = va << vc;
		_reg_write = true;
		break;
	case VEX_SLLi:
		_result = va << imm_short;
		_reg_write = true;
		break;
	case VEX_SRL:
		_result = (uint64_t)(va) >> vc;
		_reg_write = true;
		break;
	case VEX_SRLi:
		_result = (uint64_t)(va) >> imm_short;
		_reg_write = true;
		break;
	case VEX_SRLW:
		_result = (uint32_t)(va & 0xffffffff) >> vc;
		_reg_write = true;
		break;
	case VEX_SRLWi:
		_result = (uint32_t)(va & 0xffffffff) >> imm_short;
		_reg_write = true;
		break;
	case VEX_SRA:
		_result = va >> vc;
		_reg_write = true;
		break;
	case VEX_SRAW:
		_result = (int32_t)(va) >> vc;
		_reg_write = true;
		break;
	case VEX_SRAi:
		_result = va >> imm_short;
		_reg_write = true;
		break;
	case VEX_SRAWi:
		_result = (int32_t)(va) >> imm_short;
		_reg_write = true;
		break;
	case VEX_OR:
		_result = va | vc;
		_reg_write = true;
		break;
	case VEX_ORi:
		_result = va | imm_short;
		_reg_write = true;
		break;
	case VEX_AND:
		_result = va & vc;
		_reg_write = true;
		break;
	case VEX_ANDi:
		_result = va & imm_short;
		_reg_write = true;
		break;
	case VEX_XOR:
		_result = va^ vc;
		_reg_write = true;
		break;
	case VEX_XORi:
		_result = va^ imm_short;
		_reg_write = true;
		break;

	// CGRA OPERATIONS
	case CGRA_CARRY:
		_result = va;
		break;
	case CGRA_RECONF_IFEQ:
		if (va == vb)
		{
			_sim->_cgra_cycles = imm_short-1;
		}
		break;
	case CGRA_RECONF_IFNE:
		if (va != vb)
		{
			_sim->_cgra_cycles = imm_short-1;
		}
		break;
	default:
		break;
	}

	if (this->_features & FEATURE_MEM)
	{
		switch (opcode)
		{
		// MEMORY OPERATIONS
		case VEX_LDD:
			Log::out(2) << "FunctionalUnit::run() VEX_LDD not implemented yet.\n";
			exit(-1);
			//_result = (*_memory)[addr] + ((*_memory)[addr+1] << 8) + ((*_memory)[addr+2] << 16) + ((*_memory)[addr+3] << 24);
			//_reg_write = true;
			break;
		case VEX_LDW:
			_result = ((*_memory)[addr] & 0xff) + (((*_memory)[addr+1] << 8) & 0xff00) + (((*_memory)[addr+2] << 16) & 0xff0000) + (((*_memory)[addr+3] << 24) & 0xff000000);
			_reg_write = true;
			Log::out(2) << "FU: LDW " << _result << "\n";
			break;
		case VEX_LDH:
			_result = (*_memory)[addr] + ((*_memory)[addr+1] << 8);
			_reg_write = true;
			break;
		case VEX_LDB:
			_result = (*_memory)[addr];
			_reg_write = true;
			break;

		case VEX_STD:
			Log::out(2) << "FunctionalUnit::run() VEX_STD not implemented yet.\n";
			exit(-1);
			break;
		case VEX_STW:
			Log::fprintf(2, stdout, "FU: Writing byte %d at: %x\n", vb, addr);
			(*_memory)[addr+3] = (vb >> 24) & 0xff;
			(*_memory)[addr+2] = (vb >> 16) & 0xff;
		case VEX_STH:
			(*_memory)[addr+1] = (vb >>  8) & 0xff;
		case VEX_STB:
			(*_memory)[addr+0] = (vb >>  0) & 0xff;
			break;

		default:
			break;
		}
	}
}

void FunctionalUnit::commit()
{
	_out = _result;
	if (_reg_write)
	{
		uint8_t rb = cgra::regB(_instruction);
		if (rb < 64 && _reg)
		{
			_reg[rb] = _result;
		}
	}
}

void FunctionalUnit::setNeighbour(FunctionalUnit::DIR d, FunctionalUnit *u)
{
	_neighbours[d] = u;
}

void FunctionalUnit::setInstruction(uint64_t instr)
{
	_instruction = instr;
}

void FunctionalUnit::enableMult()
{
	_features |= FEATURE_MULT;
}

uint8_t FunctionalUnit::features() const
{
	return _features;
}

void FunctionalUnit::enableReg(ac_int<64, true> *reg)
{
	_features |= FEATURE_REG;
	_reg = reg;
}

void FunctionalUnit::enableReconf(CgraSimulator *sim)
{
	_features |= FEATURE_RECONF;
	_sim = sim;
}

void FunctionalUnit::enableMem(std::map<ac_int<64, false>, ac_int<8, true> > *memory)
{
	_features |= FEATURE_MEM;
	_memory = memory;
}

uint32_t FunctionalUnit::read()
{
	return _out;
}
