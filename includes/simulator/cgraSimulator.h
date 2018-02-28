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
class CgraSimulator
{
public:
	CgraSimulator(std::map<ac_int<64, false>, ac_int<8, true>> *memory, ac_int<64, true> * reg);

	/**
	 * @brief doStep executes all CGRA's FunctionalUnits
	 */
	void doStep();

	/**
	 * @brief configures the CGRA's FunctionalUnits
	 * @param config: a bitstream formated configuration
	 * @param size: the size of the bitstream in bytes
	 */
	void configure(uint64_t *config);

	/**
	 * @brief print prints the CGRA's FunctionalUnits _out register
	 */
	void print();

	/**
	 * @brief units access the CGRA's FunctionalUnits
	 * @return the units buffer
	 */
	const FunctionalUnit * units();
private:

	static constexpr unsigned int height = 3;
	static constexpr unsigned int width = 4;

	FunctionalUnit _units[12];
	std::map<ac_int<64, false>, ac_int<8, true>> * _memory;
	ac_int<64, true> * _reg;
};

#endif // CGRASIMULATOR_H
