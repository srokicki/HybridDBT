#ifndef CGRASCHEDULER_H
#define CGRASCHEDULER_H

#include <simulator/vexCgraSimulator.h>
#include <isa/irISA.h>
#include <cstdint>

class CgraScheduler
{
public:
	virtual bool schedule(VexCgraSimulator &cgra, uint128_struct *instructions, uint32_t numInstructions) = 0;
private:
};

class NodeCentricScheduler : public CgraScheduler
{
public:
	NodeCentricScheduler();
	bool schedule(VexCgraSimulator &cgra, uint128_struct *instructions, uint32_t numInstructions);
};

class EdgeCentricScheduler : public CgraScheduler
{
public:
	EdgeCentricScheduler();
	bool schedule(VexCgraSimulator &cgra, uint128_struct *instructions, uint32_t numInstructions);
};

#endif // CGRASCHEDULER_H
