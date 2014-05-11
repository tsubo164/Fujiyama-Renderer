/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VECTOR_H
#define FJ_VECTOR_H

#include <cmath>
#include <stdlib.h>

namespace fj {

typedef double real;

struct Vector2 {
  Vector2() : x(0), y(0) {}
  Vector2(real xx, real yy) : x(xx), y(yy) {}
  real x, y;
};

struct Vector {
  Vector() : x(0), y(0), z(0) {}
  Vector(real xx, real yy, real zz) : x(xx), y(yy), z(zz) {}
  ~Vector() {}

  const real &operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: return x; // TODO ERROR HANDLING
    }
  }
  real &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: return x; // TODO ERROR HANDLING
    }
  }

  const Vector &operator+=(const Vector &a)
  {
    x += a.x;
    y += a.y;
    z += a.z;
    return *this;
  }
  const Vector &operator-=(const Vector &a)
  {
    x -= a.x;
    y -= a.y;
    z -= a.z;
    return *this;
  }
  const Vector &operator*=(real scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  double x, y, z;
};

inline Vector operator+(const Vector &a, const Vector &b)
{
  return Vector(
    a.x + b.x,
    a.y + b.y,
    a.z + b.z);
}

inline Vector operator-(const Vector &a, const Vector &b)
{
  return Vector(
    a.x - b.x,
    a.y - b.y,
    a.z - b.z);
}

inline Vector operator*(const Vector &a, real scalar)
{
  return Vector(
    a.x * scalar,
    a.y * scalar,
    a.z * scalar);
}

inline Vector operator*(real scalar, const Vector &a)
{
  return a * scalar;
}

inline Vector operator/(const Vector &a, real scalar)
{
  // no checking zero division
  const real inv = 1./scalar;
  return Vector(
    a.x * inv,
    a.y * inv,
    a.z * inv);
}

inline Vector operator-(const Vector &a)
{
  return Vector(
    -a.x,
    -a.y,
    -a.z);
}

inline real Length(const Vector &a)
{
  return std::sqrt(
    a.x * a.x +
    a.y * a.y +
    a.z * a.z);
}

inline Vector Normalize(const Vector &a)
{
  const real len = Length(a);
  if (len == 0)
    return a;
  return a / len;
}

inline real Dot(const Vector &a, const Vector &b)
{
  return
    a.x * b.x +
    a.y * b.y +
    a.z * b.z;
}

inline Vector Cross(const Vector &a, const Vector &b)
{
  return Vector(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x);
}

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

} // namespace xxx

#endif /* FJ_XXX_H */
