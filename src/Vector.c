/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Vector.h"
#include <stdio.h>

void VecPrint(const struct Vector *a)
{
	printf("(%g, %g, %g)\n", a->x, a->y, a->z);
}

struct Vector *VecAlloc(long count)
{
	return (struct Vector *) malloc(sizeof(struct Vector) * count);
}

struct Vector *VecRealloc(struct Vector *v, long count)
{
	return (struct Vector *) realloc(v, sizeof(struct Vector) * count);
}

void VecFree(struct Vector *v)
{
	free(v);
}

#if 0
double VEC3_DOT(const struct Vector *a, const struct Vector *b)
{
	return
		a->x * b->x +
		a->y * b->y +
		a->z * b->z;
}

double VEC3_LEN(const struct Vector *a)
{
	return sqrt(
		a->x * a->x +
		a->y * a->y +
		a->z * a->z);
}

void VEC3_NORMALIZE(struct Vector *a)
{
	double len = VEC3_LEN(a);
	if (len == 0)
		return;

	len = 1./len;
	a->x *= len;
	a->y *= len;
	a->z *= len;
}

void VEC3_CROSS(struct Vector *dst, const struct Vector *a, const struct Vector *b)
{
	dst->x = a->y * b->z - a->z * b->y;
	dst->y = a->z * b->x - a->x * b->z;
	dst->z = a->x * b->y - a->y * b->x;
}

void VEC3_LERP(struct Vector *dst, const struct Vector *a, const struct Vector *b, double t)
{
	const double u = 1-t;
	dst->x = u * a->x + t * b->x;
	dst->y = u * a->y + t * b->y;
	dst->z = u * a->z + t * b->z;
}
#endif

