/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TIMER_H
#define FJ_TIMER_H

#include <time.h>

namespace fj {

struct Timer {
  time_t start_time;
};

struct Elapse {
  int hour;
  int min;
  int sec;
};

extern void TimerStart(struct Timer *timer);
extern struct Elapse TimerGetElapse(const struct Timer *timer);

} // namespace xxx

#endif /* FJ_XXX_H */
