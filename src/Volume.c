/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Volume.h"
#include "Box.h"
#include "Ray.h"
#include <stdlib.h>
#include <float.h>
#include <stdio.h>

struct VoxelBuffer {
	float *data;
	int size[3];
};

struct Volume {
	double bounds[6];
};

struct Volume *VolNew(void)
{
	struct Volume *volume;

	volume = (struct Volume *) malloc(sizeof(struct Volume));
	if (volume == NULL)
		return NULL;

	BOX3_SET(volume->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	/* TODO TEST bounds and constant filling */
	BOX3_SET(volume->bounds, -1, -1, -1, 1, 1, 1);

	return volume;
}

void VolFree(struct Volume *volume)
{
	if (volume == NULL)
		return;
	free(volume);
}

void VolGetBounds(const struct Volume *volume, double *bounds)
{
	BOX3_COPY(bounds, volume->bounds);
}

int VolGetSample(const struct Volume *volume, const double *point,
			struct VolumeSample *sample)
{
	int hit;

	hit = BoxContainsPoint(volume->bounds, point);
	if (hit) {
		sample->density = 1;
	}

	return hit;
}

