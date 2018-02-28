#include <simulator/cgraSimulator.h>

#include <lib/log.h>

CgraSimulator::CgraSimulator(std::map<ac_int<64, false>, ac_int<8, true> > *memory, ac_int<64, true> *reg)
	: _memory(memory), _reg(reg)
{
	//  0  1  2  3
	//  4  5  6  7
	//  8  9 10 11

	for (unsigned int i = 0; i < height; ++i)
	{
		for (unsigned int j = 0; j < width; ++j)
		{
			unsigned int id = i*4+j;
			FunctionalUnit * f = &_units[id];
			f->enableMult();

			if (i == 0)
			{
				f->enableReg(_reg);
				if (j == 1)
					f->enableMem(memory);
			}

			if (i > 0)
				f->setNeighbour(FunctionalUnit::UP, &_units[id-4]);
			if (j > 0)
				f->setNeighbour(FunctionalUnit::LEFT, &_units[id-1]);
			if (i < 2)
				f->setNeighbour(FunctionalUnit::DOWN, &_units[id+4]);
			if (j < 3)
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

void CgraSimulator::configure(uint64_t *config)
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
	for (unsigned int j = 0; j < height; ++j)
	{
		for (unsigned int i = 0; i < width; ++i)
		{
			unsigned int id = i+j*4;
			printf("%5d | ", _units[id].read());
		}
		std::cout << std::endl;
	}
}

const FunctionalUnit *CgraSimulator::units()
{
	return _units;
}
