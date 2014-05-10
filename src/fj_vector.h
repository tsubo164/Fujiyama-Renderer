/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VECTOR_H
#define FJ_VECTOR_H

#include <math.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Vector2 {
  Vector2() : x(0), y(0) {}
  Vector2(double xx, double yy) : x(xx), y(yy) {}
  double x, y;
};

struct Vector {
  Vector() : x(0), y(0), z(0) {}
  Vector(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}
  ~Vector() {}
  double x, y, z;
};

extern struct Vector *VecAlloc(long count);
extern struct Vector *VecRealloc(struct Vector *v, long count);
extern void VecFree(struct Vector *v);

extern void VecPrint(const struct Vector *a);

/* VEC2 */
#define VEC2_DOT(a,b) ((a)->x * (b)->x + (a)->y * (b)->y)

/* VEC3 */
#define VEC3_SET(dst,X,Y,Z) do { \
  (dst)->x = (X); \
  (dst)->y = (Y); \
  (dst)->z = (Z); \
  } while(0)

#define VEC3_DOT(a,b) ((a)->x * (b)->x + (a)->y * (b)->y + (a)->z * (b)->z)

#define VEC3_LEN(a) (sqrt(VEC3_DOT((a),(a))))

#define VEC3_NORMALIZE(a) do { \
  double len = VEC3_LEN((a)); \
  if (len == 0) break; \
  len = 1./len; \
  (a)->x *= len; \
  (a)->y *= len; \
  (a)->z *= len; \
  } while(0)

#define VEC3_CROSS(dst,a,b) do { \
  (dst)->x = (a)->y * (b)->z - (a)->z * (b)->y; \
  (dst)->y = (a)->z * (b)->x - (a)->x * (b)->z; \
  (dst)->z = (a)->x * (b)->y - (a)->y * (b)->x; \
  } while(0)

#define VEC3_LERP(dst,a,b,t) do { \
  (dst)->x = (1-(t)) * (a)->x + (t) * (b)->x; \
  (dst)->y = (1-(t)) * (a)->y + (t) * (b)->y; \
  (dst)->z = (1-(t)) * (a)->z + (t) * (b)->z; \
  } while(0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
