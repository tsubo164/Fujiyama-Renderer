/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
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

typedef int (*ThreadFunction)(void *data, const struct ThreadContext *context);

extern int MtGetMaxThreadCount(void);
extern int MtGetThreadCount(void);
extern int MtGetThreadID(void);

extern void MtSetMaxThreadCount(int count);

extern int MtRunThread(void *data, ThreadFunction run, int start, int end);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
