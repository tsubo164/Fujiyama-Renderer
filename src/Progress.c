/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Progress.h"
#include "Memory.h"

#include <stdio.h>
#include <assert.h>

struct Progress {
  int total_iterations;
  int iteration;
};

struct Progress *PrgNew(void)
{
  struct Progress *prg;

  prg = MEM_ALLOC(struct Progress);
  if (prg == NULL)
    return NULL;

  prg->total_iterations = 0;
  prg->iteration = 0;

  return prg;
}

void PrgFree(struct Progress *prg)
{
  if (prg == NULL)
    return;
  MEM_FREE(prg);
}

void PrgStart(struct Progress *prg, int total_iterations)
{
  assert(total_iterations > 0);

  prg->total_iterations = total_iterations;
  prg->iteration = 0;
  printf("....1....2....3....4....5....6....7....8....9....0\n");
}

void PrgIncrement(struct Progress *prg)
{
  const int TOTAL_OUTPUTS = 50;
  const float OUTPUTS_DIV = 100./TOTAL_OUTPUTS;

  float prev_percent, next_percent;
  int prev_outputs, next_outputs;
  int diff_outputs;
  int i;

  if (prg->iteration >= prg->total_iterations) {
    fprintf(stderr, "warning: progress incremented after reaching total iterations\n");
    fflush(stderr);
  }

  prev_percent = prg->iteration / (float)prg->total_iterations * 100.;
  prg->iteration++;
  next_percent = prg->iteration / (float)prg->total_iterations * 100.;

  prev_outputs = (int) (prev_percent / OUTPUTS_DIV);
  next_outputs = (int) (next_percent / OUTPUTS_DIV);
  diff_outputs = next_outputs - prev_outputs;

  for (i = 0; i < diff_outputs; i++) {
    printf("-");
    fflush(stdout);
  }
}

void PrgDone(struct Progress *prg)
{
  if (prg->iteration != prg->total_iterations) {
    fprintf(stderr, "warning: progress done before reaching total iterations: "
        "%d/%d\n", prg->iteration, prg->total_iterations);
    fflush(stderr);
  }

  printf("\n");
}

