#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

class TraceQueue
{
public:

  typedef struct _Entry
  {
    uint32_t instructions[8];
    uint64_t registers[8]
    uint64_t pc;
  } Entry;

  TraceQueue();
  ~TraceQueue();

  void trace(const Entry& t);
  std::vector<Entry> nextChunk();

private:
  std::vector<Entry> _current_trace;
  std::queue< std::vector<Entry> > _trace_queue;

  std::mutex _mtx_current;

  std::condition_variable _cv;
  std::mutex _mtx_queue;
};

