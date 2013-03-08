/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "TexCoord.h"
#include "Vector.h"
#include "Color.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;

struct Intersection {
	struct Vector P;
	struct Vector N;
	struct Color Cd;
	struct TexCoord uv;

	struct Vector dPds;
	struct Vector dPdt;

	const struct ObjectInstance *object;
	int prim_id;

	double t_hit;
};

struct IntersectionList;

extern struct IntersectionList *IsectNew();
extern void IsectFree(struct IntersectionList *isects);

extern void IsectPush(struct IntersectionList *isects, const struct Intersection *isect);
extern const struct Intersection *IsectGet(struct IntersectionList *isects, int index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

