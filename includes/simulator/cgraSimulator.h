#ifndef CGRASIMULATOR_H
#define CGRASIMULATOR_H

#include <simulator/genericSimulator.h>
#include <simulator/functionalUnit.h>


/**
 * @brief The CgraSimulator class represents a 4*3 Grid CGRA
 *
 * the units indices are as follow:
 *
 *  0  1  2  3
 *  4  5  6  7
 *  8  9 10 11
 */
class CgraSimulator : public GenericSimulator
{
public:
  CgraSimulator();

  /**
   * @brief doStep executes all CGRA's FunctionalUnits
   */
  void doStep();

  /**
   * @brief configures the CGRA's FunctionalUnits
   * @param config: a bitstream formated configuration
   * @param size: the size of the bitstream in bytes
   */
  void configure(uint8_t * config, uint32_t size);

  /**
   * @brief print prints the CGRA's FunctionalUnits _out register
   */
  void print();
private:
  FunctionalUnit _units[12];
  uint8_t * _config;
};

#endif // CGRASIMULATOR_H
