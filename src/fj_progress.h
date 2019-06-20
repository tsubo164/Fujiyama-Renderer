// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROGRESS_H
#define FJ_PROGRESS_H

#include "fj_compatibility.h"
#include "fj_types.h"

namespace fj {

typedef int64_t Iteration;

enum ProgressStatus {
  PROGRESS_ONGOING = 0,
  PROGRESS_DONE = 1
};

class FJ_API Progress {
public:
  Progress();
  ~Progress();

  void Start(Iteration total_iterations);
  ProgressStatus Increment();
  void Done();

private:
  Iteration total_iterations_;
  Iteration iteration_;
};

} // namespace xxx

#endif // FJ_XXX_H
