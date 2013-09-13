/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Volume.h"
#include "Numeric.h"
#include "Memory.h"
#include "Vector.h"
#include "Box.h"

#include <float.h>
#include <stdio.h>

struct Resolution {
  int x, y, z;
};

struct VoxelBuffer {
  float *data;
  struct Resolution res;
};

struct Volume {
  struct VoxelBuffer *buffer;
  struct Box bounds;
  struct Vector size;

  double filtersize;
};

static struct VoxelBuffer *new_voxel_buffer(void);
static void free_voxel_buffer(struct VoxelBuffer *buffer);
static void resize_voxel_buffer(struct VoxelBuffer *buffer, int xres, int yres, int zres);
static void fill_voxel_buffer(struct VoxelBuffer *buffer, float value);
static void compute_filter_size(struct Volume *volume);

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value);
static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z);

static float trilinear_buffer_value(const struct VoxelBuffer *buffer, const struct Vector *P);
static float nearest_buffer_value(const struct VoxelBuffer *buffer, const struct Vector *P);

struct Volume *VolNew(void)
{
  struct Volume *volume = SI_MEM_ALLOC(struct Volume);
  if (volume == NULL)
    return NULL;

  volume->buffer = NULL;

  /* this makes empty volume valid */
  BOX3_SET(&volume->bounds, 0, 0, 0, 0, 0, 0);
  VEC3_SET(&volume->size, 0, 0, 0);

  compute_filter_size(volume);

  return volume;
}

void VolFree(struct Volume *volume)
{
  if (volume == NULL)
    return;

  free_voxel_buffer(volume->buffer);
  SI_MEM_FREE(volume);
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

void VolSetBounds(struct Volume *volume, struct Box *bounds)
{
  volume->bounds = *bounds;

  volume->size.x = BOX3_XSIZE(&volume->bounds);
  volume->size.y = BOX3_YSIZE(&volume->bounds);
  volume->size.z = BOX3_ZSIZE(&volume->bounds);

  compute_filter_size(volume);
}

void VolGetBounds(const struct Volume *volume, struct Box *bounds)
{
  *bounds = volume->bounds;
}

void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k)
{
  *i = volume->buffer->res.x;
  *j = volume->buffer->res.y;
  *k = volume->buffer->res.z;
}

double VolGetFilterSize(const struct Volume *volume)
{
  return volume->filtersize;
}

void VolIndexToPoint(const struct Volume *volume, int i, int j, int k,
    struct Vector *point)
{
  point->x = (i + .5) / volume->buffer->res.x * volume->size.x + volume->bounds.min.x;
  point->y = (j + .5) / volume->buffer->res.y * volume->size.y + volume->bounds.min.y;
  point->z = (k + .5) / volume->buffer->res.z * volume->size.z + volume->bounds.min.z;
}

void VolPointToIndex(const struct Volume *volume, const struct Vector *point,
    int *i, int *j, int *k)
{
  *i = (int) ((point->x - volume->bounds.min.x) / volume->size.x * volume->buffer->res.x);
  *j = (int) ((point->y - volume->bounds.min.y) / volume->size.y * volume->buffer->res.y);
  *k = (int) ((point->z - volume->bounds.min.z) / volume->size.z * volume->buffer->res.z);

  if (point->x == volume->buffer->res.x)
    *i -= 1;
  if (point->y == volume->buffer->res.y)
    *j -= 1;
  if (point->z == volume->buffer->res.z)
    *k -= 1;
}

