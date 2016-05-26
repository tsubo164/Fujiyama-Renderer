// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_VECTOR_H
#define FJ_VECTOR_H

#include "fj_types.h"
#include "fj_numeric.h"
#include <iostream>
#include <cassert>
#include <cmath>

namespace fj {

class Vector2 {
public:
  Vector2()
    : x(0), y(0) {}
  Vector2(Real xx, Real yy)
    : x(xx), y(yy) {}
  Vector2(const Vector2 &a)
    : x(a[0]), y(a[1]) {}
  ~Vector2() {}

  Real operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default:
      assert(!"bounds error at Vector2::get");
      return x;
    }
  }
  Real &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default:
      assert(!"bounds error at Vector2::set");
      return x;
    }
  }

  const Vector2 &operator=(const Vector2 &a)
  {
    (*this)[0] = a[0];
    (*this)[1] = a[1];
    return *this;
  }
  const Vector2 &operator+=(const Vector2 &a)
  {
    (*this)[0] += a[0];
    (*this)[1] += a[1];
    return *this;
  }
  const Vector2 &operator-=(const Vector2 &a)
  {
    (*this)[0] -= a[0];
    (*this)[1] -= a[1];
    return *this;
  }
  const Vector2 &operator*=(const Vector2 &a)
  {
    (*this)[0] *= a[0];
    (*this)[1] *= a[1];
    return *this;
  }
  const Vector2 &operator/=(const Vector2 &a)
  {
    (*this)[0] /= a[0];
    (*this)[1] /= a[1];
    return *this;
  }
  const Vector2 &operator*=(Real scalar)
  {
    (*this)[0] *= scalar;
    (*this)[1] *= scalar;
    return *this;
  }
  const Vector2 &operator/=(Real scalar)
  {
    // no checking zero division
    const Real inv = 1./scalar;
    return *this *= inv;
  }

  Real x, y;
};

inline Vector2 operator+(const Vector2 &a, const Vector2 &b)
{
  return Vector2(
    a[0] + b[0],
    a[1] + b[1]);
}

inline Vector2 operator-(const Vector2 &a, const Vector2 &b)
{
  return Vector2(
    a[0] - b[0],
    a[1] - b[1]);
}

inline Vector2 operator*(const Vector2 &a, const Vector2 &b)
{
  return Vector2(
    a[0] * b[0],
    a[1] * b[1]);
}

inline Vector2 operator/(const Vector2 &a, const Vector2 &b)
{
  return Vector2(
    a[0] / b[0],
    a[1] / b[1]);
}

inline Vector2 operator*(const Vector2 &a, Real scalar)
{
  return Vector2(
    a[0] * scalar,
    a[1] * scalar);
}

inline Vector2 operator*(Real scalar, const Vector2 &a)
{
  return a * scalar;
}

inline Vector2 operator/(const Vector2 &a, Real scalar)
{
  // no checking zero division
  const Real inv = 1./scalar;
  return a * inv;
}

inline Vector2 operator-(const Vector2 &a)
{
  return -1 * a;
}

inline Real Dot(const Vector2 &a, const Vector2 &b)
{
  return
    a[0] * b[0] +
    a[1] * b[1];
}

inline Real Length(const Vector2 &a)
{
  return std::sqrt(Dot(a, a));
}

inline const Vector2 &Normalize(Vector2 *a)
{
  const Real len = Length(*a);
  if (len == 0)
    return *a;
  return *a /= len;
}

inline Vector2 GetNormalized(const Vector2 &a)
{
  Vector2 b(a);
  return Normalize(&b);
}

inline Vector2 Lerp(const Vector2 &a, const Vector2 &b, Real t)
{
  return (1 - t) * a + t * b;
}

inline std::ostream &operator<<(std::ostream &os, const Vector2 &a)
{
  return os << "(" <<
    a[0] << ", " <<
    a[1] << ")";
}

class Vector {
public:
  Vector()
    : x(0), y(0), z(0) {}
  Vector(Real xx, Real yy, Real zz)
    : x(xx), y(yy), z(zz) {}
  Vector(const Vector &a)
    : x(a[0]), y(a[1]), z(a[2]) {}
  ~Vector() {}

  Real operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default:
      assert(!"bounds error at Vector::get");
      return x;
    }
  }
  Real &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default:
      assert(!"bounds error at Vector::set");
      return x;
    }
  }

  const Vector &operator=(const Vector &a)
  {
    (*this)[0] = a[0];
    (*this)[1] = a[1];
    (*this)[2] = a[2];
    return *this;
  }
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
  const Vector &operator*=(const Vector &a)
  {
    (*this)[0] *= a[0];
    (*this)[1] *= a[1];
    (*this)[2] *= a[2];
    return *this;
  }
  const Vector &operator/=(const Vector &a)
  {
    (*this)[0] /= a[0];
    (*this)[1] /= a[1];
    (*this)[2] /= a[2];
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

  Real x, y, z;
};

