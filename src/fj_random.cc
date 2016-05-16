// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_random.h"
#include "fj_vector.h"
#include <limits.h>

namespace fj {

XorShift::XorShift()
{
  state[0] = 123456789;
  state[1] = 362436069;
  state[2] = 521288629;
  state[3] = 88675123;
}

XorShift::XorShift(unsigned int seed)
    : state()
{
  for (unsigned int i = 0; i < 4; i++) {
    state[i] = seed = 1812433253U * (seed^(seed>>30)) + i;
  }
}

uint32_t XorShift::NextInteger()
{
  uint32_t *st = state;
  uint32_t t;

  t = (st[0]^(st[0]<<11));
  st[0] = st[1];
  st[1] = st[2];
  st[2] = st[3];
  st[3] = (st[3]^(st[3]>>19))^(t^(t>>8));

  return st[3];
}

double XorShift::NextFloat01()
{
  return static_cast<double>(NextInteger()) / UINT32_MAX;
}

Vector XorShift::NextVector01()
{
  Vector v;
  v[0] = NextFloat01();
  v[1] = NextFloat01();
  v[2] = NextFloat01();
  return v;
}

Vector XorShift::SolidSphereRand()
{
  Vector out;
  for (;;) {
    out[0] = 2 * NextFloat01() - 1;
    out[1] = 2 * NextFloat01() - 1;
    out[2] = 2 * NextFloat01() - 1;

    if (Dot(out, out) <= 1) {
      break;
    }
  }
  return out;
}

Vector XorShift::HollowSphereRand()
{
  double dot = 0;
  Vector out;

  for (;;) {
    out[0] = 2 * NextFloat01() - 1;
    out[1] = 2 * NextFloat01() - 1;
    out[2] = 2 * NextFloat01() - 1;

    dot = Dot(out, out);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  return out / sqrt(dot);
}

Vector XorShift::SolidCubeRand()
{
  Vector out;

  out[0] = 2 * NextFloat01() - 1;
  out[1] = 2 * NextFloat01() - 1;
  out[2] = 2 * NextFloat01() - 1;

  return out;
}

Vector2 XorShift::SolidDiskRand()
{
  Vector2 out;

  for (;;) {
    out[0] = 2 * NextFloat01() - 1;
    out[1] = 2 * NextFloat01() - 1;

    if (Dot(out, out) <= 1) {
      break;
    }
  }

  return out;
}

Vector2 XorShift::HollowDiskRand()
{
  double dot = 0;
  Vector2 out;

  for (;;) {
    out[0] = 2 * NextFloat01() - 1;
    out[1] = 2 * NextFloat01() - 1;

    dot = Dot(out, out);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  return out / sqrt(dot);
}

Vector2 XorShift::GaussianDiskRand()
{
  const double gauss = GaussianRand();

  Vector2 out = HollowDiskRand();
  return out * gauss;
}

double XorShift::GaussianRand()
{
  Vector2 P;
  double dot = 0;

  for (;;) {
    P[0] = 2 * NextFloat01() - 1;
    P[1] = 2 * NextFloat01() - 1;

    dot = Dot(P, P);
    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  return P[0] * sqrt(-2 * log(dot) / dot);
}

} // namespace xxx
