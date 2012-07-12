/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Random.h"
#include "Vector.h"
#include <limits.h>

void XorInit(struct XorShift *xr)
{
	xr->state[0] = 123456789;
	xr->state[1] = 362436069;
	xr->state[2] = 521288629;
	xr->state[3] = 88675123;
}

unsigned long XorNextInteger(struct XorShift *xr)
{
	unsigned long *st = xr->state;
	unsigned long t;

	t = (st[0]^(st[0]<<11));
	st[0] = st[1];
	st[1] = st[2];
	st[2] = st[3];
	st[3] = (st[3]^(st[3]>>19))^(t^(t>>8));

	return st[3];
}

double XorNextFloat01(struct XorShift *xr)
{
	return (double) XorNextInteger(xr) / ULONG_MAX;
}

void XorSolidSphereRand(struct XorShift *xr, double *out_position)
{
	for (;;) {
		out_position[0] = 2 * XorNextFloat01(xr) - 1;
		out_position[1] = 2 * XorNextFloat01(xr) - 1;
		out_position[2] = 2 * XorNextFloat01(xr) - 1;

		if (VEC3_DOT(out_position, out_position) <= 1) {
			break;
		}
	}
}

void XorHollowSphereRand(struct XorShift *xr, double *out_position)
{
	double dot = 0;
	double len_inv = 0;

	for (;;) {
		out_position[0] = 2 * XorNextFloat01(xr) - 1;
		out_position[1] = 2 * XorNextFloat01(xr) - 1;
		out_position[2] = 2 * XorNextFloat01(xr) - 1;

		dot = VEC3_DOT(out_position, out_position);

		if (dot > 0 && dot <= 1) {
			break;
		}
	}

	len_inv = 1. / sqrt(dot);
	out_position[0] *= len_inv;
	out_position[1] *= len_inv;
	out_position[2] *= len_inv;
}

void XorSolidDiskRand(struct XorShift *xr, double *out_position)
{
	for (;;) {
		out_position[0] = 2 * XorNextFloat01(xr) - 1;
		out_position[1] = 2 * XorNextFloat01(xr) - 1;

		if (VEC2_DOT(out_position, out_position) <= 1) {
			break;
		}
	}
}

void XorSolidCubeRand(struct XorShift *xr, double *out_position)
{
	out_position[0] = 2 * XorNextFloat01(xr) - 1;
	out_position[1] = 2 * XorNextFloat01(xr) - 1;
	out_position[2] = 2 * XorNextFloat01(xr) - 1;
}

void XorHollowDiskRand(struct XorShift *xr, double *out_position)
{
	double dot = 0;
	double len_inv = 0;

	for (;;) {
		out_position[0] = 2 * XorNextFloat01(xr) - 1;
		out_position[1] = 2 * XorNextFloat01(xr) - 1;

		dot = VEC2_DOT(out_position, out_position);

		if (dot > 0 && dot <= 1) {
			break;
		}
	}

	len_inv = 1. / sqrt(dot);
	out_position[0] *= len_inv;
	out_position[1] *= len_inv;
}

void XorGaussianDiskRand(struct XorShift *xr, double *out_position)
{
	const double gauss = XorGaussianRand(xr);

	XorHollowDiskRand(xr, out_position);
	out_position[0] *= gauss;
	out_position[1] *= gauss;
}

double XorGaussianRand(struct XorShift *xr)
{
	double dot = 0;
	double P[2] = {0};

	for (;;) {
		P[0] = 2 * XorNextFloat01(xr) - 1;
		P[1] = 2 * XorNextFloat01(xr) - 1;

		dot = VEC2_DOT(P, P);

		if (dot > 0 && dot <= 1) {
			break;
		}
	}

    return P[0] * sqrt(-2 * log(dot) / dot);
}

