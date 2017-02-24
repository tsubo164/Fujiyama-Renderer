// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TIMER_H
#define FJ_TIMER_H

#include "fj_compatibility.h"
#include <ctime>

namespace fj {

class FJ_API Elapse {
public:
  Elapse() : hour(0), min(0), sec(0) {}
  ~Elapse() {}

public:
  int hour;
  int min;
  int sec;
};

class FJ_API Timer {
public:
  Timer() : start_time_(0) {}
  ~Timer() {}

  void Start();
  Elapse GetElapse() const;

private:
  time_t start_time_;
};

} // namespace xxx

#endif // FJ_XXX_H
