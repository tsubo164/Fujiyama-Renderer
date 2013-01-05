/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
 */

#include "Test.h"
#include "Box.h"
#include <stdio.h>
#include <float.h>

int main()
{
	{
		double box[6] = {-1, -1, -1, 1, 1, 1};
		double orig[3] = {0, 0, 0};
		double dir[3] = {0, 0, 1};
		double ray_tmin = 0;
		double ray_tmax = 1000;
		double hit_tmin = -FLT_MAX;
		double hit_tmax = FLT_MAX;
		int hit;

		hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

		TEST(hit == 1);
		TEST(TestDoubleEq(hit_tmin, -1));
		TEST(TestDoubleEq(hit_tmax, 1));
	}
	{
		double box[6] = {-1, -1, -1, 1, 1, 1};
		double orig[3] = {0, 0, -2};
		double dir[3] = {0, 0, 1};
		double ray_tmin = 0;
		double ray_tmax = 1000;
		double hit_tmin = -FLT_MAX;
		double hit_tmax = FLT_MAX;
		int hit;

		hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

		TEST(hit == 1);
		TEST(TestDoubleEq(hit_tmin, 1));
		TEST(TestDoubleEq(hit_tmax, 3));
	}
	{
		double box[6] = {-1, -1, -1, 1, 1, 1};
		double orig[3] = {0, 0, -2};
		double dir[3] = {0, 0, 1};
		double ray_tmin = 0;
		double ray_tmax = 2;
		double hit_tmin = -FLT_MAX;
		double hit_tmax = FLT_MAX;
		int hit;

		hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

		TEST(hit == 1);
		TEST(TestDoubleEq(hit_tmin, 1));
		TEST(TestDoubleEq(hit_tmax, 3));
	}
	{
		double box[6] = {-1, -1, -1, 1, 1, 1};
		double orig[3] = {0, 0, 2};
		double dir[3] = {0, 0, 1};
		double ray_tmin = 0;
		double ray_tmax = 1000;
		double hit_tmin = -FLT_MAX;
		double hit_tmax = FLT_MAX;
		int hit;

		hit = BoxRayIntersect(box, orig, dir, ray_tmin, ray_tmax, &hit_tmin, &hit_tmax);

		TEST(hit == 0);
		TEST(TestDoubleEq(hit_tmin, -FLT_MAX));
		TEST(TestDoubleEq(hit_tmax, FLT_MAX));
	}
	{
		double box[6] = {-1, -1, -1, 1, 1, 1};
		double orig[3] = {0, 0, -2};
		double dir[3] = {0, 0, 1};
		double ray_tmin = 0;
		double ray_tmax = 1;
		double hit_tmin = -FLT_MAX;
		double hit_tmax = FLT_MAX;
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

