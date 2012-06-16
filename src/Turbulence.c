/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Turbulence.h"
#include "Vector.h"
#include <stdlib.h>
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
	struct Turbulence *turb;

	turb = (struct Turbulence *) malloc(sizeof(struct Turbulence));
	if (turb == NULL)
		return NULL;

	VEC3_SET(turb->amplitude, 1, 1, 1);
	VEC3_SET(turb->frequency, 1, 1, 1);
	VEC3_SET(turb->offset, 0, 0, 0);
	turb->lacunarity = 2;
	turb->gain = .5;
	turb->octaves = 8;

	return turb;
}

void TrbFree(struct Turbulence *turb)
{
	if (turb == NULL)
		return;
	free(turb);
}

void TrbSetAmplitude(struct Turbulence *turb, double x, double y, double z)
{
	VEC3_SET(turb->amplitude, x, y, z);
}

void TrbSetFrequency(struct Turbulence *turb, double x, double y, double z)
{
	VEC3_SET(turb->frequency, x, y, z);
}

void TrbSetOffset(struct Turbulence *turb, double x, double y, double z)
{
	VEC3_SET(turb->offset, x, y, z);
}

void TrbSetLacunarity(struct Turbulence *turb, double lacunarity)
{
	turb->lacunarity = lacunarity;
}

void TrbSetGain(struct Turbulence *turb, double gain)
{
	turb->gain = gain;
}

void TrbSetOctaves(struct Turbulence *turb, int octaves)
{
	assert(octaves > 0);
	turb->octaves = octaves;
}

