#pragma once

#include <simulator/vexSimulator.h>
#include <lib/traceQueue.h>

class VexTraceSimulator : public VexSimulator
{
public:
  VexTraceSimulator(ac_int<128, false> * instructionMemory, TraceQueue * q);
  ~VexTraceSimulator();

  virtual int doStep();
private:
  TraceQueue * _tracer;
};
