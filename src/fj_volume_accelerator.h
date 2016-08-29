// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_VOLUMEACCERALATOR_H
#define FJ_VOLUMEACCERALATOR_H

#include "fj_box.h"

namespace fj {

class VolumeAccelerator;
class IntervalList;
class Interval;
class Box;
class Ray;

enum VolumeAcceleratorType {
  VOLACC_BRUTEFORCE = 0,
  VOLACC_BVH
};

typedef int (*VolumeIntersectFunction)(const void *volume_set, int volume_id, double time,
      const Ray *ray, Interval *interval);
typedef void (*VolumeBoundsFunction)(const void *volume_set, int volume_id, Box *bounds);

class VolumeAccelerator {
public:
  VolumeAccelerator();
  virtual ~VolumeAccelerator();

  const Box &GetBounds() const;

  int Build();
  bool Intersect(const Ray &ray, double time, IntervalList *intervals) const;

public:
  virtual int build()
  {
    return 0;
  }
  virtual bool intersect(const Ray &ray, double time, IntervalList *intervals) const
  {
    return false;
  }

  const char *name_;
  Box bounds_;
  int has_built_;

  // TODO should make PrimitiveSet?
  const void *volume_set_;
  int num_volumes_;
  Box volume_set_bounds_;
  VolumeIntersectFunction VolumeIntersect_;
  VolumeBoundsFunction VolumeBounds_;

  // private
  char *derived_;
  void (*FreeDerived_)(VolumeAccelerator *acc);
  int (*Build_)(VolumeAccelerator *acc);
  int (*Intersect_)(const VolumeAccelerator *acc, double time, const Ray *ray,
      IntervalList *intervals);
};

extern VolumeAccelerator *VolumeAccNew(int accelerator_type);
extern void VolumeAccFree(VolumeAccelerator *acc);

extern void VolumeAccSetTargetGeometry(VolumeAccelerator *acc,
  const void *primset, int nprims, const Box *primset_bounds,
  VolumeIntersectFunction prim_intersect_function,
  VolumeBoundsFunction prim_bounds_function);

extern int VolumeAccBuild(VolumeAccelerator *acc);
extern int VolumeAccIntersect(const VolumeAccelerator *acc, double time,
    const Ray *ray, IntervalList *intervals);

} // namespace xxx

#endif // FJ_XXX_H
