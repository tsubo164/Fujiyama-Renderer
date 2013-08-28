/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TIMER_H
#define TIMER_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Timer {
  clock_t start_clock;
  time_t start_time;
};

struct Elapse {
  int hour;
  int min;
  double sec;
};

extern void TimerStart(struct Timer *t);
extern struct Elapse TimerGetElapse(const struct Timer *t);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
