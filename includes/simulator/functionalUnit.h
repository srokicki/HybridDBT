#ifndef FUNCTIONALUNIT_H
#define FUNCTIONALUNIT_H

#include <cstdint>

/**
 * @brief The FunctionalUnit class represent a CGRA's FU
 *
 * It is designed for a Grid CGRA, as this unit can have 4 neighbours
 * The neighbour number 0 is the unit itself. It is used to encode
 * inputs:
 *
 * 0 = internal register
 * 1 = left neighbour
 * 2 = right
 * 3 = up
 * 4 = down
 *
 */
class FunctionalUnit
{
public:

	enum DIR : int { LEFT = 1, RIGHT = 2, UP = 3, DOWN = 4 };

	FunctionalUnit(uint8_t * memory = nullptr);

	/**
	 * @brief run performs an instruction, and stores its result in _result variable.
	 */
	void run();

	/**
	 * @brief commit stores _result in _out. It is used to simulate parallel update,
	 * as updating the _out variable of a unit at the middle of an update can lead to
	 * false results.
	 */
	void commit();

	/**
	 * @brief setNeighbour uses the enum DIR to ensure that the user doesn't use a wrong value
	 * @param d: the relative direction of the neighbour
	 * @param u: the neighbour's pointer
	 */
	void setNeighbour(DIR d, FunctionalUnit * u);

	void setInstruction(uint32_t instr);

	void setMemory(uint8_t * memory);

	uint32_t read();
private:
	FunctionalUnit * _neighbours[5];
	uint32_t _result;
	uint32_t _out;
	uint32_t _instruction;

	uint8_t * _memory;
};

#endif // FUNCTIONALUNIT_H
