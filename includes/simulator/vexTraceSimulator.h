#pragma once

#include <lib/traceQueue.h>
#include <simulator/vexSimulator.h>

class VexTraceSimulator : public VexSimulator {
public:
  VexTraceSimulator(unsigned int* instructionMemory, TraceQueue* q);
  ~VexTraceSimulator();

  virtual int doStep();

private:
  TraceQueue* _tracer;
};
