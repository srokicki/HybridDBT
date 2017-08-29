#include <lib/threadedDebug.h>
#include <lib/traceQueue.h>
#include <lib/log.h>
#include <chrono>

ThreadedDebug::ThreadedDebug(TraceQueue * tracer) :
  _tracer(tracer),
  _running(false),
  _thread(nullptr)
{
}

ThreadedDebug::~ThreadedDebug()
{
  if (_thread)
  {
    _running = false;
    _thread->join();
    delete _thread;
  }
}

void ThreadedDebug::_run_func()
{
  while (_running)
  {
    if (!_tracer->hasNext())
      continue;

    auto v = _tracer->nextChunk();

    for (auto e : v) {
      for (auto i : e.registers) {
        Log::printf(0, "%d ", i);// << ' ';
      }
      Log::printf(0, "\n");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void ThreadedDebug::run()
{
  _running = true;
  _thread = new std::thread(&ThreadedDebug::_run_func, *this);
}
