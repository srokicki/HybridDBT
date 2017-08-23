#include <lib/traceQueue.h>

TraceQueue::TraceQueue() {}
TraceQueue::~TraceQueue() {}

inline bool pushSignal(const TraceQueue::Entry& e)
{
  return !(e.pc % 100);
}

void TraceQueue::trace(const TraceQueue::Entry& e)
{
  std::unique_lock<std::mutex> lck(_mtx_current);

  _current_trace.push_back(e);

  if (pushSignal(e))
  {
    std::unique_lock<std::mutex> lck_queue(_mtx_queue);
    _trace_queue.push(_current_trace);
    _current_trace.clear();

    _cv.notify_one();
  }
}

//void TraceQueue::push_trace()
//{
//  std::unique_lock<std::mutex> lck1(_mtx_current);
//  std::unique_lock<std::mutex> lck2(_mtx_queue);
//
//  _trace_queue.push(_current_trace);
//  _current_trace.clear();
//
//  _cv.notify_one();
//}

std::vector<TraceQueue::Entry> TraceQueue::nextChunk()
{
  std::unique_lock<std::mutex> lck(_mtx_queue);
  while (_trace_queue.empty()) _cv.wait(lck);

  std::vector<TraceQueue::Entry> ret = _trace_queue.back();
  _trace_queue.pop();

  return ret;
}


