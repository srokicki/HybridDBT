#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

/**
 * @brief This class represents a concurrency-safe queue used to register execution
 * traces chunks. It is useful for threaded multi-simulators applications.
 */
class TraceQueue
{
public:

  /**
   * @brief Entry represent an execution trace line
   */
  typedef struct _Entry
  {
    uint32_t instructions[8];
    uint64_t registers[8];
    uint64_t pc;
  } Entry;

  TraceQueue();
  ~TraceQueue();

  /**
   * @brief Registers a trace in the current chunk being executed
   */
  void trace(const Entry& e);

  /**
   * @brief Read and pops the next readable trace chunk.
   */
  std::vector<Entry> nextChunk();

private:
  std::vector<Entry> _current_trace;
  std::queue< std::vector<Entry> > _trace_queue;

  std::mutex _mtx_current;

  std::condition_variable _cv;
  std::mutex _mtx_queue;
};

