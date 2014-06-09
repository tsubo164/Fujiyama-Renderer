/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_volume.h"
#include "fj_numeric.h"
#include "fj_box.h"

#include <cfloat>
#include <cstdio>

namespace fj {

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

const Resolution &VoxelBuffer::GetResolution() const
{
  return res_;
}

bool VoxelBuffer::IsEmpty() const
{
  return data_.empty();
}

void VoxelBuffer::SetValue(int x, int y, int z, float value)
{
  if (x < 0 || res_.x <= x)
    return;
  if (y < 0 || res_.y <= y)
    return;
  if (z < 0 || res_.z <= z)
    return;

  const int64_t index = (z * res_.x * res_.y) + (y * res_.x) + x;
  data_[index] = value;
}

float VoxelBuffer::GetValue(int x, int y, int z) const
{
  if (x < 0 || res_.x <= x)
    return 0;
  if (y < 0 || res_.y <= y)
    return 0;
  if (z < 0 || res_.z <= z)
    return 0;

  const int64_t index = (z * res_.x * res_.y) + (y * res_.x) + x;
  return data_[index];
}

static float trilinear_buffer_value(const VoxelBuffer *buffer, const struct Vector *P);
static float nearest_buffer_value(const VoxelBuffer *buffer, const struct Vector *P);

class Volume {
public:
  Volume();
  ~Volume();

public:
  void compute_filter_size();

  VoxelBuffer buffer_;
  Box bounds_;
  Vector size_;

  Real filtersize_;
};

Volume::Volume() :
  buffer_(),
  bounds_(),
  size_()

{
  compute_filter_size();
}

Volume::~Volume()
{
}

void Volume::compute_filter_size()
{
  if (buffer_.IsEmpty()) {
    filtersize_ = 0;
    return;
  }

  const Resolution &res = buffer_.GetResolution();
  const Vector voxelsize(
      size_.x / res.x,
      size_.y / res.y,
      size_.z / res.z);

  filtersize_ = Length(voxelsize);
}

struct Volume *VolNew(void)
{
  return new Volume();
}

void VolFree(struct Volume *volume)
{
  delete volume;
}

void VolResize(struct Volume *volume, int xres, int yres, int zres)
{
  if (xres < 1 || yres < 1 || zres < 1) {
    return;
  }

  volume->buffer_.Resize(xres, yres, zres);
  volume->compute_filter_size();
}

void VolSetBounds(struct Volume *volume, struct Box *bounds)
{
  volume->bounds_ = *bounds;
  volume->size_ = BoxSize(volume->bounds_);

  volume->compute_filter_size();
}

void VolGetBounds(const struct Volume *volume, struct Box *bounds)
{
  *bounds = volume->bounds_;
}

void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k)
{
  const Resolution &res = volume->buffer_.GetResolution();
  *i = res.x;
  *j = res.y;
  *k = res.z;
}

double VolGetFilterSize(const struct Volume *volume)
{
  return volume->filtersize_;
}

void VolIndexToPoint(const struct Volume *volume, int i, int j, int k,
    struct Vector *point)
{
  const Resolution &res = volume->buffer_.GetResolution();
  point->x = (i + .5) / res.x * volume->size_.x + volume->bounds_.min.x;
  point->y = (j + .5) / res.y * volume->size_.y + volume->bounds_.min.y;
  point->z = (k + .5) / res.z * volume->size_.z + volume->bounds_.min.z;
}

void VolPointToIndex(const struct Volume *volume, const struct Vector *point,
    int *i, int *j, int *k)
{
  const Resolution &res = volume->buffer_.GetResolution();
  *i = (int) ((point->x - volume->bounds_.min.x) / volume->size_.x * res.x);
  *j = (int) ((point->y - volume->bounds_.min.y) / volume->size_.y * res.y);
  *k = (int) ((point->z - volume->bounds_.min.z) / volume->size_.z * res.z);

  if (point->x == res.x)
    *i -= 1;
  if (point->y == res.y)
    *j -= 1;
  if (point->z == res.z)
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
  if (volume->buffer_.IsEmpty()) {
    return;
  }
  volume->buffer_.SetValue(x, y, z, value);
}

float VolGetValue(const struct Volume *volume, int x, int y, int z)
{
  return volume->buffer_.GetValue(x, y, z);
}

int VolGetSample(const struct Volume *volume, const struct Vector *point,
      struct VolumeSample *sample)
{
  const int hit = BoxContainsPoint(volume->bounds_, *point);
  struct Vector P;

  if (!hit) {
    return 0;
  }
  if (volume->buffer_.IsEmpty()) {
    return 0;
  }

  const Resolution &res = volume->buffer_.GetResolution();
  P.x = (point->x - volume->bounds_.min.x) / volume->size_.x * res.x;
  P.y = (point->y - volume->bounds_.min.y) / volume->size_.y * res.y;
  P.z = (point->z - volume->bounds_.min.z) / volume->size_.z * res.z;

  if (1) {
    sample->density = trilinear_buffer_value(&volume->buffer_, &P);
  } else {
    sample->density = nearest_buffer_value(&volume->buffer_, &P);
  }

  return 1;
}

static float trilinear_buffer_value(const VoxelBuffer *buffer, const struct Vector *P)
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

        value += weight[0] * weight[1] * weight[2] * buffer->GetValue(x, y, z);
      }
    }
  }

  return value;
}

static float nearest_buffer_value(const VoxelBuffer *buffer, const struct Vector *P)
{
  const int x = (int) P->x;
  const int y = (int) P->y;
  const int z = (int) P->z;

  return buffer->GetValue(x, y, z);
}

} // namespace xxx
