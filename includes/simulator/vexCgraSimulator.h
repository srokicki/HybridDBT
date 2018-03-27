#ifndef VEXCGRASIMULATOR_H
#define VEXCGRASIMULATOR_H

#include <simulator/cgraSimulator.h>
#include <simulator/vexSimulator.h>
#include <map>


class VexCgraSimulator : public VexSimulator
{
public:

	int cgraStall;
	CgraSimulator cgraSimulator;
	bool cgraRunning;
	VexCgraSimulator(unsigned int * bin);

	virtual int doStep();
	virtual void doDC(FtoDC ftoDC, DCtoEx *dctoEx);
	virtual void doDCBr(FtoDC ftoDC, DCtoEx *dctoEx);
};

#endif // VEXCGRASIMULATOR_H
