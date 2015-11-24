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

uint32_t XorNextInteger(XorShift *xr)
{
  uint32_t *st = xr->state;
  uint32_t t;

  t = (st[0]^(st[0]<<11));
  st[0] = st[1];
  st[1] = st[2];
  st[2] = st[3];
  st[3] = (st[3]^(st[3]>>19))^(t^(t>>8));

  return st[3];
}

double XorNextFloat01(XorShift *xr)
{
  return (double) XorNextInteger(xr) / UINT32_MAX;
}

void XorSolidSphereRand(XorShift *xr, Vector *out_position)
{
  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;
    out_position->z = 2 * XorNextFloat01(xr) - 1;

    if (Dot(*out_position, *out_position) <= 1) {
      break;
    }
  }
}

void XorHollowSphereRand(XorShift *xr, Vector *out_position)
{
  double dot = 0;
  double len_inv = 0;

  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;
    out_position->z = 2 * XorNextFloat01(xr) - 1;

    dot = Dot(*out_position, *out_position);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  len_inv = 1. / sqrt(dot);
  out_position->x *= len_inv;
  out_position->y *= len_inv;
  out_position->z *= len_inv;
}

void XorSolidCubeRand(XorShift *xr, Vector *out_position)
{
  out_position->x = 2 * XorNextFloat01(xr) - 1;
  out_position->y = 2 * XorNextFloat01(xr) - 1;
  out_position->z = 2 * XorNextFloat01(xr) - 1;
}

void XorSolidDiskRand(XorShift *xr, Vector2 *out_position)
{
  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;

    if (Dot(*out_position, *out_position) <= 1) {
      break;
    }
  }
}

void XorHollowDiskRand(XorShift *xr, Vector2 *out_position)
{
  double dot = 0;
  double len_inv = 0;

  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;

    dot = Dot(*out_position, *out_position);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  len_inv = 1. / sqrt(dot);
  out_position->x *= len_inv;
  out_position->y *= len_inv;
}

void XorGaussianDiskRand(XorShift *xr, Vector2 *out_position)
{
  const double gauss = XorGaussianRand(xr);

  XorHollowDiskRand(xr, out_position);
  out_position->x *= gauss;
  out_position->y *= gauss;
}

double XorGaussianRand(XorShift *xr)
{
  Vector2 P(0, 0);
  double dot = 0;

  for (;;) {
    P.x = 2 * XorNextFloat01(xr) - 1;
    P.y = 2 * XorNextFloat01(xr) - 1;

    dot = Dot(P, P);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

    return P.x * sqrt(-2 * log(dot) / dot);
}

} // namespace xxx
