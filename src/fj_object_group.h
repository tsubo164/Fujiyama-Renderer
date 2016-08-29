// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_OBJECT_GROUP_H
#define FJ_OBJECT_GROUP_H

#include "fj_object_set.h"

namespace fj {

class ObjectInstance;
class VolumeAccelerator;
class Accelerator;

class ObjectGroup {
public:
  ObjectGroup();
  ~ObjectGroup();

  void AddObject(const ObjectInstance *obj);
  const Accelerator *GetSurfaceAccelerator() const;
  const VolumeAccelerator *GetVolumeAccelerator() const;

  void ComputeBounds();

private:
  ObjectSet surface_set;
  ObjectSet volume_set;

  Accelerator *surface_acc;
  VolumeAccelerator *volume_acc;
};

extern ObjectGroup *ObjGroupNew(void);
extern void ObjGroupFree(ObjectGroup *grp);

} // namespace xxx

#endif // FJ_XXX_H
