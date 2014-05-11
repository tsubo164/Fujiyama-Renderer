/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_timer.h"

namespace fj {

void TimerStart(struct Timer *timer)
{
  time(&timer->start_time);
}

struct Elapse TimerGetElapse(const struct Timer *timer)
{
  struct Elapse elapse = {0, 0, 1};
  double total_seconds = 0;
  time_t end_time;

  time(&end_time);
  total_seconds = difftime(end_time, timer->start_time);

  if (total_seconds > 1) {
    elapse.hour = (int) (total_seconds / (60*60));
    elapse.min = (int) ((total_seconds - elapse.hour*60*60) / 60);
    elapse.sec = total_seconds - elapse.hour*60*60 - elapse.min*60;
  }

  return elapse;
}

} // namespace xxx
