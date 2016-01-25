// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_accelerator.h"
#include "fj_primitive_set.h"
#include "fj_multi_thread.h"
#include "fj_ray.h"

#include <iostream>

namespace fj {

static const Real PADDING = .0001;

class NullPrimitiveSet : public PrimitiveSet {
public:
  NullPrimitiveSet() {}
  virtual ~NullPrimitiveSet() {}

private:
  virtual bool ray_intersect(Index prim_id, const Ray &ray,
      Real time, Intersection *isect) const { return false; }
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const { *bounds = Box(); }
  virtual void get_bounds(Box *bounds) const { *bounds = Box(); }
  virtual Index get_primitive_count() const { return 0; }
};

static NullPrimitiveSet null_primset;

// for critical session
static void build_accelerator_callback(void *data);

Accelerator::Accelerator() : bounds_(), has_built_(false), primset_(NULL)
{
  SetPrimitiveSet(NULL);
}

Accelerator::~Accelerator()
{
}

Real Accelerator::GetBoundsPadding() const
{
  return PADDING;
}

const Box &Accelerator::GetBounds() const
{
  return bounds_;
}

const char *Accelerator::GetName() const
{
  return get_name();
}

bool Accelerator::HasBuilt() const
{
  return has_built_;
}

void Accelerator::ComputeBounds()
{
  primset_->GetEntireBounds(&bounds_);
  bounds_.Expand(GetBoundsPadding());
}

void Accelerator::SetPrimitiveSet(PrimitiveSet *primset)
{
  if (primset == NULL) {
    primset_ = &null_primset;
  } else {
    primset_ = primset;
  }

  ComputeBounds();
}

int Accelerator::Build()
{
  if (HasBuilt()) { 
    return -1;
  }

  const int err = build();
  if (err) {
    return -1;
  }

  has_built_ = true;
  return 0;
}

bool Accelerator::Intersect(const Ray &ray, Real time, Intersection *isect) const
{
  Real boxhit_tmin = 0;
  Real boxhit_tmax = 0;

  // check intersection with overall bounds
  const bool hit = BoxRayIntersect(bounds_, ray.orig, ray.dir, ray.tmin, ray.tmax,
        &boxhit_tmin, &boxhit_tmax);

  if (!hit) {
    return false;
  }

  if (0) {
    // dynamic build is now disabled. build all accelerators before rendering
    MtCriticalSection((void *) this, build_accelerator_callback);
  }

  return intersect(ray, time, isect);
}

static void build_accelerator_callback(void *data)
{
  Accelerator *acc = reinterpret_cast<Accelerator *>(data);

  if (!acc->HasBuilt()) {
    std::cout << "\nbuilding accelerator ... : " << acc->GetName() << std::endl;
    acc->Build();
  }
}

} // namespace xxx
