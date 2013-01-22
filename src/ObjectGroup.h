/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef OBJECTGROUP_H
#define OBJECTGROUP_H

#include "Transform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectGroup;
struct ObjectInstance;

struct Intersection;
struct Accelerator;
struct Ray;

extern struct ObjectGroup *ObjGroupNew(void);
extern void ObjGroupFree(struct ObjectGroup *grp);

extern void ObjGroupAdd(struct ObjectGroup *grp, const struct ObjectInstance *obj);
extern const struct Accelerator *ObjGroupGetSurfaceAccelerator(const struct ObjectGroup *grp);
extern const struct VolumeAccelerator *ObjGroupGetVolumeAccelerator(const struct ObjectGroup *grp);

extern void ObjGroupComputeBounds(struct ObjectGroup *grp);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

