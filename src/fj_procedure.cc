// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_timer.h"
#include <cstdio>

namespace fj {

Procedure::Procedure()
{
}

Procedure::~Procedure()
{
}

int Procedure::Run() const
{
  // TODO come up with the best place to put message
  printf("Running Procedure ...\n");

  Timer timer;
  timer.Start();

  const int err = run();

  const Elapse elapse = timer.GetElapse();

  if (err) {
    printf("Error: %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
    return -1;
  } else {
    printf("Done: %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
    return 0;
  }
}

} // namespace xxx
