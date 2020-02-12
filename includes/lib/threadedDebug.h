#pragma once

class TraceQueue;

#include <atomic>
#include <thread>

class ThreadedDebug {
public:
  ThreadedDebug(TraceQueue* tracer);
  ~ThreadedDebug();

  void run();

private:
  void _run_func();

  TraceQueue* _tracer;
  bool _running;
  std::thread* _thread;
};
