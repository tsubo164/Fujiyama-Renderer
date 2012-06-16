/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Volume.h"
#include "Numeric.h"
#include "Vector.h"
#include "Box.h"
#include <stdlib.h>
#include <float.h>
#include <stdio.h>

struct VoxelBuffer {
	float *data;
	int res[3];
};

struct Volume {
	struct VoxelBuffer *buffer;
	double bounds[6];
	double size[3];

	double filtersize;
};

static struct VoxelBuffer *new_voxel_buffer(void);
static void free_voxel_buffer(struct VoxelBuffer *buffer);
static void resize_voxel_buffer(struct VoxelBuffer *buffer, int xres, int yres, int zres);
static void fill_voxel_buffer(struct VoxelBuffer *buffer, float value);
static void compute_filter_size(struct Volume *volume);

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value);
static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z);

static float trilinear_buffer_value(const struct VoxelBuffer *buffer, const double *P);
static float nearest_buffer_value(const struct VoxelBuffer *buffer, const double *P);

struct Volume *VolNew(void)
{
	struct Volume *volume;

	volume = (struct Volume *) malloc(sizeof(struct Volume));
	if (volume == NULL)
		return NULL;

	volume->buffer = NULL;

	/* this makes empty volume valid */
	BOX3_SET(volume->bounds, 0, 0, 0, 0, 0, 0);
	VEC3_SET(volume->size, 0, 0, 0);

	compute_filter_size(volume);

	return volume;
}

void VolFree(struct Volume *volume)
{
	if (volume == NULL)
		return;

	free_voxel_buffer(volume->buffer);
	free(volume);
}

void VolResize(struct Volume *volume, int xres, int yres, int zres)
{
	if (xres < 1 || yres < 1 || zres < 1) {
		return;
	}

	if (volume->buffer == NULL) {
		volume->buffer = new_voxel_buffer();
		if (volume->buffer == NULL) {
			/* TODO error handling when new_voxel_buffer fails */
		}
	}

	resize_voxel_buffer(volume->buffer, xres, yres, zres);
	fill_voxel_buffer(volume->buffer, 0);

	compute_filter_size(volume);
}

void VolSetBounds(struct Volume *volume, double *bounds)
{
	BOX3_COPY(volume->bounds, bounds);
	volume->size[0] = BOX3_XSIZE(volume->bounds);
	volume->size[1] = BOX3_YSIZE(volume->bounds);
	volume->size[2] = BOX3_ZSIZE(volume->bounds);

	compute_filter_size(volume);
}

void VolGetBounds(const struct Volume *volume, double *bounds)
{
	BOX3_COPY(bounds, volume->bounds);
}

void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k)
{
	*i = volume->buffer->res[0];
	*j = volume->buffer->res[1];
	*k = volume->buffer->res[2];
}

double VolGetFilterSize(const struct Volume *volume)
{
	return volume->filtersize;
}

void VolPointToIndex(const struct Volume *volume, const double *point,
		int *i, int *j, int *k)
{
	*i = (int) ((point[0] - volume->bounds[0]) / volume->size[0] * volume->buffer->res[0]);
	*j = (int) ((point[1] - volume->bounds[1]) / volume->size[1] * volume->buffer->res[1]);
	*k = (int) ((point[2] - volume->bounds[2]) / volume->size[2] * volume->buffer->res[2]);

	if (point[0] == volume->buffer->res[0])
		*i -= 1;
	if (point[1] == volume->buffer->res[1])
		*j -= 1;
	if (point[2] == volume->buffer->res[2])
		*k -= 1;
}

void VolIndexToPoint(const struct Volume *volume, int i, int j, int k, double *point)
{
	point[0] = (i + .5) / volume->buffer->res[0] * volume->size[0] + volume->bounds[0];
	point[1] = (j + .5) / volume->buffer->res[1] * volume->size[1] + volume->bounds[1];
	point[2] = (k + .5) / volume->buffer->res[2] * volume->size[2] + volume->bounds[2];
}

void VolSetValue(struct Volume *volume, int x, int y, int z, float value)
{
	if (volume->buffer == NULL) {
		return;
	}
	set_buffer_value(volume->buffer, x, y, z, value);
}

float VolGetValue(const struct Volume *volume, int x, int y, int z)
{
	return get_buffer_value(volume->buffer, x, y, z);
}

int VolGetSample(const struct Volume *volume, const double *point,
			struct VolumeSample *sample)
{
	const int hit = BoxContainsPoint(volume->bounds, point);
	double P[3];

