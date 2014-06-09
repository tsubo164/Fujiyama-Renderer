/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VECTOR_H
#define FJ_VECTOR_H

#include "fj_types.h"
#include <cmath>

namespace fj {

class Vector2 {
public:
  Vector2() : x(0), y(0) {}
  Vector2(Real xx, Real yy) : x(xx), y(yy) {}
  ~Vector2() {}

  const Real &operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default: return x; // TODO ERROR HANDLING
    }
  }
  Real &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default: return x; // TODO ERROR HANDLING
    }
  }

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
  const Vector &operator*=(const Vector &a)
  {
    x *= a.x;
    y *= a.y;
    z *= a.z;
    return *this;
  }
  const Vector &operator/=(const Vector &a)
  {
    x /= a.x;
    y /= a.y;
    z /= a.z;
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

  Real x, y, z;
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

inline Vector operator*(const Vector &a, const Vector &b)
{
  return Vector(
    a.x * b.x,
    a.y * b.y,
    a.z * b.z);
}

inline Vector operator/(const Vector &a, const Vector &b)
{
  return Vector(
    a.x / b.x,
    a.y / b.y,
    a.z / b.z);
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

inline const Vector &Normalize(Vector *a)
{
  const Real len = Length(*a);
  if (len == 0)
    return *a;
  return *a /= len;
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

inline Vector LerpVec3(const Vector &a, const Vector &b, Real t)
{
  return (1 - t) * a + t * b;
}

extern struct Vector *VecAlloc(long count);
extern struct Vector *VecRealloc(struct Vector *v, long count);
extern void VecFree(struct Vector *v);

extern void VecPrint(const struct Vector *a);

/* VEC2 */
#define VEC2_DOT(a,b) ((a)->x * (b)->x + (a)->y * (b)->y)

} // namespace xxx

#endif /* FJ_XXX_H */
