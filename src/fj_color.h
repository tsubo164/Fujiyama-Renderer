// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_COLOR_H
#define FJ_COLOR_H

#include "fj_compatibility.h"
#include "fj_numeric.h"
#include <iostream>
#include <cassert>

namespace fj {

class FJ_API Color {
public:
  Color() : r(0), g(0), b(0) {}
  Color(float rr, float gg, float bb) : r(rr), g(gg), b(bb) {}
  ~Color() {}

  const float &operator[](int i) const
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    default:
      assert(!"bounds error at Color::get");
      return r;
    }
  }
  float &operator[](int i)
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    default:
      assert(!"bounds error at Color::get");
      return r;
    }
  }

  const Color &operator+=(const Color &A)
  {
    (*this)[0] += A[0];
    (*this)[1] += A[1];
    (*this)[2] += A[2];
    return *this;
  }
  const Color &operator-=(const Color &A)
  {
    (*this)[0] -= A[0];
    (*this)[1] -= A[1];
    (*this)[2] -= A[2];
    return *this;
  }
  const Color &operator*=(float scalar)
  {
    (*this)[0] *= scalar;
    (*this)[1] *= scalar;
    (*this)[2] *= scalar;
    return *this;
  }
  const Color &operator/=(float scalar)
  {
    // no checking zero division
    const float inv = 1.f/scalar;
    return *this *= inv;
  }

  float r, g, b;
};

inline Color operator+(const Color &A, const Color &B)
{
  return Color(
    A[0] + B[0],
    A[1] + B[1],
    A[2] + B[2]);
}

inline Color operator-(const Color &A, const Color &B)
{
  return Color(
    A[0] - B[0],
    A[1] - B[1],
    A[2] - B[2]);
}

inline Color operator*(const Color &A, const Color &B)
{
  return Color(
    A[0] * B[0],
    A[1] * B[1],
    A[2] * B[2]);
}

inline Color operator*(const Color &A, float scalar)
{
  return Color(
    A[0] * scalar,
    A[1] * scalar,
    A[2] * scalar);
}

inline Color operator*(float scalar, const Color &A)
{
  return A * scalar;
}

inline Color operator/(const Color &A, float scalar)
{
  // no checking zero division
  const float inv = 1.f/scalar;
  return A * inv;
}

inline Color operator-(const Color &A)
{
  return -1 * A;
}

inline std::ostream &operator<<(std::ostream &os, const Color &A)
{
  os << "(" <<
    A[0] << ", " <<
    A[1] << ", " <<
    A[2] << ")";
  return os;
}

class Color4 {
public:
  Color4() : r(0), g(0), b(0), a(0) {}
  Color4(float rr, float gg, float bb, float aa) : r(rr), g(gg), b(bb), a(aa) {}
  ~Color4() {}

  const float &operator[](int i) const
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    case 3: return a;
    default:
      assert(!"bounds error at Color4::get");
      return r;
    }
  }
  float &operator[](int i)
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    case 3: return a;
    default:
      assert(!"bounds error at Color4::get");
      return r;
    }
  }

  const Color4 &operator+=(const Color4 &A)
  {
    (*this)[0] += A[0];
    (*this)[1] += A[1];
    (*this)[2] += A[2];
    (*this)[3] += A[3];
    return *this;
  }
  const Color4 &operator-=(const Color4 &A)
  {
    (*this)[0] -= A[0];
    (*this)[1] -= A[1];
    (*this)[2] -= A[2];
    (*this)[3] -= A[3];
    return *this;
  }
  const Color4 &operator*=(float scalar)
  {
    (*this)[0] *= scalar;
    (*this)[1] *= scalar;
    (*this)[2] *= scalar;
    (*this)[3] *= scalar;
    return *this;
  }
  const Color4 &operator/=(float scalar)
  {
    // no checking zero division
    const float inv = 1.f/scalar;
    return *this *= inv;
  }

  float r, g, b, a;
};

inline Color4 operator+(const Color4 &A, const Color4 &B)
{
  return Color4(
    A[0] + B[0],
    A[1] + B[1],
    A[2] + B[2],
    A[3] + B[3]);
}

inline Color4 operator-(const Color4 &A, const Color4 &B)
{
  return Color4(
    A[0] - B[0],
    A[1] - B[1],
    A[2] - B[2],
    A[3] - B[3]);
}

inline Color4 operator*(const Color4 &A, const Color4 &B)
{
  return Color4(
    A[0] * B[0],
    A[1] * B[1],
    A[2] * B[2],
    A[3] * B[3]);
}

inline Color4 operator*(const Color4 &A, float scalar)
{
  return Color4(
    A[0] * scalar,
    A[1] * scalar,
    A[2] * scalar,
    A[3] * scalar);
}

inline Color4 operator*(float scalar, const Color4 &A)
{
  return A * scalar;
}

inline Color4 operator/(const Color4 &A, float scalar)
{
  // no checking zero division
  const float inv = 1.f/scalar;
  return A * inv;
}

inline Color4 operator-(const Color4 &A)
{
  return -1 * A;
}

inline std::ostream &operator<<(std::ostream &os, const Color4 &A)
{
  os << "(" <<
    A[0] << ", " <<
    A[1] << ", " <<
    A[2] << ", " <<
    A[3] << ")";
  return os;
}

inline float Luminance(const Color &A)
{
  return .298912 * A[0] + .586611 * A[1] + .114478 * A[2];
}

inline float Luminance4(const Color4 &A)
{
  return .298912 * A[0] + .586611 * A[1] + .114478 * A[2];
}

inline Color Lerp(const Color &A, const Color &B, float t)
{
  return (1 - t) * A + t * B;
}

inline Color Gamma(const Color &A, float gamma)
{
  return Color(
      Gamma(A[0], gamma),
      Gamma(A[1], gamma),
      Gamma(A[2], gamma));
}

inline Color ToColor(const Color4 &A)
{
  return Color(
      A[0],
      A[1],
      A[2]);
}

inline Color4 ToColor4(const Color &A, float alpha = 1.0)
{
  return Color4(
      A[0],
      A[1],
      A[2],
      alpha);
}

} // namespace xxx

#endif // FJ_XXX_H
