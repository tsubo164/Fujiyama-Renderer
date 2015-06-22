// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "unit_test.h"
#include "fj_box.h"
#include "fj_vector.h"
#include <cstdio>
#include <cfloat>

using namespace fj;

int main()
{
  {
    Box box(-1, -1, -1, 1, 1, 1);
    Vector orig;
    Vector dir(0, 0, 1);
    Real ray_tmin = 0;
    Real ray_tmax = 1000;
    Real hit_tmin = -FLT_MAX;
    Real hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 1);
    TEST(TestDoubleEq(hit_tmin, -1));
    TEST(TestDoubleEq(hit_tmax, 1));
  }
  {
    Box box(-1, -1, -1, 1, 1, 1);
    Vector orig(0, 0, -2);
    Vector dir(0, 0, 1);
    Real ray_tmin = 0;
    Real ray_tmax = 1000;
    Real hit_tmin = -FLT_MAX;
    Real hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 1);
    TEST(TestDoubleEq(hit_tmin, 1));
    TEST(TestDoubleEq(hit_tmax, 3));
  }
  {
    Box box(-1, -1, -1, 1, 1, 1);
    Vector orig(0, 0, -2);
    Vector dir(0, 0, 1);
    Real ray_tmin = 0;
    Real ray_tmax = 2;
    Real hit_tmin = -FLT_MAX;
    Real hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 1);
    TEST(TestDoubleEq(hit_tmin, 1));
    TEST(TestDoubleEq(hit_tmax, 3));
  }
  {
    Box box(-1, -1, -1, 1, 1, 1);
    Vector orig(0, 0, 2);
    Vector dir(0, 0, 1);
    Real ray_tmin = 0;
    Real ray_tmax = 1000;
    Real hit_tmin = -FLT_MAX;
    Real hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 0);
    TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
    TEST(TestDoubleEq(hit_tmax, FLT_MAX));
  }
  {
    Box box(-1, -1, -1, 1, 1, 1);
    Vector orig(0, 0, -2);
    Vector dir(0, 0, 1);
    Real ray_tmin = 0;
    Real ray_tmax = 1;
    Real hit_tmin = -FLT_MAX;
    Real hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 0);
    TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
    TEST(TestDoubleEq(hit_tmax, FLT_MAX));
  }
  {
    Box box(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    Vector orig;
    Vector dir(0, 0, 1);
    Real ray_tmin = 0;
    Real ray_tmax = 1;
    Real hit_tmin = -FLT_MAX;
    Real hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 0);
    TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
    TEST(TestDoubleEq(hit_tmax, FLT_MAX));
  }
  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
      TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}
