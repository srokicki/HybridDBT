#ifndef FUNCTIONALUNIT_H
#define FUNCTIONALUNIT_H

#include <cstdint>

#define OP 0xFF

class FunctionalUnit
{
public:

  enum DIR : int { LEFT = 1, RIGHT = 2, UP = 3, DOWN = 4 };
  FunctionalUnit();

  void run();
  void commit();

  void setNeighbour(DIR d, FunctionalUnit * u);
  void setInstruction(uint32_t instr);

  uint32_t read();
private:
  FunctionalUnit * _neighbours[5];
  uint32_t _result;
  uint32_t _out;
  uint32_t _instruction;
};

#endif // FUNCTIONALUNIT_H