	if (!hit) {
		return 0;
	}
	if (volume->buffer == NULL) {
		return 0;
	}

	P[0] = (point[0] - volume->bounds[0]) / volume->size[0] * volume->buffer->res[0];
	P[1] = (point[1] - volume->bounds[1]) / volume->size[1] * volume->buffer->res[1];
	P[2] = (point[2] - volume->bounds[2]) / volume->size[2] * volume->buffer->res[2];

	if (1) {
		sample->density = trilinear_buffer_value(volume->buffer, P);
	} else {
		sample->density = nearest_buffer_value(volume->buffer, P);
	}

	return 1;
}

static struct VoxelBuffer *new_voxel_buffer(void)
{
	struct VoxelBuffer *buffer;

	buffer = (struct VoxelBuffer *) malloc(sizeof(struct VoxelBuffer));
	if (buffer == NULL)
		return NULL;

	buffer->data = NULL;
	VEC3_SET(buffer->res, 0, 0, 0);

	return buffer;
}

static void free_voxel_buffer(struct VoxelBuffer *buffer)
{
	if (buffer == NULL)
		return;

	free(buffer->data);
	buffer->data = NULL;
	VEC3_SET(buffer->res, 0, 0, 0);
}

static void resize_voxel_buffer(struct VoxelBuffer *buffer, int xres, int yres, int zres)
{
	const long int new_size = xres * yres * zres * sizeof(float);

	buffer->data = (float *) realloc(buffer->data, new_size);
	if (buffer->data == NULL) {
		VEC3_SET(buffer->res, 0, 0, 0);
		return;
	}

	buffer->res[0] = xres;
	buffer->res[1] = yres;
	buffer->res[2] = zres;
}

static void fill_voxel_buffer(struct VoxelBuffer *buffer, float value)
{
	const int N = buffer->res[0] * buffer->res[1] * buffer->res[2];
	int i;

	for (i = 0; i < N; i++) {
		buffer->data[i] = value;
	}
}

static void compute_filter_size(struct Volume *volume)
{
	double voxelsize[3] = {0};

	if (volume->buffer == NULL) {
		volume->filtersize = 0;
		return;
	}

	voxelsize[0] = volume->size[0] / volume->buffer->res[0];
	voxelsize[1] = volume->size[1] / volume->buffer->res[1];
	voxelsize[2] = volume->size[2] / volume->buffer->res[2];

	volume->filtersize = VEC3_LEN(voxelsize) * .5;
}

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value)
{
	int index;

	if (x < 0 || buffer->res[0] <= x)
		return;
	if (y < 0 || buffer->res[1] <= y)
		return;
	if (z < 0 || buffer->res[2] <= z)
		return;

	index = (z * buffer->res[0] * buffer->res[1]) + (y * buffer->res[0]) + x;
	buffer->data[index] = value;
}

static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z)
{
	int index;

	if (x < 0 || buffer->res[0] <= x)
		return 0;
	if (y < 0 || buffer->res[1] <= y)
		return 0;
	if (z < 0 || buffer->res[2] <= z)
		return 0;

	index = (z * buffer->res[0] * buffer->res[1]) + (y * buffer->res[0]) + x;

	return buffer->data[index];
}

static float trilinear_buffer_value(const struct VoxelBuffer *buffer, const double *P)
{
	int x, y, z;
	int i, j, k;
	int lowest_corner[3];
	double P_sample[3];
	float weight[3];
	float value = 0;

	P_sample[0] = P[0] - .5;
	P_sample[1] = P[1] - .5;
	P_sample[2] = P[2] - .5;

	lowest_corner[0] = (int) P_sample[0];
	lowest_corner[1] = (int) P_sample[1];
	lowest_corner[2] = (int) P_sample[2];

	for (i = 0; i < 2; i++) {
		x = lowest_corner[0] + i;
		weight[0] = 1 - fabs(P_sample[0] - x);

		for (j = 0; j < 2; j++) {
			y = lowest_corner[1] + j;
			weight[1] = 1 - fabs(P_sample[1] - y);

			for (k = 0; k < 2; k++) {
				z = lowest_corner[2] + k;
				weight[2] = 1 - fabs(P_sample[2] - z);

				value += weight[0] * weight[1] * weight[2] * get_buffer_value(buffer, x, y, z);
			}
		}
	}

	return value;
}

static float nearest_buffer_value(const struct VoxelBuffer *buffer, const double *P)
{
	const int x = (int) P[0];
	const int y = (int) P[1];
	const int z = (int) P[2];

	return get_buffer_value(buffer, x, y, z);
}

