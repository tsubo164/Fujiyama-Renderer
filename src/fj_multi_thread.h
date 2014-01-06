/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MULTI_THREAD_H
#define FJ_MULTI_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

struct ThreadContext {
  int iteration_id;
  int iteration_count;
  int thread_id;
  int thread_count;
};

enum {
  THREAD_LOOP_CONTINUE = 0,
  THREAD_LOOP_CANCEL = 1
};
typedef int ThreadStatus;

typedef ThreadStatus (*ThreadFunction)(void *data, const struct ThreadContext *context);
typedef void (*CriticalFunction)(void *data);

extern int MtGetMaxThreadCount(void);
extern int MtGetRunningThreadCount(void);
extern int MtGetThreadID(void);

extern void MtSetMaxThreadCount(int count);

extern ThreadStatus MtRunThreadLoop(void *data, ThreadFunction run_thread, int thread_count,
    int start, int end);
extern void MtCriticalSection(void *data, CriticalFunction critical);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
