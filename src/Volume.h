/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VOLUME_H
#define VOLUME_H

#ifdef __cplusplus
extern "C" {
#endif

struct Volume;
struct VolumeList;
struct Accelerator;

extern struct Volume *VolNew(void);
extern void VolFree(struct Volume *volume);

extern void VolSetupAccelerator(const struct Volume *volume, struct Accelerator *acc);
/*
extern void *VolAllocateVertex(struct Volume *volume, const char *attr_name, int nverts);
extern void *VolAllocateCurve(struct Volume *volume, const char *attr_name, int ncurves);

extern void VolComputeBounds(struct Volume *volume);

*/

/* XXX TEST */
struct Ray;
struct Intersection;
extern int VolSample(const struct Volume *volume, const struct Ray *ray,
		struct Intersection *isect, double *t_hit);

/* VolumeList interfaces */
/*
struct VolumeList *VolumeListNew(void);
void VolumeListFree(struct VolumeList *list);
void VolumeListAdd(struct VolumeList *list, const struct Volume *vol);
*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

