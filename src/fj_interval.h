// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_INTERVAL_H
#define FJ_INTERVAL_H

#include "fj_types.h"
#include <cstddef>

namespace fj {

class IntervalList;
class ObjectInstance;

// ray-march interval for volumetric object
class Interval {
public:
  Interval() :
      tmin(0),
      tmax(0),
      object(NULL),
      next(NULL)
  {}
  ~Interval() {}

public:
  Real tmin;
  Real tmax;
  const ObjectInstance *object;
  Interval *next;
};

class IntervalList {
public:
  IntervalList();
  ~IntervalList();

  void Push(const Interval &interval);
  int GetCount() const;

  Real GetMinT() const;
  Real GetMaxT() const;

  const Interval *GetHead() const;

private:
  Interval root_;
  int num_nodes_;
  Real tmin_;
  Real tmax_;
};

} // namespace xxx

#endif // FJ_XXX_H
