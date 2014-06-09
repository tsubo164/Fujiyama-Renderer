/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_VOLUME_H
#define FJ_VOLUME_H

#include "fj_vector.h"
#include "fj_types.h"
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

struct Volume;
struct Vector;
struct Box;

struct VolumeSample {
  float density;
};

extern struct Volume *VolNew(void);
extern void VolFree(struct Volume *volume);

extern void VolResize(struct Volume *volume, int xres, int yres, int zres);
extern void VolSetBounds(struct Volume *volume, struct Box *bounds);
extern void VolGetBounds(const struct Volume *volume, struct Box *bounds);
extern void VolGetResolution(const struct Volume *volume, int *i, int *j, int *k);

extern double VolGetFilterSize(const struct Volume *volume);

extern void VolIndexToPoint(const struct Volume *volume, int i, int j, int k,
    struct Vector *point);
extern void VolPointToIndex(const struct Volume *volume, const struct Vector *point,
    int *i, int *j, int *k);

extern void VolGetIndexRange(const struct Volume *volume,
    const struct Vector *center, double radius,
    int *xmin, int *ymin, int *zmin,
    int *xmax, int *ymax, int *zmax);

extern void VolSetValue(struct Volume *volume, int x, int y, int z, float value);
extern float VolGetValue(const struct Volume *volume, int x, int y, int z);

extern int VolGetSample(const struct Volume *volume, const struct Vector *point,
      struct VolumeSample *sample);

} // namespace xxx

#endif // FJ_XXX_H
