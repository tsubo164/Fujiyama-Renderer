// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_VOLUMEACCERALATOR_H
#define FJ_VOLUMEACCERALATOR_H

#include "fj_box.h"

namespace fj {

struct VolumeAccelerator;
struct IntervalList;
struct Interval;
struct Box;
struct Ray;

enum VolumeAcceleratorType {
  VOLACC_BRUTEFORCE = 0,
  VOLACC_BVH
};

typedef int (*VolumeIntersectFunction)(const void *volume_set, int volume_id, double time,
      const struct Ray *ray, struct Interval *interval);
typedef void (*VolumeBoundsFunction)(const void *volume_set, int volume_id, struct Box *bounds);

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
  struct Box bounds_;
  int has_built_;

  // TODO should make struct PrimitiveSet?
  const void *volume_set_;
  int num_volumes_;
  struct Box volume_set_bounds_;
  VolumeIntersectFunction VolumeIntersect_;
  VolumeBoundsFunction VolumeBounds_;

  // private
  char *derived_;
  void (*FreeDerived_)(struct VolumeAccelerator *acc);
  int (*Build_)(struct VolumeAccelerator *acc);
  int (*Intersect_)(const struct VolumeAccelerator *acc, double time, const struct Ray *ray,
      struct IntervalList *intervals);
};

extern struct VolumeAccelerator *VolumeAccNew(int accelerator_type);
extern void VolumeAccFree(struct VolumeAccelerator *acc);

extern void VolumeAccSetTargetGeometry(struct VolumeAccelerator *acc,
  const void *primset, int nprims, const struct Box *primset_bounds,
  VolumeIntersectFunction prim_intersect_function,
  VolumeBoundsFunction prim_bounds_function);

extern int VolumeAccBuild(struct VolumeAccelerator *acc);
extern int VolumeAccIntersect(const struct VolumeAccelerator *acc, double time,
    const struct Ray *ray, struct IntervalList *intervals);

} // namespace xxx

#endif // FJ_XXX_H