void VolGetIndexRange(const struct Volume *volume,
    const struct Vector *center, double radius,
    int *xmin, int *ymin, int *zmin,
    int *xmax, int *ymax, int *zmax)
{
  struct Vector P_min = {0, 0, 0};
  struct Vector P_max = {0, 0, 0};

  P_min.x = center->x - radius;
  P_min.y = center->y - radius;
  P_min.z = center->z - radius;
  P_max.x = center->x + radius;
  P_max.y = center->y + radius;
  P_max.z = center->z + radius;

  VolPointToIndex(volume, &P_min, xmin, ymin, zmin);
  VolPointToIndex(volume, &P_max, xmax, ymax, zmax);
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

int VolGetSample(const struct Volume *volume, const struct Vector *point,
      struct VolumeSample *sample)
{
  const int hit = BoxContainsPoint(&volume->bounds, point);
  struct Vector P = {0, 0, 0};

  if (!hit) {
    return 0;
  }
  if (volume->buffer == NULL) {
    return 0;
  }

  P.x = (point->x - volume->bounds.min.x) / volume->size.x * volume->buffer->res.x;
  P.y = (point->y - volume->bounds.min.y) / volume->size.y * volume->buffer->res.y;
  P.z = (point->z - volume->bounds.min.z) / volume->size.z * volume->buffer->res.z;

  if (1) {
    sample->density = trilinear_buffer_value(volume->buffer, &P);
  } else {
    sample->density = nearest_buffer_value(volume->buffer, &P);
  }

  return 1;
}

static struct VoxelBuffer *new_voxel_buffer(void)
{
  struct VoxelBuffer *buffer = SI_MEM_ALLOC(struct VoxelBuffer);
  if (buffer == NULL)
    return NULL;

  buffer->data = NULL;
  buffer->res.x = 0;
  buffer->res.y = 0;
  buffer->res.z = 0;

  return buffer;
}

static void free_voxel_buffer(struct VoxelBuffer *buffer)
{
  if (buffer == NULL)
    return;

  SI_MEM_FREE(buffer->data);
  buffer->data = NULL;
  buffer->res.x = 0;
  buffer->res.y = 0;
  buffer->res.z = 0;

  SI_MEM_FREE(buffer);
}

static void resize_voxel_buffer(struct VoxelBuffer *buffer, int xres, int yres, int zres)
{
  buffer->data = SI_MEM_REALLOC_ARRAY(buffer->data, float, xres * yres * zres);

  if (buffer->data == NULL) {
    buffer->res.x = 0;
    buffer->res.y = 0;
    buffer->res.z = 0;
    return;
  }

  buffer->res.x = xres;
  buffer->res.y = yres;
  buffer->res.z = zres;
}

static void fill_voxel_buffer(struct VoxelBuffer *buffer, float value)
{
  const int N = buffer->res.x * buffer->res.y * buffer->res.z;
  int i;

  for (i = 0; i < N; i++) {
    buffer->data[i] = value;
  }
}

static void compute_filter_size(struct Volume *volume)
{
  struct Vector voxelsize = {0, 0, 0};

  if (volume->buffer == NULL) {
    volume->filtersize = 0;
    return;
  }

  voxelsize.x = volume->size.x / volume->buffer->res.x;
  voxelsize.y = volume->size.y / volume->buffer->res.y;
  voxelsize.z = volume->size.z / volume->buffer->res.z;

  volume->filtersize = VEC3_LEN(&voxelsize);
}

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value)
{
  int index;

  if (x < 0 || buffer->res.x <= x)
    return;
  if (y < 0 || buffer->res.y <= y)
    return;
  if (z < 0 || buffer->res.z <= z)
    return;

  index = (z * buffer->res.x * buffer->res.y) + (y * buffer->res.x) + x;
  buffer->data[index] = value;
}

static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z)
{
  int index;

  if (x < 0 || buffer->res.x <= x)
    return 0;
  if (y < 0 || buffer->res.y <= y)
    return 0;
  if (z < 0 || buffer->res.z <= z)
    return 0;

  index = (z * buffer->res.x * buffer->res.y) + (y * buffer->res.x) + x;

  return buffer->data[index];
}

static float trilinear_buffer_value(const struct VoxelBuffer *buffer, const struct Vector *P)
{
  int x, y, z;
  int i, j, k;
  int lowest_corner[3];
  double P_sample[3];
  float weight[3];
  float value = 0;

  P_sample[0] = P->x - .5;
  P_sample[1] = P->y - .5;
  P_sample[2] = P->z - .5;

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

static float nearest_buffer_value(const struct VoxelBuffer *buffer, const struct Vector *P)
{
  const int x = (int) P->x;
  const int y = (int) P->y;
  const int z = (int) P->z;

  return get_buffer_value(buffer, x, y, z);
}
