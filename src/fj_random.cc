/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_random.h"
#include "fj_vector.h"
#include <limits.h>

void XorInit(struct XorShift *xr)
{
  xr->state[0] = 123456789;
  xr->state[1] = 362436069;
  xr->state[2] = 521288629;
  xr->state[3] = 88675123;
}

uint32_t XorNextInteger(struct XorShift *xr)
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

double XorNextFloat01(struct XorShift *xr)
{
  return (double) XorNextInteger(xr) / UINT32_MAX;
}

void XorSolidSphereRand(struct XorShift *xr, struct Vector *out_position)
{
  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;
    out_position->z = 2 * XorNextFloat01(xr) - 1;

    if (VEC3_DOT(out_position, out_position) <= 1) {
      break;
    }
  }
}

void XorHollowSphereRand(struct XorShift *xr, struct Vector *out_position)
{
  double dot = 0;
  double len_inv = 0;

  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;
    out_position->z = 2 * XorNextFloat01(xr) - 1;

    dot = VEC3_DOT(out_position, out_position);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  len_inv = 1. / sqrt(dot);
  out_position->x *= len_inv;
  out_position->y *= len_inv;
  out_position->z *= len_inv;
}

void XorSolidCubeRand(struct XorShift *xr, struct Vector *out_position)
{
  out_position->x = 2 * XorNextFloat01(xr) - 1;
  out_position->y = 2 * XorNextFloat01(xr) - 1;
  out_position->z = 2 * XorNextFloat01(xr) - 1;
}

void XorSolidDiskRand(struct XorShift *xr, struct Vector2 *out_position)
{
  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;

    if (VEC2_DOT(out_position, out_position) <= 1) {
      break;
    }
  }
}

void XorHollowDiskRand(struct XorShift *xr, struct Vector2 *out_position)
{
  double dot = 0;
  double len_inv = 0;

  for (;;) {
    out_position->x = 2 * XorNextFloat01(xr) - 1;
    out_position->y = 2 * XorNextFloat01(xr) - 1;

    dot = VEC2_DOT(out_position, out_position);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

  len_inv = 1. / sqrt(dot);
  out_position->x *= len_inv;
  out_position->y *= len_inv;
}

void XorGaussianDiskRand(struct XorShift *xr, struct Vector2 *out_position)
{
  const double gauss = XorGaussianRand(xr);

  XorHollowDiskRand(xr, out_position);
  out_position->x *= gauss;
  out_position->y *= gauss;
}

double XorGaussianRand(struct XorShift *xr)
{
  struct Vector2 P = {0, 0};
  double dot = 0;

  for (;;) {
    P.x = 2 * XorNextFloat01(xr) - 1;
    P.y = 2 * XorNextFloat01(xr) - 1;

    dot = VEC2_DOT(&P, &P);

    if (dot > 0 && dot <= 1) {
      break;
    }
  }

    return P.x * sqrt(-2 * log(dot) / dot);
}

