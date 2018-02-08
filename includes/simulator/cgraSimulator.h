#ifndef CGRASIMULATOR_H
#define CGRASIMULATOR_H

#include <simulator/genericSimulator.h>
#include <simulator/functionalUnit.h>

class CgraSimulator : public GenericSimulator
{
public:
  CgraSimulator();

  void doStep();
  void configure(uint8_t * config, uint32_t size);

  void print();
private:
  FunctionalUnit _units[12];
  uint8_t * _config;
};

#endif // CGRASIMULATOR_H
