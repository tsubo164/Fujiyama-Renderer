/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VECTOR_H
#define FJ_VECTOR_H

#include "fj_types.h"
#include <cstdlib>
#include <cmath>

namespace fj {

struct Vector2 {
  Vector2() : x(0), y(0) {}
  Vector2(Real xx, Real yy) : x(xx), y(yy) {}
  ~Vector2() {}

  Real x, y;
};

struct Vector {
  Vector() : x(0), y(0), z(0) {}
  Vector(Real xx, Real yy, Real zz) : x(xx), y(yy), z(zz) {}
  ~Vector() {}

  const Real &operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: return x; // TODO ERROR HANDLING
    }
  }
  Real &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: return x; // TODO ERROR HANDLING
    }
  }

#if 0
  const Vector &operator+=(const Vector &a)
  {
    (*this)[0] += a[0];
    (*this)[1] += a[1];
    (*this)[2] += a[2];
    return *this;
  }
  const Vector &operator-=(const Vector &a)
  {
    (*this)[0] -= a[0];
    (*this)[1] -= a[1];
    (*this)[2] -= a[2];
    return *this;
  }
  const Vector &operator*=(Real scalar)
  {
    (*this)[0] *= scalar;
    (*this)[1] *= scalar;
    (*this)[2] *= scalar;
    return *this;
  }
  const Vector &operator/=(Real scalar)
  {
    // no checking zero division
    const Real inv = 1./scalar;
    return *this *= inv;
  }
#endif
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
  const Vector &operator*=(Real scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }
  const Vector &operator/=(Real scalar)
  {
    // no checking zero division
    const Real inv = 1./scalar;
    return *this *= inv;
  }

  double x, y, z;
};

inline Vector operator+(const Vector &a, const Vector &b)
{
#if 0
  return Vector(
    a[0] + b[0],
    a[1] + b[1],
    a[2] + b[2]);
#endif
  return Vector(
    a.x + b.x,
    a.y + b.y,
    a.z + b.z);
}

inline Vector operator-(const Vector &a, const Vector &b)
{
#if 0
  return Vector(
    a[0] - b[0],
    a[1] - b[1],
    a[2] - b[2]);
#endif
  return Vector(
    a.x - b.x,
    a.y - b.y,
    a.z - b.z);
}

inline Vector operator*(const Vector &a, Real scalar)
{
  return Vector(
    a.x * scalar,
    a.y * scalar,
    a.z * scalar);
}

inline Vector operator*(Real scalar, const Vector &a)
{
  return a * scalar;
}

inline Vector operator/(const Vector &a, Real scalar)
{
  // no checking zero division
  const Real inv = 1./scalar;
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

inline Real Length(const Vector &a)
{
  return std::sqrt(
    a.x * a.x +
    a.y * a.y +
    a.z * a.z);
}

inline Vector Normalize(const Vector &a)
{
  const Real len = Length(a);
  if (len == 0)
    return a;
  return a / len;
}

inline Real Dot(const Vector &a, const Vector &b)
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
