#include <isa/vexISA.h>
#include <lib/log.h>
#include <simulator/emptySimulator.h>
#include <simulator/loadQueueVexSimulator.h>

EmptySimulator::EmptySimulator(unsigned int* instructionMemory, unsigned int* specData)
    : LoadQueueVexSimulator(instructionMemory, specData)
{
  this->PC = 0;
};
EmptySimulator::~EmptySimulator(void){};

int EmptySimulator::doStep()
{
  return 0;
}
