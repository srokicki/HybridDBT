#include <simulator/cgraSimulator.h>

#include <iostream>

CgraSimulator::CgraSimulator()
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

void CgraSimulator::configure(uint8_t *config, uint32_t size)
{
	unsigned int i = 0;
	unsigned int num = 0;
	while (i < size*2 && num < 12)
	{
		uint8_t first_half_byte = !(i % 2) ? (config[i/2] & 0xF0) >> 4 : config[i/2] & 0xF;
		uint32_t instr = first_half_byte;

		if (first_half_byte == 0xF) // RT (Routing configuration)
		{
			++i;
			if (i % 2)
				instr += (config[i/2] & 0xF) << 4;
			else
				instr += config[i/2] & 0xF0;
			++i;
		}
		else if (first_half_byte == 0x0) // NOP
		{
			instr = 0;
			++i;
		}
		else if (first_half_byte & 0x8) // IMM USED
		{
			for (unsigned int j = 1; j < 5; ++j)
			{
				if ((i+j) % 2)
					instr += (config[(i+j)/2] & 0xF) << 4*j;
				else
					instr += ((config[(i+j)/2] & 0xF0) >> 4) << 4*j;
			}
			i += 5;
		}
		else // NO IMM USED
		{
			for (unsigned int j = 1; j < 3; ++j)
			{
				if ((i+j) % 2)
					instr += (config[(i+j)/2] & 0xF) << 4*j;
				else
					instr += ((config[(i+j)/2] & 0xF0) >> 4) << 4*j;
			}
			i += 3;
		}
		_units[num++].setInstruction(instr);
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
