#ifndef CGRASCHEDULER_H
#define CGRASCHEDULER_H

#include <simulator/vexCgraSimulator.h>
#include <isa/irISA.h>
#include <cstdint>

class CgraScheduler
{
public:
	CgraScheduler();

	bool schedule(VexCgraSimulator &cgra, uint128_struct *instructions, uint32_t numInstructions);
private:
	uint8_t * _cgra_cache;

};

#endif // CGRASCHEDULER_H
