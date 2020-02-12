
#ifndef INCLUDES_SIMULATOR_EMPTYSIMULATOR_CPP_
#define INCLUDES_SIMULATOR_EMPTYSIMULATOR_CPP_

#include <simulator/loadQueueVexSimulator.h>
#include <types.h>

class EmptySimulator : public LoadQueueVexSimulator {
public:
  EmptySimulator(unsigned int* instructionMemory, unsigned int* specData);
  ~EmptySimulator(void);

  int doStep();
};

#endif /* INCLUDES_SIMULATOR_EMPTYSIMULATOR_CPP_ */
