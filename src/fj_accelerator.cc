/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_accelerator.h"
#include "fj_grid_accelerator.h"
#include "fj_bvh_accelerator.h"
#include "fj_primitive_set.h"
#include "fj_multi_thread.h"
#include "fj_memory.h"
#include "fj_box.h"
#include "fj_ray.h"

#include <assert.h>
#include <float.h>
#include <stdio.h>

namespace fj {

static const double PADDING = .0001;

class NullPrimitiveSet : public PrimitiveSet {
public:
  NullPrimitiveSet() {}
  virtual ~NullPrimitiveSet() {}

private:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const { return false; }
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const { *bounds = Box(); }
  virtual void get_bounds(Box *bounds) const { *bounds = Box(); }
  virtual Index get_primitive_count() const { return 0; }
};

static NullPrimitiveSet null_primset;

struct Accelerator {
  const char *name;
  struct Box bounds;
  int has_built;

  struct PrimitiveSet *primset;

  /* private */
  DerivedAccelerator derived;
  NewDerivedFunction NewDerived;
  FreeDerivedFunction FreeDerived;
  BuildDerivedFunction BuildDerived;
  IntersectDerivedFunction IntersectDerived;
  GetDerivedNameFunction GetDerivedName;
};

struct Accelerator *AccNew(int accelerator_type)
{
  struct Accelerator *acc = FJ_MEM_ALLOC(struct Accelerator);

  if (acc == NULL)
    return NULL;

  switch (accelerator_type) {
  case ACC_GRID:
    GetGridAcceleratorFunction(acc);
    break;
  case ACC_BVH:
    GetBVHAcceleratorFunction(acc);
    break;
  default:
    assert(!"invalid accelerator type");
    break;
  }

  acc->derived = acc->NewDerived();
  if (acc->derived == NULL) {
    AccFree(acc);
    return NULL;
  }
  acc->name = acc->GetDerivedName();
  acc->has_built = 0;

  acc->primset = &null_primset;
  BoxReverseInfinite(&acc->bounds);

  return acc;
}

void AccFree(struct Accelerator *acc)
{
  if (acc == NULL)
    return;
  if (acc->derived == NULL)
    return;

  acc->FreeDerived(acc->derived);
  FJ_MEM_FREE(acc);
}

double AccGetBoundsPadding(void)
{
  return PADDING;
}

void AccGetBounds(const struct Accelerator *acc, struct Box *bounds)
{
  *bounds = acc->bounds;
}

void AccComputeBounds(struct Accelerator *acc)
{
  acc->primset->GetBounds(&acc->bounds);
  BoxExpand(&acc->bounds, AccGetBoundsPadding());
}

int AccBuild(struct Accelerator *acc)
{
  int err = 0;

  if (acc->has_built)
    return -1;

  err = acc->BuildDerived(acc->derived, acc->primset);
  if (err)
    return -1;

  acc->has_built = 1;
  return 0;
}

static void build_accelerator(void *data)
{
  struct Accelerator *acc = (struct Accelerator *) data;

  if (!acc->has_built) {
    printf("\nbuilding %s accelerator ...\n", acc->name);
    AccBuild(acc);
    fflush(stdout);
  }
}

int AccIntersect(const struct Accelerator *acc, double time,
    const struct Ray *ray, struct Intersection *isect)
{
  double boxhit_tmin = 0;
  double boxhit_tmax = 0;

  /* check intersection with overall bounds */
  if (!BoxRayIntersect(acc->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
        &boxhit_tmin, &boxhit_tmax)) {
    return 0;
  }

  if (0) {
    /* dynamic build */
    /* now disabled. build all accelerators before rendering */
    MtCriticalSection((void *) acc, build_accelerator);
  }

  return acc->IntersectDerived(acc->derived, acc->primset, time, ray, isect);
}

void AccSetPrimitiveSet(struct Accelerator *acc, struct PrimitiveSet *primset)
{
  acc->primset = primset;
  AccComputeBounds(acc);
}

void AccSetDerivedFunctions(struct Accelerator *acc,
    NewDerivedFunction       new_derived_function,
    FreeDerivedFunction      free_derived_function,
    BuildDerivedFunction     build_derived_function,
    IntersectDerivedFunction intersect_derived_function,
    GetDerivedNameFunction   get_derived_name_function)
{
  acc->NewDerived = new_derived_function;
  acc->FreeDerived = free_derived_function;
  acc->BuildDerived = build_derived_function;
  acc->IntersectDerived = intersect_derived_function;
  acc->GetDerivedName = get_derived_name_function;
}

} // namespace xxx
