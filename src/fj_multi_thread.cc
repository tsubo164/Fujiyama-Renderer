// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_multi_thread.h"

#include <unordered_map>
#include <utility>
#include <vector>
#include <thread>
#include <mutex>

namespace fj {

// c++ thread id (0x2384b312) <=> our thread id (0 to N-1)
static std::unordered_map<std::thread::id, int> thread_id_map;
static std::mutex thread_id_map_mtx;
static int active_thread_count = 1;

// manage all threads' status
static ThreadStatus global_loop_status = THREAD_LOOP_CONTINUE;
static std::mutex global_stat_mtx;

// manage iterations
static int iteration_que_index = 0;
static std::mutex iteration_que_index_mtx;

static void register_thread_id(int id)
{
  std::lock_guard<std::mutex> lock(thread_id_map_mtx);

  const auto this_id = std::this_thread::get_id();
  thread_id_map[this_id] = id;
}

static void unregister_thread_id()
{
  std::lock_guard<std::mutex> lock(thread_id_map_mtx);

  const auto this_id = std::this_thread::get_id();
  thread_id_map.erase(this_id);
}

/*
static int get_registered_thread_count()
{
  std::lock_guard<std::mutex> lock(thread_id_map_mtx);
  return thread_id_map.size();
}
*/

// thread registeration RAII
class ThreadRegistration {
public:
  ThreadRegistration(int thread_id)
  {
    register_thread_id(thread_id);
  }
  ~ThreadRegistration()
  {
    unregister_thread_id();
  }
};

static int get_thread_id()
{
  std::lock_guard<std::mutex> lock(thread_id_map_mtx);
  const auto this_id = std::this_thread::get_id();
  return thread_id_map[this_id];
}

static int get_running_thread_count()
{
  std::lock_guard<std::mutex> lock(thread_id_map_mtx);
  return thread_id_map.size();
}

static int get_global_loop_status()
{
  std::lock_guard<std::mutex> lock(global_stat_mtx);
  return global_loop_status;
}

static void cancel_parallel_loop()
{
  std::lock_guard<std::mutex> lock(global_stat_mtx);
  global_loop_status = THREAD_LOOP_CANCEL;
}

static void init_iteration_que_index()
{
  iteration_que_index = 0;
}

static int checkout_iteration_id(const std::vector<int> &iteration_que)
{
  static std::mutex mtx;
  std::lock_guard<std::mutex> lock(iteration_que_index_mtx);

  if (iteration_que_index >= iteration_que.size()) {
    return -1;
  }

  const int id = iteration_que[iteration_que_index];
  iteration_que_index++;

  return id;
}

static void parallel_for(int thread_id, const std::vector<int> &iteration_que,
    void *data, ThreadFunction task)
{
  ThreadRegistration reg(thread_id);

  for (;;) {
    // check global loop status
    const ThreadStatus global_status = get_global_loop_status();
    if (global_status == THREAD_LOOP_CANCEL) {
      break;
    }

    // checkout next iteration
    const int iteration_id = checkout_iteration_id(iteration_que);
    if (iteration_id == -1) {
      break;
    }

    ThreadContext cxt;
    cxt.thread_count = MtGetRunningThreadCount();
    cxt.thread_id = MtGetThreadID();
    cxt.iteration_count = iteration_que.size();
    cxt.iteration_id = iteration_id;

    const ThreadStatus local_status = task(data, &cxt);
    if (local_status == THREAD_LOOP_CANCEL) {
      cancel_parallel_loop();
      break;
    }
  }
}

//TODO rename to MtGetAvailableMaxThreadCount
int MtGetMaxThreadCount(void)
{
  return std::thread::hardware_concurrency();
}

//TODO rename to MtGetActiveThreadCount
int MtGetRunningThreadCount(void)
{
  return get_running_thread_count();
}

int MtGetThreadID(void)
{
  return get_thread_id();
}

//TODO rename to MtSetActiveThreadCount
void MtSetMaxThreadCount(int count)
{
  const int max = MtGetMaxThreadCount();
  int n = count < 1 ? 1 : count;
  n = n > max ? max : n;

  active_thread_count = n;
}

//TODO rename to MtRunParallelLoop
ThreadStatus MtRunThreadLoop(void *data, ThreadFunction run_thread, int thread_count,
    int start, int end)
{
  //TODO take iteration_que from caller
  std::vector<int> iteration_que;
  for (int i = start; i < end; i++) {
    iteration_que.push_back(i);
  }

  std::vector<std::thread> threads;
  init_iteration_que_index();

  for (int i = 0; i < thread_count; i++) {
    const int thread_id = i;
    threads.push_back(std::thread(parallel_for, thread_id, std::cref(iteration_que),
          data, run_thread));
  }

  for (auto &t: threads) {
    t.join();
  }

  return get_global_loop_status();
}

void MtCriticalSection(void *data, CriticalFunction critical)
{
  static std::mutex mtx;
  std::lock_guard<std::mutex> lock(mtx);
  critical(data);
}

} // namespace xxx
