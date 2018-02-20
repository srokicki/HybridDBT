#ifndef VEXCGRASIMULATOR_H
#define VEXCGRASIMULATOR_H

#include <simulator/cgraSimulator.h>
#include <simulator/vexSimulator.h>
#include <map>

typedef struct
{
	uint64_t * configuration;
	int cycles;
} configuration_t;

class VexCgraSimulator : public VexSimulator
{
public:

	std::map<ac_int<19, false>, configuration_t> configurationCache;
	int cgraCycles;
	int currentConf;
	CgraSimulator cgraSimulator;
	VexCgraSimulator(unsigned int * bin);

	virtual int doStep();
	virtual void doDC(FtoDC ftoDC, DCtoEx *dctoEx);
	virtual void doDCBr(FtoDC ftoDC, DCtoEx *dctoEx);
};

#endif // VEXCGRASIMULATOR_H
