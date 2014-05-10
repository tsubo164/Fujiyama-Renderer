/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "unit_test.h"
#include <stdio.h>

static int n_pass = 0;
static int n_fail = 0;
static int n_total = 0;

void TestPass(const char *expr, const char *file, int line)
{
  fprintf(stdout, "  :PASS :%s:%d: %s\n", file, line, expr);
  n_pass++;
  n_total++;
}

void TestFail(const char *expr, const char *file, int line)
{
  fprintf(stdout, "* :FAIL :%s:%d: %s\n", file, line, expr);
  n_fail++;
  n_total++;
}

int TestGetPassCount()
{
  return n_pass;
}

int TestGetFailCount()
{
  return n_fail;
}

int TestGetTotalCount()
{
  return n_total;
}

/* the following codes from comp.lang.c FAQ list · Question 14.5
   http://c-faq.com/fp/fpequal.html */
#define Abs(x)    ((x) < 0 ? -(x) : (x))
#define Max(a, b) ((a) > (b) ? (a) : (b))

static double RelDif(double a, double b)
{
  double c = Abs(a);
  double d = Abs(b);

  d = Max(c, d);

  return d == 0.0 ? 0.0 : Abs(a - b) / d;
}

int TestDoubleEq(double a, double b)
{
  return RelDif(a, b) <= 1e-15 ? 1 : 0;
}

int TestFloatEq(float a, float b)
{
  return RelDif(a, b) <= 1e-7 ? 1 : 0;
}

