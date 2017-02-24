// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_multi_thread.h"
#include <stddef.h>
#include <assert.h>
#include <omp.h>

namespace fj {

int MtGetMaxThreadCount(void)
{
  return omp_get_max_threads();
}

int MtGetRunningThreadCount(void)
{
  return omp_get_num_threads();
}

int MtGetThreadID(void)
{
  return omp_get_thread_num();
}

void MtSetMaxThreadCount(int count)
{
  omp_set_num_threads(count);
}

ThreadStatus MtRunThreadLoop(void *data, ThreadFunction run_thread, int thread_count,
    int start, int end)
{
  ThreadStatus global_status = THREAD_LOOP_CONTINUE;
  int i = 0;

  assert(run_thread != NULL);
  MtSetMaxThreadCount(thread_count);

#pragma omp parallel for schedule(dynamic)
  for (i = start; i < end; i++) {
    ThreadStatus local_status = THREAD_LOOP_CONTINUE;
    ThreadContext cxt;

    if (global_status == THREAD_LOOP_CANCEL) {
      continue;
    }

    cxt.thread_count = MtGetRunningThreadCount();
    cxt.thread_id = MtGetThreadID();
    cxt.iteration_count = end - start;
    cxt.iteration_id = i;

    local_status = run_thread(data, &cxt);
    if (local_status == THREAD_LOOP_CANCEL) {
      global_status = THREAD_LOOP_CANCEL;
    }
  }

  return global_status;
}

void MtCriticalSection(void *data, CriticalFunction critical)
{
#pragma omp critical
  critical(data);
}

} // namespace xxx
