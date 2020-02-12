#include <lib/traceQueue.h>

TraceQueue::TraceQueue(unsigned int max_size)
    : _max_size(max_size), _trace_queue(new std::vector<TraceQueue::Entry>[max_size]), _written_trace(0), _read_trace(0)
{
}

TraceQueue::~TraceQueue() {}

inline bool pushSignal(const TraceQueue::Entry& e)
{
  return !(e.pc % 100);
}

void TraceQueue::trace(const TraceQueue::Entry& e)
{
  _trace_queue[_written_trace].push_back(e);

  if (pushSignal(e)) {
    std::unique_lock<std::mutex> lck(_mtx);
    while ((_written_trace + 1) % _max_size == _read_trace)
      _cv.wait(lck);

    _written_trace = (_written_trace + 1) % _max_size;

    _cv.notify_one();
  }
}

// void TraceQueue::push_trace()
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
  std::unique_lock<std::mutex> lck(_mtx);
  while (_read_trace == _written_trace)
    _cv.wait(lck);

  std::vector<TraceQueue::Entry> ret = _trace_queue[_read_trace];
  _trace_queue[_read_trace].clear();

  _read_trace = (_read_trace + 1) % _max_size;

  _cv.notify_one();
  return ret;
}

bool TraceQueue::hasNext()
{
  std::unique_lock<std::mutex> lck(_mtx);
  return _read_trace != _written_trace;
}
