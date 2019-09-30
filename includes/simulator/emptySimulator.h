
#ifndef INCLUDES_SIMULATOR_EMPTYSIMULATOR_CPP_
#define INCLUDES_SIMULATOR_EMPTYSIMULATOR_CPP_

#include <types.h>
#include <simulator/vexSimulator.h>


class EmptySimulator : public VexSimulator {
public:

	EmptySimulator(unsigned int *instructionMemory, unsigned int *specData);
	~EmptySimulator(void);

};


#endif /* INCLUDES_SIMULATOR_EMPTYSIMULATOR_CPP_ */
