/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_volume.h"
#include "fj_numeric.h"

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

static float trilinear_buffer_value(const VoxelBuffer &buffer, const Vector &P);
static float nearest_buffer_value(const VoxelBuffer &buffer, const Vector &P);

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

void Volume::Resize(int xres, int yres, int zres)
{
  if (xres < 1 || yres < 1 || zres < 1) {
    return;
  }

  buffer_.Resize(xres, yres, zres);
  compute_filter_size();
}

void Volume::SetBounds(const Box &bounds)
{
  bounds_ = bounds;
  size_ = BoxSize(bounds_);

  compute_filter_size();
}

const Box &Volume::GetBounds() const
{
  return bounds_;
}

void Volume::GetResolution(int *i, int *j, int *k) const
{
  const Resolution &res = buffer_.GetResolution();
  *i = res.x;
  *j = res.y;
  *k = res.z;
}

Real Volume::GetFilterSize() const
{
  return filtersize_;
}

Vector Volume::IndexToPoint(int i, int j, int k) const
{
  const Resolution &res = buffer_.GetResolution();
  return Vector(
      (i + .5) / res.x * size_.x + bounds_.min.x,
      (j + .5) / res.y * size_.y + bounds_.min.y,
      (k + .5) / res.z * size_.z + bounds_.min.z);
}

void Volume::PointToIndex(const Vector &point, int *i, int *j, int *k) const
{
  const Resolution &res = buffer_.GetResolution();
  *i = (int) ((point.x - bounds_.min.x) / size_.x * res.x);
  *j = (int) ((point.y - bounds_.min.y) / size_.y * res.y);
  *k = (int) ((point.z - bounds_.min.z) / size_.z * res.z);

  if (point.x == res.x)
    *i -= 1;
  if (point.y == res.y)
    *j -= 1;
  if (point.z == res.z)
    *k -= 1;
}

void Volume::SetValue(int x, int y, int z, float value)
{
  if (buffer_.IsEmpty()) {
    return;
  }
  buffer_.SetValue(x, y, z, value);
}

float Volume::GetValue(int x, int y, int z) const
{
  return buffer_.GetValue(x, y, z);
}

bool Volume::GetSample(const Vector &point, VolumeSample *sample) const
{
  if (buffer_.IsEmpty()) {
    return false;
  }

  const bool hit = BoxContainsPoint(bounds_, point);
  if (!hit) {
    return false;
  }

  const Resolution &res = buffer_.GetResolution();
  const Vector P(
      (point.x - bounds_.min.x) / size_.x * res.x,
      (point.y - bounds_.min.y) / size_.y * res.y,
      (point.z - bounds_.min.z) / size_.z * res.z);

  if (1) {
    sample->density = trilinear_buffer_value(buffer_, P);
  } else {
    sample->density = nearest_buffer_value(buffer_, P);
  }

  return true;
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

Volume *VolNew(void)
{
  return new Volume();
}

void VolFree(Volume *volume)
{
  delete volume;
}

void VolGetIndexRange(const Volume *volume,
    const Vector *center, double radius,
    int *xmin, int *ymin, int *zmin,
    int *xmax, int *ymax, int *zmax)
{
  const Vector P_min(
      center->x - radius,
      center->y - radius,
      center->z - radius);

  const Vector P_max(
      center->x + radius,
      center->y + radius,
      center->z + radius);

  volume->PointToIndex(P_min, xmin, ymin, zmin);
  volume->PointToIndex(P_max, xmax, ymax, zmax);
}

static float trilinear_buffer_value(const VoxelBuffer &buffer, const Vector &P)
{
  const Vector P_sample(
      P.x - .5,
      P.y - .5,
      P.z - .5);

  const int lowest_corner[3] = {
      (int) P_sample[0],
      (int) P_sample[1],
      (int) P_sample[2]};

  int x, y, z;
  float weight[3];
  float value = 0;

  for (int i = 0; i < 2; i++) {
    x = lowest_corner[0] + i;
    weight[0] = 1 - fabs(P_sample[0] - x);

    for (int j = 0; j < 2; j++) {
      y = lowest_corner[1] + j;
      weight[1] = 1 - fabs(P_sample[1] - y);

      for (int k = 0; k < 2; k++) {
        z = lowest_corner[2] + k;
        weight[2] = 1 - fabs(P_sample[2] - z);

        value += weight[0] * weight[1] * weight[2] * buffer.GetValue(x, y, z);
      }
    }
  }

  return value;
}

static float nearest_buffer_value(const VoxelBuffer &buffer, const Vector &P)
{
  const int x = (int) P.x;
  const int y = (int) P.y;
  const int z = (int) P.z;

  return buffer.GetValue(x, y, z);
}

} // namespace xxx
