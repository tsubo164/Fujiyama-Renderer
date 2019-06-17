// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_MULTI_THREAD_H
#define FJ_MULTI_THREAD_H

#include "fj_compatibility.h"
#include <vector>

namespace fj {

class ThreadContext {
public:
  ThreadContext() {}
  ~ThreadContext() {}

public:
  int iteration_id = 0;
  int iteration_count = 0;
  int thread_id = 0;
  int thread_count = 0;
};

enum class LoopStatus {
  Continue = 0,
  Cancel,
};

using TaskFunction = LoopStatus (*)(void *data, const ThreadContext *context);
using CriticalFunction = void (*)(void *data);

int MtGetMaxAvailableThreadCount();
int MtGetActiveThreadCount();
void MtSetActiveThreadCount(int count);
// TODO possible to hide this from plugin?
FJ_API int MtGetThreadID();

LoopStatus MtRunParallelLoop(void *data, TaskFunction task_fn,
    int thread_count, const std::vector<int> &iteration_que);
void MtCriticalSection(void *data, CriticalFunction critical_fn);

} // namespace xxx

#endif // FJ_XXX_H
