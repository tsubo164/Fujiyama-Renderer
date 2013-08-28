/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VOLUMEACCERALATOR_H
#define VOLUMEACCERALATOR_H

#ifdef __cplusplus
extern "C" {
#endif

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

extern struct VolumeAccelerator *VolumeAccNew(int accelerator_type);
extern void VolumeAccFree(struct VolumeAccelerator *acc);

extern void VolumeAccGetBounds(const struct VolumeAccelerator *acc, struct Box *bounds);
extern void VolumeAccSetTargetGeometry(struct VolumeAccelerator *acc,
  const void *primset, int nprims, const struct Box *primset_bounds,
  VolumeIntersectFunction prim_intersect_function,
  VolumeBoundsFunction prim_bounds_function);

extern int VolumeAccBuild(struct VolumeAccelerator *acc);
extern int VolumeAccIntersect(const struct VolumeAccelerator *acc, double time,
    const struct Ray *ray, struct IntervalList *intervals);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

