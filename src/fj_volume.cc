/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_volume.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_box.h"

#include <vector>
#include <cfloat>
#include <cstdio>

namespace fj {

class Resolution {
public:
  Resolution() : x(0), y(0), z(0) {}
  Resolution(int xx, int yy, int zz) : x(xx), y(yy), z(zz) {}
  ~Resolution() {}

  int x, y, z;
};

class VoxelBuffer {
public:
  VoxelBuffer();
  ~VoxelBuffer();

  void Resize(int xres, int yres, int zres);
  Resolution GetResolution() const;
  bool IsEmpty() const;

public:
  std::vector<float> data_;
  Resolution res_;
};

VoxelBuffer::VoxelBuffer() : data_(), res_()
{
}

VoxelBuffer::~VoxelBuffer()
{
}

void VoxelBuffer::Resize(int xres, int yres, int zres)
{
  data_.resize(xres * yres * zres, 0);
  res_ = Resolution(xres, yres, zres);
}

Resolution VoxelBuffer::GetResolution() const
{
  return res_;
}

bool VoxelBuffer::IsEmpty() const
{
  return data_.empty();
}

static struct VoxelBuffer *new_voxel_buffer(void);
static void free_voxel_buffer(struct VoxelBuffer *buffer);

static void compute_filter_size(struct Volume *volume);
static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value);
static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z);

static float trilinear_buffer_value(const struct VoxelBuffer *buffer, const struct Vector *P);
static float nearest_buffer_value(const struct VoxelBuffer *buffer, const struct Vector *P);

class Volume {
public:
  Volume();
  ~Volume();

public:
  VoxelBuffer *buffer_;
  Box bounds_;
  Vector size_;

  double filtersize_;
};

Volume::Volume()
{
}

Volume::~Volume()
{
}

struct Volume *VolNew(void)
{
  struct Volume *volume = FJ_MEM_ALLOC(struct Volume);
  if (volume == NULL)
    return NULL;

  volume->buffer_ = NULL;

  /* this makes empty volume valid */
  volume->bounds_ = Box();
  volume->size_ = Vector();

  compute_filter_size(volume);

  return volume;
}

void VolFree(struct Volume *volume)
{
  if (volume == NULL)
    return;

  free_voxel_buffer(volume->buffer_);
  FJ_MEM_FREE(volume);
}

void VolResize(struct Volume *volume, int xres, int yres, int zres)
{
  if (xres < 1 || yres < 1 || zres < 1) {
    return;
  }

  if (volume->buffer_ == NULL) {
    volume->buffer_ = new_voxel_buffer();
    if (volume->buffer_ == NULL) {
      /* TODO error handling when new_voxel_buffer fails */
    }
  }

  volume->buffer_->Resize(xres, yres, zres);

  compute_filter_size(volume);
}

void VolSetBounds(struct Volume *volume, struct Box *bounds)
{
  volume->bounds_ = *bounds;
  volume->size_ = BoxSize(volume->bounds_);

  compute_filter_size(volume);
}

void VolGetBounds(const struct Volume *volume, struct Box *bounds)
{
  *bounds = volume->bounds_;
}

void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k)
{
  *i = volume->buffer_->res_.x;
  *j = volume->buffer_->res_.y;
  *k = volume->buffer_->res_.z;
}

double VolGetFilterSize(const struct Volume *volume)
{
  return volume->filtersize_;
}

void VolIndexToPoint(const struct Volume *volume, int i, int j, int k,
    struct Vector *point)
{
  point->x = (i + .5) / volume->buffer_->res_.x * volume->size_.x + volume->bounds_.min.x;
  point->y = (j + .5) / volume->buffer_->res_.y * volume->size_.y + volume->bounds_.min.y;
  point->z = (k + .5) / volume->buffer_->res_.z * volume->size_.z + volume->bounds_.min.z;
}

void VolPointToIndex(const struct Volume *volume, const struct Vector *point,
    int *i, int *j, int *k)
{
  *i = (int) ((point->x - volume->bounds_.min.x) / volume->size_.x * volume->buffer_->res_.x);
  *j = (int) ((point->y - volume->bounds_.min.y) / volume->size_.y * volume->buffer_->res_.y);
  *k = (int) ((point->z - volume->bounds_.min.z) / volume->size_.z * volume->buffer_->res_.z);

  if (point->x == volume->buffer_->res_.x)
    *i -= 1;
  if (point->y == volume->buffer_->res_.y)
    *j -= 1;
  if (point->z == volume->buffer_->res_.z)
    *k -= 1;
}

void VolGetIndexRange(const struct Volume *volume,
    const struct Vector *center, double radius,
    int *xmin, int *ymin, int *zmin,
    int *xmax, int *ymax, int *zmax)
{
  struct Vector P_min;
  struct Vector P_max;

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
  if (volume->buffer_ == NULL) {
    return;
  }
  set_buffer_value(volume->buffer_, x, y, z, value);
}

float VolGetValue(const struct Volume *volume, int x, int y, int z)
{
  return get_buffer_value(volume->buffer_, x, y, z);
}

int VolGetSample(const struct Volume *volume, const struct Vector *point,
      struct VolumeSample *sample)
{
  const int hit = BoxContainsPoint(volume->bounds_, *point);
  struct Vector P;

  if (!hit) {
    return 0;
  }
  if (volume->buffer_ == NULL) {
    return 0;
  }

  P.x = (point->x - volume->bounds_.min.x) / volume->size_.x * volume->buffer_->res_.x;
  P.y = (point->y - volume->bounds_.min.y) / volume->size_.y * volume->buffer_->res_.y;
  P.z = (point->z - volume->bounds_.min.z) / volume->size_.z * volume->buffer_->res_.z;

  if (1) {
    sample->density = trilinear_buffer_value(volume->buffer_, &P);
  } else {
    sample->density = nearest_buffer_value(volume->buffer_, &P);
  }

  return 1;
}

static struct VoxelBuffer *new_voxel_buffer(void)
{
  return new VoxelBuffer();
}

static void free_voxel_buffer(struct VoxelBuffer *buffer)
{
  delete buffer;
}

static void compute_filter_size(struct Volume *volume)
{
  struct Vector voxelsize;

  if (volume->buffer_ == NULL) {
    volume->filtersize_ = 0;
    return;
  }

  voxelsize.x = volume->size_.x / volume->buffer_->res_.x;
  voxelsize.y = volume->size_.y / volume->buffer_->res_.y;
  voxelsize.z = volume->size_.z / volume->buffer_->res_.z;

  volume->filtersize_ = Length(voxelsize);
}

static void set_buffer_value(struct VoxelBuffer *buffer, int x, int y, int z, float value)
{
  int index;

  if (x < 0 || buffer->res_.x <= x)
    return;
  if (y < 0 || buffer->res_.y <= y)
    return;
  if (z < 0 || buffer->res_.z <= z)
    return;

  index = (z * buffer->res_.x * buffer->res_.y) + (y * buffer->res_.x) + x;
  buffer->data_[index] = value;
}

static float get_buffer_value(const struct VoxelBuffer *buffer, int x, int y, int z)
{
  int index;

  if (x < 0 || buffer->res_.x <= x)
    return 0;
  if (y < 0 || buffer->res_.y <= y)
    return 0;
  if (z < 0 || buffer->res_.z <= z)
    return 0;

  index = (z * buffer->res_.x * buffer->res_.y) + (y * buffer->res_.x) + x;

  return buffer->data_[index];
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

} // namespace xxx
