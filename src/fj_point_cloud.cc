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

#include <string.h>
#include <float.h>

namespace fj {

static int point_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect);
static void point_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static void point_cloud_bounds(const void *prim_set, struct Box *bounds);
static int point_count(const void *prim_set);

static void update_bounds(struct PointCloud *ptc);

struct PointCloud {
  int point_count;
  struct Vector *P;
  struct Vector *velocity;
  double *radius;

  struct Box bounds;
};

struct PointCloud *PtcNew(void)
{
  struct PointCloud *ptc = FJ_MEM_ALLOC(struct PointCloud);

  if (ptc == NULL) {
    return NULL;
  }

  ptc->point_count = 0;
  ptc->P = NULL;
  ptc->velocity = NULL;
  ptc->radius = NULL;
  ptc->bounds = Box();

  return ptc;
}

void PtcFree(struct PointCloud *ptc)
{
  if (ptc == NULL) {
    return;
  }

  VecFree(ptc->P);
  VecFree(ptc->velocity);
  free(ptc->radius);

  FJ_MEM_FREE(ptc);
}

struct Vector *PtcAllocatePoint(struct PointCloud *ptc, int point_count)
{
  struct Vector *P_tmp = NULL;

  if (ptc == NULL) {
    return NULL;
  }

  if (point_count < 1) {
    return NULL;
  }

  P_tmp = VecRealloc(ptc->P, point_count);
  if (P_tmp == NULL) {
    return NULL;
  }

  /* commit */
  ptc->P = P_tmp;
  ptc->point_count = point_count;

  return ptc->P;
}

void PtcSetPosition(struct PointCloud *ptc, int index, const struct Vector *P)
{
  if (index < 0 && index >= ptc->point_count) {
    return;
  }
  ptc->P[index] = *P;
}

void PtcGetPosition(const struct PointCloud *ptc, int index, struct Vector *P)
{
  if (index < 0 && index >= ptc->point_count) {
    return;
  }
  *P = ptc->P[index];
}

double *PtcAddAttributeDouble(struct PointCloud *ptc, const char *name)
{
  int i;

  if (strcmp(name, "radius") == 0) {
    ptc->radius = FJ_MEM_REALLOC_ARRAY(ptc->radius, double, ptc->point_count);
    for (i = 0; i < ptc->point_count; i++) {
      ptc->radius[i] = .01;
    }
    return ptc->radius;
  }

  return NULL;
}

struct Vector *PtcAddAttributeVector(struct PointCloud *ptc, const char *name)
{
  int i;

  if (strcmp(name, "velocity") == 0) {
    ptc->velocity = VecAlloc(ptc->point_count);
    for (i = 0; i < ptc->point_count; i++) {
      struct Vector v;
      ptc->velocity[i] = v;
    }
    return ptc->velocity;
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
  return ptc->point_count;
}

static void update_bounds(struct PointCloud *ptc)
{
  int i;

  BoxReverseInfinite(&ptc->bounds); 

  for (i = 0; i < ptc->point_count; i++) {
    struct Box ptbox;

    point_bounds(ptc, i, &ptbox);
    BoxAddBox(&ptc->bounds, ptbox);
  }
}

} // namespace xxx
