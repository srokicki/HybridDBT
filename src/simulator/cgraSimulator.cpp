#include <simulator/cgraSimulator.h>

#include <iostream>

CgraSimulator::CgraSimulator(uint8_t * memory)
	: _memory(memory)
{
	//  0  1  2  3
	//  4  5  6  7
	//  8  9 10 11

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			unsigned int id = i+j*4;
			FunctionalUnit * f = &_units[id];
			f->setMemory(memory);
			if (j > 0)
				f->setNeighbour(FunctionalUnit::UP, &_units[id-4]);
			if (i > 0)
				f->setNeighbour(FunctionalUnit::LEFT, &_units[id-1]);
			if (j < 2)
				f->setNeighbour(FunctionalUnit::DOWN, &_units[id+4]);
			if (i < 4)
				f->setNeighbour(FunctionalUnit::RIGHT, &_units[id+1]);
		}
	}
}

void CgraSimulator::doStep()
{
	for (FunctionalUnit& u : _units)
		u.run();

	for (FunctionalUnit& u : _units)
		u.commit();
}

void CgraSimulator::configure(uint32_t *config, uint32_t size)
{
	unsigned int num = 0;
	while (num < 12)
	{
		_units[num].setInstruction(config[num]);
		++num;
	}
}

void CgraSimulator::print()
{
	for (unsigned int j = 0; j < 3; ++j)
	{
		for (unsigned int i = 0; i < 4; ++i)
		{
			unsigned int id = i+j*4;
			printf("%5d | ", _units[id].read());
		}
		std::cout << std::endl;
	}
}
