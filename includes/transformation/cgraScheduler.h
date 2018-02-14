#ifndef CGRASCHEDULER_H
#define CGRASCHEDULER_H

#include <simulator/cgraSimulator.h>
#include <isa/irISA.h>
#include <cstdint>

class CgraScheduler
{
public:
	CgraScheduler();

	bool schedule(CgraSimulator &cgra, uint128_struct *instructions, uint32_t numInstructions);
private:
	uint8_t * _cgra_cache;

};

#endif // CGRASCHEDULER_H
