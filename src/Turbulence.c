/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Turbulence.h"
#include "Vector.h"
#include "Noise.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct Turbulence {
	double amplitude[3];
	double frequency[3];
	double offset[3];
	double lacunarity;
	double gain;
	int octaves;
};

struct Turbulence *TrbNew(void)
{
	struct Turbulence *turbulence;

	turbulence = (struct Turbulence *) malloc(sizeof(struct Turbulence));
	if (turbulence == NULL)
		return NULL;

	VEC3_SET(turbulence->amplitude, 1, 1, 1);
	VEC3_SET(turbulence->frequency, 1, 1, 1);
	VEC3_SET(turbulence->offset, 0, 0, 0);
	turbulence->lacunarity = 2;
	turbulence->gain = .5;
	turbulence->octaves = 8;

	return turbulence;
}

void TrbFree(struct Turbulence *turbulence)
{
	if (turbulence == NULL)
		return;
	free(turbulence);
}

void TrbSetAmplitude(struct Turbulence *turbulence, double x, double y, double z)
{
	VEC3_SET(turbulence->amplitude, x, y, z);
}

void TrbSetFrequency(struct Turbulence *turbulence, double x, double y, double z)
{
	VEC3_SET(turbulence->frequency, x, y, z);
}

void TrbSetOffset(struct Turbulence *turbulence, double x, double y, double z)
{
	VEC3_SET(turbulence->offset, x, y, z);
}

void TrbSetLacunarity(struct Turbulence *turbulence, double lacunarity)
{
	turbulence->lacunarity = lacunarity;
}

void TrbSetGain(struct Turbulence *turbulence, double gain)
{
	turbulence->gain = gain;
}

void TrbSetOctaves(struct Turbulence *turbulence, int octaves)
{
	assert(octaves > 0);
	turbulence->octaves = octaves;
}

double TrbEvaluate(const struct Turbulence *turbulence, double *position)
{
	double P[3] = {0};
	double noise_value = 0;

	P[0] = position[0] * turbulence->frequency[0] + turbulence->offset[0];
	P[1] = position[1] * turbulence->frequency[1] + turbulence->offset[1];
	P[2] = position[2] * turbulence->frequency[2] + turbulence->offset[2];

	noise_value = PerlinNoise(P,
			turbulence->lacunarity,
			turbulence->gain,
			turbulence->octaves);

	return noise_value * turbulence->amplitude[0];
}

void TrbEvaluate3d(const struct Turbulence *turbulence, double *position, double *out_noise)
{
	double P[3] = {0};

	P[0] = position[0] * turbulence->frequency[0] + turbulence->offset[0];
	P[1] = position[1] * turbulence->frequency[1] + turbulence->offset[1];
	P[2] = position[2] * turbulence->frequency[2] + turbulence->offset[2];

	PerlinNoise3d(P,
			turbulence->lacunarity,
			turbulence->gain,
			turbulence->octaves,
			out_noise);

	out_noise[0] *= turbulence->amplitude[0];
	out_noise[1] *= turbulence->amplitude[1];
	out_noise[2] *= turbulence->amplitude[2];
}

