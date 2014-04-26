/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_CURVE_H
#define FJ_CURVE_H

#include "fj_box.h"

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
  struct Vector *velocity;
  int *indices;

  int nverts;
  int ncurves;

  int *split_depth;
};

extern struct Curve *CrvNew(void);
extern void CrvFree(struct Curve *curve);

extern void *CrvAllocateVertex(struct Curve *curve, const char *attr_name, int nverts);
extern void *CrvAllocateCurve(struct Curve *curve, const char *attr_name, int ncurves);

extern struct Vector *CrvAddVelocity(struct Curve *curve);

extern void CrvComputeBounds(struct Curve *curve);
extern void CrvGetPrimitiveSet(const struct Curve *curve, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
