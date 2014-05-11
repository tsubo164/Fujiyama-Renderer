/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_OBJECTGROUP_H
#define FJ_OBJECTGROUP_H

#include "fj_transform.h"

namespace fj {

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

} // namespace xxx

#endif /* FJ_XXX_H */
