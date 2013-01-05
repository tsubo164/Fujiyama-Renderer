/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Test.h"
#include "Vector.h"
#include <stdio.h>

int main()
{
	{
		const int x = 2;
		const int y = 122;
		int v[2];

		VEC2_SET(v, x, y);
		TEST(v[0] == x);
		TEST(v[1] == y);
	}
	printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
		TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

	return 0;
}

