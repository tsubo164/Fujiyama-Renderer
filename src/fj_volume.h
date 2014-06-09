/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VOLUME_H
#define FJ_VOLUME_H

#include "fj_compatibility.h"
#include "fj_vector.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

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
  const Resolution &GetResolution() const;
  bool IsEmpty() const;

  void SetValue(int x, int y, int z, float value);
  float GetValue(int x, int y, int z) const;

private:
  std::vector<float> data_;
  Resolution res_;
};

class VolumeSample {
public:
  VolumeSample() : density(0) {}
  ~VolumeSample() {}

  float density;
};

class FJ_API Volume {
public:
  Volume();
  ~Volume();

  void Resize(int xres, int yres, int zres);

  void SetBounds(const Box &bounds);
  const Box &GetBounds() const;

  void GetResolution(int *i, int *j, int *k) const;
  Real GetFilterSize() const;

  Vector IndexToPoint(int i, int j, int k) const;
  void PointToIndex(const Vector &point, int *i, int *j, int *k) const;

  void SetValue(int x, int y, int z, float value);
  float GetValue(int x, int y, int z) const;

  bool GetSample(const Vector &point, VolumeSample *sample) const;

public:
  void compute_filter_size();

  VoxelBuffer buffer_;
  Box bounds_;
  Vector size_;

  Real filtersize_;
};

extern Volume *VolNew(void);
extern void VolFree(Volume *volume);

extern void VolGetIndexRange(const Volume *volume,
    const Vector *center, double radius,
    int *xmin, int *ymin, int *zmin,
    int *xmax, int *ymax, int *zmax);

} // namespace xxx

#endif // FJ_XXX_H
