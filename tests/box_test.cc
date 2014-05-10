/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
 */

#include "unit_test.h"
#include "fj_box.h"
#include "fj_vector.h"
#include <stdio.h>
#include <float.h>

int main()
{
  {
    struct Box box = {{-1, -1, -1}, {1, 1, 1}};
    struct Vector orig;
    struct Vector dir(0, 0, 1);
    double ray_tmin = 0;
    double ray_tmax = 1000;
    double hit_tmin = -FLT_MAX;
    double hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(&box, &orig, &dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 1);
    TEST(TestDoubleEq(hit_tmin, -1));
    TEST(TestDoubleEq(hit_tmax, 1));
  }
  {
    struct Box box = {{-1, -1, -1}, {1, 1, 1}};
    struct Vector orig(0, 0, -2);
    struct Vector dir(0, 0, 1);
    double ray_tmin = 0;
    double ray_tmax = 1000;
    double hit_tmin = -FLT_MAX;
    double hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(&box, &orig, &dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 1);
    TEST(TestDoubleEq(hit_tmin, 1));
    TEST(TestDoubleEq(hit_tmax, 3));
  }
  {
    struct Box box = {{-1, -1, -1}, {1, 1, 1}};
    struct Vector orig(0, 0, -2);
    struct Vector dir(0, 0, 1);
    double ray_tmin = 0;
    double ray_tmax = 2;
    double hit_tmin = -FLT_MAX;
    double hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(&box, &orig, &dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 1);
    TEST(TestDoubleEq(hit_tmin, 1));
    TEST(TestDoubleEq(hit_tmax, 3));
  }
  {
    struct Box box = {{-1, -1, -1}, {1, 1, 1}};
    struct Vector orig(0, 0, 2);
    struct Vector dir(0, 0, 1);
    double ray_tmin = 0;
    double ray_tmax = 1000;
    double hit_tmin = -FLT_MAX;
    double hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(&box, &orig, &dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 0);
    TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
    TEST(TestDoubleEq(hit_tmax, FLT_MAX));
  }
  {
    struct Box box = {{-1, -1, -1}, {1, 1, 1}};
    struct Vector orig(0, 0, -2);
    struct Vector dir(0, 0, 1);
    double ray_tmin = 0;
    double ray_tmax = 1;
    double hit_tmin = -FLT_MAX;
    double hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(&box, &orig, &dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 0);
    TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
    TEST(TestDoubleEq(hit_tmax, FLT_MAX));
  }
  {
    struct Box box(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    struct Vector orig;
    struct Vector dir(0, 0, 1);
    double ray_tmin = 0;
    double ray_tmax = 1;
    double hit_tmin = -FLT_MAX;
    double hit_tmax = FLT_MAX;
    int hit;

    hit = BoxRayIntersect(&box, &orig, &dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

    TEST(hit == 0);
    TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
    TEST(TestDoubleEq(hit_tmax, FLT_MAX));
  }
  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
      TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}

