/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_point_cloud.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_box.h"
#include "fj_ray.h"

#include <vector>
#include <cstring>
#include <cfloat>

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Point, Vector,   P,        Position) \
  ATTR(Point, Vector,   velocity, Velocity) \
  ATTR(Point, Real,     radius,   Radius)

namespace fj {

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect);
static void point_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static void point_cloud_bounds(const void *prim_set, struct Box *bounds);
static int point_count(const void *prim_set);

static void update_bounds(struct PointCloud *ptc);

struct PointCloud {
public:
  PointCloud();
  ~PointCloud();

  int GetPointCount() const;
  void SetPointCount(int point_count);

  void AddPointPosition();
  void AddPointVelocity();
  void AddPointRadius();

  Vector   GetPointPosition(int idx) const;
  Vector   GetPointVelocity(int idx) const;
  Real     GetPointRadius(int idx) const;

  void SetPointPosition(int idx, const Vector &value);
  void SetPointVelocity(int idx, const Vector &value);
  void SetPointRadius(int idx, const Real &value);

  bool HasPointPosition() const;
  bool HasPointVelocity() const;
  bool HasPointRadius() const;

public:
  int point_count_;
  std::vector<Vector> P;
  std::vector<Vector> velocity;
  std::vector<Real> radius;
  Box bounds;
};

PointCloud::PointCloud() : point_count_(0)
{
}

PointCloud::~PointCloud()
{
}

int PointCloud::GetPointCount() const
{
  return point_count_;
}

void PointCloud::SetPointCount(int point_count)
{
  point_count_ = point_count;
}

#define ATTR(Class, Type, Name, Label) \
void PointCloud::Add##Class##Label() \
{ \
  Name.resize(Get##Class##Count()); \
} \
Type PointCloud::Get##Class##Label(int idx) const \
{ \
  if (idx < 0 || idx >= static_cast<int>(this->Name.size())) { \
    return Type(); \
  } \
  return this->Name[idx]; \
} \
void PointCloud::Set##Class##Label(int idx, const Type &value) \
{ \
  if (idx < 0 || idx >= static_cast<int>(this->Name.size())) \
    return; \
  this->Name[idx] = value; \
} \
bool PointCloud::Has##Class##Label() const \
{ \
  return this->Name.size() > 0; \
}
  ATTRIBUTE_LIST(ATTR)
#undef ATTR

struct PointCloud *PtcNew(void)
{
  return new PointCloud();
}

void PtcFree(struct PointCloud *ptc)
{
  delete ptc;
}

struct Vector *PtcAllocatePoint(struct PointCloud *ptc, int point_count)
{
  ptc->SetPointCount(point_count);

  ptc->AddPointPosition();
  // TODO REMOVE THIS;
  return &ptc->P[0];
}

void PtcSetPosition(struct PointCloud *ptc, int index, const struct Vector *P)
{
  ptc->SetPointPosition(index, *P);
}

void PtcGetPosition(const struct PointCloud *ptc, int index, struct Vector *P)
{
  *P = ptc->GetPointPosition(index);
}

double *PtcAddAttributeDouble(struct PointCloud *ptc, const char *name)
{
  if (strcmp(name, "radius") == 0) {
    ptc->AddPointRadius();
    return &ptc->radius[0];
  }
  return NULL;
}

struct Vector *PtcAddAttributeVector(struct PointCloud *ptc, const char *name)
{
  if (strcmp(name, "velocity") == 0) {
    ptc->AddPointVelocity();
    return &ptc->velocity[0];
  }

  return NULL;
}

void PtcComputeBounds(struct PointCloud *ptc)
{
  update_bounds(ptc);
}

void PtcGetPrimitiveSet(const struct PointCloud *ptc, struct PrimitiveSet *primset)
{
  MakePrimitiveSet(primset,
      "PointCloud",
      ptc,
      point_ray_intersect,
      point_bounds,
      point_cloud_bounds,
      point_count);
}

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect)
{
/*
  X = o + t * d;
  (X - center) * (X - center) = R * R;
  |d|^2 * t^2 + 2 * d * (o - center) * t + |o - center|^2 - r^2 = 0;

  t = (-d * (o - center) +- sqrt(D)) / |d|^2;
  D = {d * (o - center)}^2 - |d|^2 * (|o - center|^2 - r^2);
*/
  const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
  const struct Vector *P = &ptc->P[prim_id];
  const struct Vector *velocity = &ptc->velocity[prim_id];
  const double radius = ptc->radius[prim_id];

  struct Vector center;
  struct Vector orig_local;
  double a = 0, b = 0, c = 0;
  double discriminant = 0, disc_sqrt = 0;
  double t_hit = 0, t0 = 0, t1 = 0;

  center.x = P->x + time * velocity->x;
  center.y = P->y + time * velocity->y;
  center.z = P->z + time * velocity->z;

  orig_local.x = ray->orig.x - center.x;
  orig_local.y = ray->orig.y - center.y;
  orig_local.z = ray->orig.z - center.z;

  a = VEC3_DOT(&ray->dir, &ray->dir);
  b = VEC3_DOT(&ray->dir, &orig_local);
  c = VEC3_DOT(&orig_local, &orig_local) - radius * radius;

  discriminant = b * b - a * c;
  if (discriminant < 0) {
    return 0;
  }

  disc_sqrt = sqrt(discriminant);
  t0 = -b - disc_sqrt;
  t1 = -b + disc_sqrt;

  if (t1 <= 0) {
    return 0;
  }
  /* TODO handle case with ray->t_min */
  if (t0 <= 0) {
    t_hit = t1;
  } else {
    t_hit = t0;
  }

  isect->P = RayPointAt(*ray, t_hit);
  isect->N.x = isect->P.x - center.x;
  isect->N.y = isect->P.y - center.y;
  isect->N.z = isect->P.z - center.z;
  VEC3_NORMALIZE(&isect->N);

  isect->object = NULL;
  isect->prim_id = prim_id;
  isect->t_hit = t_hit;

  return 1;
}

static void point_bounds(const void *prim_set, int prim_id, struct Box *bounds)
{
  const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
  const struct Vector *P = &ptc->P[prim_id];
  const struct Vector *velocity = &ptc->velocity[prim_id];
  const double radius = ptc->radius[prim_id];
  struct Vector P_close = *P;

  *bounds = Box(
      P->x, P->y, P->z,
      P->x, P->y, P->z);

  P_close.x = P->x + velocity->x;
  P_close.y = P->y + velocity->y;
  P_close.z = P->z + velocity->z;

  BoxAddPoint(bounds, P_close);
  BoxExpand(bounds, radius);
}

static void point_cloud_bounds(const void *prim_set, struct Box *bounds)
{
  const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
  *bounds = ptc->bounds;
}

static int point_count(const void *prim_set)
{
  const struct PointCloud *ptc = (const struct PointCloud *) prim_set;
  return ptc->point_count_;
}

static void update_bounds(struct PointCloud *ptc)
{
  int i;

  BoxReverseInfinite(&ptc->bounds); 

  for (i = 0; i < ptc->point_count_; i++) {
    struct Box ptbox;

    point_bounds(ptc, i, &ptbox);
    BoxAddBox(&ptc->bounds, ptbox);
  }
}

} // namespace xxx
