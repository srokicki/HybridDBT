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

FunctionalUnit::FunctionalUnit()
{
  _neighbours[0] = this;
  for (unsigned int i = 1; i < 5; ++i)
    _neighbours[i] = nullptr;

  _out = _result = 0;
}

void FunctionalUnit::run()
{
  uint32_t imm13, imm16;
  FunctionalUnit * op1, * op2;

  op1 = _neighbours[(_instruction & OP1) >> OP1_OFF];
  op2 = _neighbours[(_instruction & OP2) >> OP2_OFF];
  imm13 = (_instruction & IMM13) >> IMM13_OFF;
  imm16 = (_instruction & IMM16) >> IMM16_OFF;

  switch (_instruction & 0xF)
  {
  case CGRA_ADD:
    _result = op1->_out + op2->_out;
    break;
  case CGRA_SUB:
    _result = op1->_out - op2->_out;
    break;
  case CGRA_MUL:
    _result = op1->_out * op2->_out;
    break;
  case CGRA_DIV:
    _result = op1->_out / op2->_out;
    break;
  case CGRA_ADDi:
    _result = op1->_out + imm13;
    break;
  case CGRA_SUBi:
    _result = op1->_out - imm13;
    break;
  case CGRA_LDi:
    _result = imm16;
    break;
  case CGRA_RT:
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

uint32_t FunctionalUnit::read()
{
  return _out;
}
