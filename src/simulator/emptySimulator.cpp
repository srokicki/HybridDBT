#include <simulator/loadQueueVexSimulator.h>
#include <isa/vexISA.h>
#include <lib/log.h>

EmptySimulator::EmptySimulator(unsigned int *instructionMemory, unsigned int *specData) : LoadQueueVexSimulator(instructionMemory, specData){this->PC = 0;};
EmptySimulator::~EmptySimulator(void){};

int EmptySimulator::doStep(){
    return 0;
}
