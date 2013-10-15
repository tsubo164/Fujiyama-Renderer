/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_progress.h"
#include "fj_memory.h"

#include <stdio.h>
#include <assert.h>

void PrgStart(struct Progress *progress, Iteration total_iterations)
{
  assert(total_iterations > 0);

  progress->total_iterations = total_iterations;
  progress->iteration = 0;
  printf("....1....2....3....4....5....6....7....8....9....0\n");
}

void PrgIncrement(struct Progress *progress)
{
  const int TOTAL_OUTPUTS = 50;
  const float OUTPUTS_DIV = 100./TOTAL_OUTPUTS;

  float prev_percent, next_percent;
  int prev_outputs, next_outputs;
  int diff_outputs;
  int i;

  if (progress->iteration >= progress->total_iterations) {
    fprintf(stderr, "warning: progress incremented after reaching total iterations\n");
    fflush(stderr);
  }

  prev_percent = progress->iteration / (float)progress->total_iterations * 100.;
  progress->iteration++;
  next_percent = progress->iteration / (float)progress->total_iterations * 100.;

  prev_outputs = (int) (prev_percent / OUTPUTS_DIV);
  next_outputs = (int) (next_percent / OUTPUTS_DIV);
  diff_outputs = next_outputs - prev_outputs;

  for (i = 0; i < diff_outputs; i++) {
    printf("-");
    fflush(stdout);
  }
}

void PrgDone(struct Progress *progress)
{
  if (progress->iteration != progress->total_iterations) {
    fprintf(stderr, "warning: progress done before reaching total iterations: "
        "%ld/%ld\n", progress->iteration, progress->total_iterations);
    fflush(stderr);
  }

  printf("\n");
}
