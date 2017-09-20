#include <simulator/vexTraceSimulator.h>
#include <iostream>

VexTraceSimulator::VexTraceSimulator(ac_int<128, false> * memoryPtr, TraceQueue * q) :
  VexSimulator(memoryPtr),
  _tracer(q)
{
}

VexTraceSimulator::~VexTraceSimulator()
{
}

int VexTraceSimulator::doStep()
{
  static int c = 0;
    TraceQueue::Entry e;
    e.pc = this->cycle;
    for (int i = 0; i < 32; ++i)
      e.registers[i] = REG[i];

    for (int i = 0; i < 8; ++i)
      e.instructions[i] = RI[ac_int<32,false>(PC+(i/4))].slc<32>((i%4)*32);

  _tracer->trace(e);

  VexSimulator::doStep();
}
