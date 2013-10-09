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

int MtGetThreadCount(void)
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

int MtRunThread(void *data, ThreadFunction run, int start, int end)
{
  struct ThreadContext cxt;
	int i = 0;

  cxt.thread_id = 0;
  cxt.iteration_id = 0;
  cxt.iteration_count = end - start;

#pragma omp parallel for
	for (i = start; i < end; i++) {
    cxt.thread_count = MtGetThreadCount();
    cxt.thread_id = MtGetThreadID();
    cxt.iteration_id = i;

		run(data, &cxt);
	}

  return 0;
}
