/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "unit_test.h"
#include "fj_numeric.h"
#include <stdio.h>

int main()
{
  {
    const int x = 2;
    const int y = 122;

    TEST(MIN(x, y) == x);
    TEST(MAX(x, y) == y);
  }
  {
    const float x = 12.35;
    const float y = 389.93;

    TEST(MIN(x, y) == x);
    TEST(MAX(x, y) == y);
  }
  {
    const double x = 12.35;
    const double y = 389.93;

    TEST(MIN(x, y) == x);
    TEST(MAX(x, y) == y);
  }
  {
    const int x = 2;
    const int l = -1;
    const int u = 122;

    TEST(CLAMP(x, l, u) == x);

    TEST(CLAMP(-14, l, u) == l);

    TEST(CLAMP(135, l, u) == u);

    TEST(CLAMP(u, l, u) == u);
  }
  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
    TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}

