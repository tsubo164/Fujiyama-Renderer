/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef CURVE_H
#define CURVE_H

#include "Box.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Curve;
struct PrimitiveSet;
struct TexCoord;
struct Vector;
struct Color;

/* TODO temporary visible structure */
struct Curve {
	struct Box bounds;

	struct Vector *P;
	double *width;
	struct Color *Cd;
	struct TexCoord *uv;
	int *indices;

	int nverts;
	int ncurves;

	int *split_depth;
};

extern struct Curve *CrvNew(void);
extern void CrvFree(struct Curve *curve);

extern void *CrvAllocateVertex(struct Curve *curve, const char *attr_name, int nverts);
extern void *CrvAllocateCurve(struct Curve *curve, const char *attr_name, int ncurves);

extern void CrvComputeBounds(struct Curve *curve);
extern void CrvGetPrimitiveSet(const struct Curve *curve, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

