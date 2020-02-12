#include <chrono>
#include <lib/log.h>
#include <lib/threadedDebug.h>
#include <lib/traceQueue.h>

#include <isa/vexISA.h>

FILE* outFile;

ThreadedDebug::ThreadedDebug(TraceQueue* tracer) : _tracer(tracer), _running(false), _thread(nullptr)
{
  outFile = fopen("trace.log", "w");
}

ThreadedDebug::~ThreadedDebug()
{
  if (_thread) {
    _running = false;
    _thread->join();
    delete _thread;
  }
}

void ThreadedDebug::_run_func()
{

  while (_running) {
    if (!_tracer->hasNext())
      continue;

    auto v = _tracer->nextChunk();

    //  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void ThreadedDebug::run()
{
  _running = true;
  _thread  = new std::thread(&ThreadedDebug::_run_func, *this);
}
