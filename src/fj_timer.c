/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_timer.h"

void TimerStart(struct Timer *t)
{
  t->start_clock = clock();
  time(&t->start_time);
}

struct Elapse TimerGetElapse(const struct Timer *t)
{
  const clock_t end_clock = clock();
  struct Elapse elapse;
  time_t end_time;

  double total_seconds;
  double diff_seconds;

  time(&end_time);
  diff_seconds = difftime(end_time, t->start_time);

  if (diff_seconds > 60 || end_clock == -1) {
    total_seconds = diff_seconds;
  } else {
    total_seconds = (double)(end_clock - t->start_clock) / CLOCKS_PER_SEC;
  }

  elapse.hour = (int) (total_seconds / (60*60));
  elapse.min = (int) ((total_seconds - elapse.hour*60*60) / 60);
  elapse.sec = total_seconds - elapse.hour*60*60 - elapse.min*60;

  return elapse;
}

