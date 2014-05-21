/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_COLOR_H
#define FJ_COLOR_H

namespace fj {

struct Color {
  Color() : r(0), g(0), b(0) {}
  Color(float rr, float gg, float bb) : r(rr), g(gg), b(bb) {}
  ~Color() {}

  const float &operator[](int i) const
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    default: return r; // TODO ERROR HANDLING
    }
  }
  float &operator[](int i)
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    default: return r; // TODO ERROR HANDLING
    }
  }

  const Color &operator+=(const Color &a)
  {
    r += a.r;
    g += a.g;
    b += a.b;
    return *this;
  }
  const Color &operator-=(const Color &a)
  {
    r -= a.r;
    g -= a.g;
    b -= a.b;
    return *this;
  }
  const Color &operator*=(float scalar)
  {
    r *= scalar;
    g *= scalar;
    b *= scalar;
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

inline Color operator+(const Color &a, const Color &b)
{
  return Color(
    a.r + b.r,
    a.g + b.g,
    a.b + b.b);
}

inline Color operator-(const Color &a, const Color &b)
{
  return Color(
    a.r - b.r,
    a.g - b.g,
    a.b - b.b);
}

inline Color operator*(const Color &a, float scalar)
{
  return Color(
    a.r * scalar,
    a.g * scalar,
    a.b * scalar);
}

inline Color operator*(float scalar, const Color &a)
{
  return a * scalar;
}

inline Color operator/(const Color &a, float scalar)
{
  // no checking zero division
  const float inv = 1.f/scalar;
  return Color(
    a.r * inv,
    a.g * inv,
    a.b * inv);
}

inline Color operator-(const Color &a)
{
  return Color(
    -a.r,
    -a.g,
    -a.b);
}

struct Color4 {
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
    default: return r; // TODO ERROR HANDLING
    }
  }
  float &operator[](int i)
  {
    switch(i) {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    case 3: return a;
    default: return r; // TODO ERROR HANDLING
    }
  }

  const Color4 &operator+=(const Color4 &A)
  {
    r += A.r;
    g += A.g;
    b += A.b;
    a += A.a;
    return *this;
  }
  const Color4 &operator-=(const Color4 &A)
  {
    r -= A.r;
    g -= A.g;
    b -= A.b;
    a -= A.a;
    return *this;
  }
  const Color4 &operator*=(float scalar)
  {
    r *= scalar;
    g *= scalar;
    b *= scalar;
    a *= scalar;
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
    A.r + B.r,
    A.g + B.g,
    A.b + B.b,
    A.a + B.a);
}

inline Color4 operator-(const Color4 &A, const Color4 &B)
{
  return Color4(
    A.r - B.r,
    A.g - B.g,
    A.b - B.b,
    A.a - B.a);
}

inline Color4 operator*(const Color4 &A, float scalar)
{
  return Color4(
    A.r * scalar,
    A.g * scalar,
    A.b * scalar,
    A.a * scalar);
}

inline Color4 operator*(float scalar, const Color4 &A)
{
  return A * scalar;
}

inline Color4 operator/(const Color4 &A, float scalar)
{
  // no checking zero division
  const float inv = 1.f/scalar;
  return Color4(
    A.r * inv,
    A.g * inv,
    A.b * inv,
    A.a * inv);
}

inline Color4 operator-(const Color4 &A)
{
  return Color4(
    -A.r,
    -A.g,
    -A.b,
    -A.a);
}

inline float Luminance(const Color &A)
{
  return .298912 * A.r + .586611 * A.g + .114478 * A.b;
}

inline float Luminance4(const Color4 &A)
{
  return .298912 * A.r + .586611 * A.g + .114478 * A.b;
}

inline Color ColLerp(const Color &A, const Color &B, float t)
{
  return (1 - t) * A + t * B;
}

extern struct Color *ColAlloc(long count);
extern struct Color *ColRealloc(struct Color *c, long count);
extern void ColFree(struct Color *c);

} // namespace xxx

#endif /* FJ_XXX_H */
