/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
  DO_NOT_CULL_BACKFACES = 0,
  CULL_BACKFACES = 1
};

struct Vector;
struct Box;

extern double TriComputeArea(
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2);

extern void TriComputeBounds(struct Box *box,
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2);

extern void TriComputeFaceNormal(struct Vector *N,
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2);

extern void TriComputeNormal(struct Vector *N,
    const struct Vector *N0, const struct Vector *N1, const struct Vector *N2,
    double u, double v);

extern int TriRayIntersect(
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2,
    const struct Vector *orig, const struct Vector *dir, int cull_backfaces,
    double *t, double *u, double *v);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

