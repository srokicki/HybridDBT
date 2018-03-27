#include <simulator/cgraSimulator.h>

#include <lib/log.h>

CgraSimulator::CgraSimulator(std::map<ac_int<64, false>, ac_int<8, true> > *memory, ac_int<64, true> *reg)
	: _memory(memory), _reg(reg), _current_conf(-1)
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

			f->enableReconf(this);

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

bool CgraSimulator::doStep()
{
	if (_current_conf == -1)
		return false;

	configure(_configuration_cache[_current_conf].configuration + CgraSimulator::height*CgraSimulator::width*_cgra_cycles);

//	int id = 0;
	for (FunctionalUnit& u : _units)
	{
//		Log::out(0) << "RUN (" << (id/4) << "," << (id%4) << ")\n";
//		++id;
		u.run();
	}

	for (FunctionalUnit& u : _units)
		u.commit();

	_cgra_cycles++;

	if (_cgra_cycles == _configuration_cache[_current_conf].cycles+1)
		_current_conf = -1;

	return true;
}

void CgraSimulator::start(int config)
{
	if (_configuration_cache.find(config) == _configuration_cache.end())
		return;

	_current_conf = config;
	_cgra_cycles = 0;
}

void CgraSimulator::configure(uint64_t *config)
{
	unsigned int num = 0;
	while (num < CgraSimulator::height*CgraSimulator::width)
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
			printf("%10d | ", _units[id].read());
		}
		std::cout << std::endl;
	}
}

int CgraSimulator::registerConfiguration(uint64_t *configuration, int cycles)
{
	configuration_t c = { configuration, cycles };
	int id = _configuration_cache.size();
	_configuration_cache[_configuration_cache.size()] = c;
	return id;
}

const FunctionalUnit *CgraSimulator::units()
{
	return _units;
}
