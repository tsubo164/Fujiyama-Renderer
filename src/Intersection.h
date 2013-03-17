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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

