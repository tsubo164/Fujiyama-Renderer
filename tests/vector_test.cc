/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "unit_test.h"
#include "fj_vector.h"
#include <stdio.h>

using namespace fj;

int main()
{
  {
    const struct Vector a(1, 0, 0);
    const struct Vector b(0, 1, 0);
    struct Vector c;

    VEC3_CROSS(&c, &a, &b);

    TEST(c.x == 0);
    TEST(c.y == 0);
    TEST(c.z == 1);
  }
  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
    TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}