inline Vector operator+(const Vector &a, const Vector &b)
{
  return Vector(
    a[0] + b[0],
    a[1] + b[1],
    a[2] + b[2]);
}

inline Vector operator-(const Vector &a, const Vector &b)
{
  return Vector(
    a[0] - b[0],
    a[1] - b[1],
    a[2] - b[2]);
}

inline Vector operator*(const Vector &a, const Vector &b)
{
  return Vector(
    a[0] * b[0],
    a[1] * b[1],
    a[2] * b[2]);
}

inline Vector operator/(const Vector &a, const Vector &b)
{
  return Vector(
    a[0] / b[0],
    a[1] / b[1],
    a[2] / b[2]);
}

inline Vector operator*(const Vector &a, Real scalar)
{
  return Vector(
    a[0] * scalar,
    a[1] * scalar,
    a[2] * scalar);
}

inline Vector operator*(Real scalar, const Vector &a)
{
  return a * scalar;
}

inline Vector operator/(const Vector &a, Real scalar)
{
  // no checking zero division
  const Real inv = 1./scalar;
  return a * inv;
}

inline Vector operator-(const Vector &a)
{
  return -1 * a;
}

inline Real Dot(const Vector &a, const Vector &b)
{
  return
    a[0] * b[0] +
    a[1] * b[1] +
    a[2] * b[2];
}

inline Vector Cross(const Vector &a, const Vector &b)
{
  return Vector(
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0]);
}

inline Real Length(const Vector &a)
{
  return std::sqrt(Dot(a, a));
}

inline const Vector &Normalize(Vector *a)
{
  const Real len = Length(*a);
  if (len == 0)
    return *a;
  return *a /= len;
}

inline Vector GetNormalized(const Vector &a)
{
  Vector b(a);
  return Normalize(&b);
}

inline Vector Lerp(const Vector &a, const Vector &b, Real t)
{
  return (1 - t) * a + t * b;
}

inline std::ostream &operator<<(std::ostream &os, const Vector &a)
{
  return os << "(" <<
    a[0] << ", " <<
    a[1] << ", " <<
    a[2] << ")";
}

class Vector4 {
public:
  Vector4()
    : x(0), y(0), z(0), w(0) {}
  Vector4(Real xx, Real yy, Real zz, Real ww)
    : x(xx), y(yy), z(zz), w(ww) {}
  Vector4(const Vector4 &a)
    : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {}
  ~Vector4() {}

  Real operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    case 3: return w;
    default:
      assert(!"bounds error at Vector4::get");
      return x;
    }
  }
  Real &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    case 3: return w;
    default:
      assert(!"bounds error at Vector4::set");
      return x;
    }
  }

  const Vector4 &operator=(const Vector4 &a)
  {
    (*this)[0] = a[0];
    (*this)[1] = a[1];
    (*this)[2] = a[2];
    (*this)[3] = a[3];
    return *this;
  }
  const Vector4 &operator+=(const Vector4 &a)
  {
    (*this)[0] += a[0];
    (*this)[1] += a[1];
    (*this)[2] += a[2];
    (*this)[3] += a[3];
    return *this;
  }
  const Vector4 &operator-=(const Vector4 &a)
  {
    (*this)[0] -= a[0];
    (*this)[1] -= a[1];
    (*this)[2] -= a[2];
    (*this)[3] -= a[3];
    return *this;
  }
  const Vector4 &operator*=(const Vector4 &a)
  {
    (*this)[0] *= a[0];
    (*this)[1] *= a[1];
    (*this)[2] *= a[2];
    (*this)[3] *= a[3];
    return *this;
  }
  const Vector4 &operator/=(const Vector4 &a)
  {
    (*this)[0] /= a[0];
    (*this)[1] /= a[1];
    (*this)[2] /= a[2];
    (*this)[3] /= a[3];
    return *this;
  }
  const Vector4 &operator*=(Real scalar)
  {
    (*this)[0] *= scalar;
    (*this)[1] *= scalar;
    (*this)[2] *= scalar;
    (*this)[3] *= scalar;
    return *this;
  }
  const Vector4 &operator/=(Real scalar)
  {
    // no checking zero division
    const Real inv = 1./scalar;
    return *this *= inv;
  }

  Real x, y, z, w;
};

inline Vector4 operator+(const Vector4 &a, const Vector4 &b)
{
  return Vector4(
    a[0] + b[0],
    a[1] + b[1],
    a[2] + b[2],
    a[3] + b[3]);
}

inline Vector4 operator-(const Vector4 &a, const Vector4 &b)
{
  return Vector4(
    a[0] - b[0],
    a[1] - b[1],
    a[2] - b[2],
    a[3] - b[3]);
}

