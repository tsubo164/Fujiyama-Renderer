// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_timer.h"

namespace fj {

void Timer::Start()
{
  time(&start_time_);
}

Elapse Timer::GetElapse() const
{
  time_t end_time;
  time(&end_time);

  const double total_seconds = difftime(end_time, start_time_);
  Elapse elapse;
  elapse.sec = 1;

  if (total_seconds > 1.) {
    elapse.hour = static_cast<int>(total_seconds / (60*60));
    elapse.min = static_cast<int>((total_seconds - elapse.hour*60*60) / 60);
    elapse.sec = total_seconds - elapse.hour*60*60 - elapse.min*60;
  }

  return elapse;
}

} // namespace xxx
