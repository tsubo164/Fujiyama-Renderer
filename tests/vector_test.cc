// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "unit_test.h"
#include "fj_vector.h"
#include <cstdio>

using namespace fj;

int main()
{
  {
    const Vector a(1, 0, 0);
    const Vector b(0, 1, 0);

    const Vector c = Cross(a, b);

    TEST(c.x == 0);
    TEST(c.y == 0);
    TEST(c.z == 1);
  }
  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
    TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}