inline Vector4 operator*(const Vector4 &a, const Vector4 &b)
{
  return Vector4(
    a[0] * b[0],
    a[1] * b[1],
    a[2] * b[2],
    a[3] * b[3]);
}

inline Vector4 operator/(const Vector4 &a, const Vector4 &b)
{
  return Vector4(
    a[0] / b[0],
    a[1] / b[1],
    a[2] / b[2],
    a[3] / b[3]);
}

inline Vector4 operator*(const Vector4 &a, Real scalar)
{
  return Vector4(
    a[0] * scalar,
    a[1] * scalar,
    a[2] * scalar,
    a[3] * scalar);
}

inline Vector4 operator*(Real scalar, const Vector4 &a)
{
  return a * scalar;
}

inline Vector4 operator/(const Vector4 &a, Real scalar)
{
  // no checking zero division
  const Real inv = 1./scalar;
  return a * inv;
}

inline Vector4 operator-(const Vector4 &a)
{
  return -1 * a;
}

inline Vector4 Lerp(const Vector4 &a, const Vector4 &b, Real t)
{
  return (1 - t) * a + t * b;
}

inline Vector4 Min(const Vector4 &a, const Vector4 &b)
{
  return Vector4(
      Min(a[0], b[0]),
      Min(a[1], b[1]),
      Min(a[2], b[2]),
      Min(a[3], b[3]));
}

inline Vector4 Max(const Vector4 &a, const Vector4 &b)
{
  return Vector4(
      Max(a[0], b[0]),
      Max(a[1], b[1]),
      Max(a[2], b[2]),
      Max(a[3], b[3]));
}

inline std::ostream &operator<<(std::ostream &os, const Vector4 &a)
{
  return os << "(" <<
    a[0] << ", " <<
    a[1] << ", " <<
    a[2] << ", " <<
    a[3] << ")";
}

class Int2 {
public:
  Int2()
    : x(0), y(0) {}
  Int2(int xx, int yy)
    : x(xx), y(yy) {}
  Int2(const Int2 &a)
    : x(a[0]), y(a[1]) {}
  ~Int2() {}

  int operator[](int i) const
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default:
      assert(!"bounds error at Int2::get");
      return x;
    }
  }
  int &operator[](int i)
  {
    switch(i) {
    case 0: return x;
    case 1: return y;
    default:
      assert(!"bounds error at Int2::set");
      return x;
    }
  }

  const Int2 &operator=(const Int2 &a)
  {
    (*this)[0] = a[0];
    (*this)[1] = a[1];
    return *this;
  }
  const Int2 &operator+=(const Int2 &a)
  {
    (*this)[0] += a[0];
    (*this)[1] += a[1];
    return *this;
  }
  const Int2 &operator-=(const Int2 &a)
  {
    (*this)[0] -= a[0];
    (*this)[1] -= a[1];
    return *this;
  }
  const Int2 &operator*=(const Int2 &a)
  {
    (*this)[0] *= a[0];
    (*this)[1] *= a[1];
    return *this;
  }
  const Int2 &operator/=(const Int2 &a)
  {
    (*this)[0] /= a[0];
    (*this)[1] /= a[1];
    return *this;
  }
  const Int2 &operator*=(int scalar)
  {
    (*this)[0] *= scalar;
    (*this)[1] *= scalar;
    return *this;
  }
  const Int2 &operator/=(int scalar)
  {
    // no checking zero division
    (*this)[0] /= scalar;
    (*this)[1] /= scalar;
    return *this;
  }

  int x, y;
};

inline Int2 operator+(const Int2 &a, const Int2 &b)
{
  return Int2(
    a[0] + b[0],
    a[1] + b[1]);
}

inline Int2 operator-(const Int2 &a, const Int2 &b)
{
  return Int2(
    a[0] - b[0],
    a[1] - b[1]);
}

inline Int2 operator*(const Int2 &a, const Int2 &b)
{
  return Int2(
    a[0] * b[0],
    a[1] * b[1]);
}

inline Int2 operator/(const Int2 &a, const Int2 &b)
{
  return Int2(
    a[0] / b[0],
    a[1] / b[1]);
}

inline Int2 operator*(const Int2 &a, int scalar)
{
  return Int2(
    a[0] * scalar,
    a[1] * scalar);
}

inline Int2 operator*(int scalar, const Int2 &a)
{
  return a * scalar;
}

inline Int2 operator/(const Int2 &a, int scalar)
{
  // no checking zero division
  return Int2(
    a[0] / scalar,
    a[1] / scalar);
}

inline Int2 operator-(const Int2 &a)
{
  return -1 * a;
}

inline std::ostream &operator<<(std::ostream &os, const Int2 &a)
{
  return os << "(" <<
    a[0] << ", " <<
    a[1] << ")";
}

} // namespace xxx

#endif // FJ_XXX_H
