#ifndef CGRASCHEDULER_H
#define CGRASCHEDULER_H

#include <simulator/cgraSimulator.h>
#include <cstdint>

class CgraScheduler
{
public:
  CgraScheduler();

  void schedule(CgraSimulator &cgra, uint32_t *instructions, uint32_t numInstructions);
private:
  uint8_t * _cgra_cache;

};

#endif // CGRASCHEDULER_H
