/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_multi_thread.h"
#include <stddef.h>
#include <omp.h>

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

int MtRunThread(void *data, ThreadFunction run, int thread_count, int start, int end)
{
	int i = 0;

  MtSetMaxThreadCount(thread_count);

#pragma omp parallel for schedule(dynamic)
	for (i = start; i < end; i++) {
    struct ThreadContext cxt;
    int err = 0;

    cxt.thread_count = MtGetRunningThreadCount();
    cxt.thread_id = MtGetThreadID();
    cxt.iteration_count = end - start;
    cxt.iteration_id = i;

		err = run(data, &cxt);
    if (err) {
      /*
      break;
      */
    }
	}

  return 0;
}

void MtCriticalSection(void *data, CriticalFunction critical)
{
#pragma omp critical
  critical(data);
}
