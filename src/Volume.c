/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Volume.h"
#include "Vector.h"
#include "Box.h"
#include <stdlib.h>
#include <float.h>
#include <stdio.h>

struct VoxelBuffer {
	float *data;
	int xres, yres, zres;
};

struct Volume {
	struct VoxelBuffer *buffer;
	double bounds[6];
	double size[3];
};

static struct VoxelBuffer *new_voxel_buffer(void);
static void free_voxel_buffer(struct VoxelBuffer *buffer);
static void resize_voxel_buffer(struct VoxelBuffer *buffer, int xres, int yres, int zres);
static void fill_voxel_buffer(struct VoxelBuffer *buffer, float value);

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value);
static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z);

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
}

void VolSetBounds(struct Volume *volume, double *bounds)
{
	BOX3_COPY(volume->bounds, bounds);
	volume->size[0] = BOX3_XSIZE(volume->bounds);
	volume->size[1] = BOX3_YSIZE(volume->bounds);
	volume->size[2] = BOX3_ZSIZE(volume->bounds);
}

void VolGetBounds(const struct Volume *volume, double *bounds)
{
	BOX3_COPY(bounds, volume->bounds);
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
	return 0;
}

int VolGetSample(const struct Volume *volume, const double *point,
			struct VolumeSample *sample)
{
	const int hit = BoxContainsPoint(volume->bounds, point);
	int x, y, z;

	if (!hit) {
		return 0;
	}
	if (volume->buffer == NULL) {
		return 0;
	}

	x = (int) ((point[0] - volume->bounds[0]) * volume->buffer->xres * volume->size[0]);
	y = (int) ((point[1] - volume->bounds[1]) * volume->buffer->yres * volume->size[1]);
	z = (int) ((point[2] - volume->bounds[2]) * volume->buffer->zres * volume->size[2]);

	sample->density = get_buffer_value(volume->buffer, x, y, z);

	return 1;
}

static struct VoxelBuffer *new_voxel_buffer(void)
{
	struct VoxelBuffer *buffer;

	buffer = (struct VoxelBuffer *) malloc(sizeof(struct VoxelBuffer));
	if (buffer == NULL)
		return NULL;

	buffer->data = NULL;
	buffer->xres = 0;
	buffer->yres = 0;
	buffer->zres = 0;

	return buffer;
}

static void free_voxel_buffer(struct VoxelBuffer *buffer)
{
	if (buffer == NULL)
		return;

	free(buffer->data);
	buffer->data = NULL;
	buffer->xres = 0;
	buffer->yres = 0;
	buffer->zres = 0;
}

static void resize_voxel_buffer(struct VoxelBuffer *buffer, int xres, int yres, int zres)
{
	const long int new_size = xres * yres * zres * sizeof(float);

	buffer->data = (float *) realloc(buffer->data, new_size);
	if (buffer->data == NULL) {
		buffer->xres = 0;
		buffer->yres = 0;
		buffer->zres = 0;
		return;
	}

	buffer->xres = xres;
	buffer->yres = yres;
	buffer->zres = zres;
}

static void fill_voxel_buffer(struct VoxelBuffer *buffer, float value)
{
	const int N = buffer->xres * buffer->yres * buffer->zres;
	int i;

	for (i = 0; i < N; i++) {
		buffer->data[i] = value;
	}
}

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value)
{
	int index;

	if (x < 0 || buffer->xres <= x)
		return;
	if (y < 0 || buffer->yres <= y)
		return;
	if (z < 0 || buffer->zres <= z)
		return;

	index = (z * buffer->xres * buffer->yres) + (y * buffer->xres) + x;
	buffer->data[index] = value;
}

static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z)
{
	int index;

	if (x < 0 || buffer->xres <= x)
		return 0;
	if (y < 0 || buffer->yres <= y)
		return 0;
	if (z < 0 || buffer->zres <= z)
		return 0;

	index = (z * buffer->xres * buffer->yres) + (y * buffer->xres) + x;

	return buffer->data[index];
}

