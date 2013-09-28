/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_RANDOM_H
#define FJ_RANDOM_H

#include "fj_compatibility.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Vector2;
struct Vector;

struct XorShift {
  uint32_t state[4];
};

extern void XorInit(struct XorShift *xr);

extern uint32_t XorNextInteger(struct XorShift *xr);
extern double XorNextFloat01(struct XorShift *xr);

extern void XorSolidSphereRand(struct XorShift *xr, struct Vector *out_position);
extern void XorHollowSphereRand(struct XorShift *xr, struct Vector *out_position);
extern void XorSolidCubeRand(struct XorShift *xr, struct Vector *out_position);

extern void XorSolidDiskRand(struct XorShift *xr, struct Vector2 *out_position);
extern void XorHollowDiskRand(struct XorShift *xr, struct Vector2 *out_position);
extern void XorGaussianDiskRand(struct XorShift *xr, struct Vector2 *out_position);

extern double XorGaussianRand(struct XorShift *xr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */

