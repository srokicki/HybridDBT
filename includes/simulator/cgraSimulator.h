#ifndef CGRASIMULATOR_H
#define CGRASIMULATOR_H

#include <simulator/genericSimulator.h>
#include <simulator/functionalUnit.h>
#include <map>


typedef struct
{
	uint64_t * configuration;
	int cycles;
} configuration_t;

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
	 * @return true if doStep() has performed the very last step of the current configuration
	 */
	bool doStep();

	/**
	 * @brief starts the CGRA to execute a configuration
	 * @param config: the configuration ID
	 */
	void start(int config);

	/**
	 * @brief print prints the CGRA's FunctionalUnits _out registers
	 */
	void print();

	/**
	 * @brief registerConfiguration registers a CGRA configuration into the simulator
	 * @param configuration is the bitstream of the configuration
	 * @param cycles is the address of the last configuration layer
	 * @return the ID of the pushed configuration
	 */
	int registerConfiguration(uint64_t * configuration, int cycles);

	/**
	 * @brief units access the CGRA's FunctionalUnits
	 * @return the units buffer
	 */
	const FunctionalUnit * units();

	static constexpr unsigned int height = 2;
	static constexpr unsigned int width = 4;

private:

	FunctionalUnit _units[height*width];
	std::map<ac_int<64, false>, ac_int<8, true>> * _memory;
	ac_int<64, true> * _reg;
	std::map<ac_int<19, false>, configuration_t> _configuration_cache;
	int _cgra_cycles;
	int _current_conf;
	void configure(uint64_t *config);

	friend class FunctionalUnit;
};

#endif // CGRASIMULATOR_H
