/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Filter.h"
#include "Memory.h"

#include <assert.h>
#include <math.h>

struct Filter {
  double xwidth, ywidth;
  double (*evaluate)(const struct Filter *filter, double x, double y);
};

static double eval_gaussian(const struct Filter *filter, double x, double y);
static double eval_box(const struct Filter *filter, double x, double y);

struct Filter *FltNew(int filtertype, double xwidth, double ywidth)
{
  struct Filter *filter;

  assert(xwidth > 0);
  assert(ywidth > 0);

  filter = SI_MEM_ALLOC(struct Filter);
  if (filter == NULL)
    return NULL;

  switch (filtertype) {
  case FLT_GAUSSIAN:
    filter->evaluate = eval_gaussian;
    break;
  case FLT_BOX:
    filter->evaluate = eval_box;
    break;
  default:
    assert(!"invalid filter type");
    break;
  }
  filter->xwidth = xwidth;
  filter->ywidth = ywidth;

  return filter;
}

void FltFree(struct Filter *filter)
{
  if (filter == NULL)
    return;

  SI_MEM_FREE(filter);
}

double FltEvaluate(const struct Filter *filter, double x, double y)
{
  return filter->evaluate(filter, x, y);
}

static double eval_gaussian(const struct Filter *filter, double x, double y)
{
  /* 
   *  The RenderMan Interface
   *  Version 3.2.1
   *  November, 2005
   */
  double xx = 2 * x / filter->xwidth;
  double yy = 2 * y / filter->ywidth;

  return exp(-2 * ( xx * xx + yy * yy ));
}

static double eval_box(const struct Filter *filter, double x, double y)
{
  return 1;
}

