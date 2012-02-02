/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Noise.h"
#include "Vector.h"
#include "Numeric.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

static double smoothstep(double x, double a, double b);
static double rand1(double x);

static double rand3(double x, double y, double z);
static double noise3(double *position);

extern void PerlinNoise(const double *position, const double *amplitude,
		const double *frequency, const double *offset,
		double lacunarity, double persistence, int octaves, double *result)
{
	double P[3];
	double amp[3];
	double freq[3];
	double n;
	int i;

	P[0] = position[0] + offset[0];
	P[1] = position[1] + offset[1];
	P[2] = position[2] + offset[2];
	P[0] *= frequency[0];
	P[1] *= frequency[1];
	P[2] *= frequency[2];
	/*
	*/

	VEC3_COPY(amp, amplitude);
	VEC3_COPY(freq, frequency);

	n = 0;
	for (i = 0; i < octaves; i++) {
		n += amp[0] * noise3(P);

		VEC3_MUL_ASGN(amp, persistence);
		VEC3_MUL_ASGN(P, lacunarity);
	}

	result[0] = n;
	result[1] = n;
	result[2] = n;
}

static double rand1(double x)
{
	srand(((unsigned int)x)%UINT_MAX);
	return ((double) rand()) / ((double) RAND_MAX);
}

static double smoothstep(double x, double a, double b)
{
	const double t = (x-a) / (b-a);

	if (t <= 0)
		return 0;

	if (t >= 1)
		return 1;

	return t*t*(3 - 2*t);
}

static double rand3(double x, double y, double z)
{
	return rand1(x + 123.456*y + 987.123*z);
}

static double noise3(double *position)
{
	double result;

	int ix, iy, iz;
	double biasx, biasy, biasz;

	ix = floor(position[0]);
	iy = floor(position[1]);
	iz = floor(position[2]);

	biasx = smoothstep(position[0], ix, ix+1);
	biasy = smoothstep(position[1], iy, iy+1);
	biasz = smoothstep(position[2], iz, iz+1);

	{
		int i, j, k;
		double NZ[2];

		for (k = 0; k <= 1; k++) {
			double NY[2];

			for (j = 0; j <= 1; j++) {
				double NX[2];

				for (i = 0; i <= 1; i++) {
					NX[i] = rand3(ix+i, iy+j, iz+k);
				}
				NY[j] = LERP(biasx, NX[0], NX[1]);
			}
			NZ[k] = LERP(biasy, NY[0], NY[1]);
		}
		result = LERP(biasz, NZ[0], NZ[1]);
	}
	return result;
}

