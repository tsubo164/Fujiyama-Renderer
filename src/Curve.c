/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Curve.h"
#include "Intersection.h"
#include "PrimitiveSet.h"
#include "Transform.h"
#include "Numeric.h"
#include "Matrix.h"
#include "Memory.h"
#include "Vector.h"
#include "Ray.h"
#include "Box.h"
#include <string.h>
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

#define VEC_SUB(dst,a,b) do { \
  (dst)->x = (a)->x - (b)->x; \
  (dst)->y = (a)->y - (b)->y; \
  (dst)->z = (a)->z - (b)->z; \
  } while(0)

struct ControlPoint {
  struct Vector P;
};

struct Bezier3 {
  struct ControlPoint cp[4];
  double width[2];
};

/* curve interfaces */
static int curve_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect);
static void curve_bounds(const void *prim_set, int prim_id, struct Box *bounds);
static void curveset_bounds(const void *prim_set, struct Box *bounds);
static int curve_count(const void *prim_set);

/* bezier curve interfaces */
static double get_bezier3_max_radius(const struct Bezier3 *bezier);
static double get_bezier3_width(const struct Bezier3 *bezier, double t);
static void get_bezier3_bounds(const struct Bezier3 *bezier, struct Box *bounds);
static void get_bezier3(const struct Curve *curve, int prim_id, struct Bezier3 *bezier);
static void eval_bezier3(struct Vector *evalP, const struct ControlPoint *cp, double t);
static void derivative_bezier3(struct Vector *deriv, const struct ControlPoint *cp, double t);
static void split_bezier3(const struct Bezier3 *bezier,
    struct Bezier3 *left, struct Bezier3 *right);
static int converge_bezier3(const struct Bezier3 *bezier,
    double v0, double vn, int depth,
    double *v_hit, double *P_hit);

/* helper functions */
static void mid_point(struct Vector *mid, const struct Vector *a, const struct Vector *b);
static void cache_split_depth(const struct Curve *curve);
static int compute_split_depth_limit(const struct ControlPoint *cp, double epsilon);
static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst);

struct Curve *CrvNew(void)
{
  struct Curve *curve;

  curve = MEM_ALLOC(struct Curve);
  if (curve == NULL)
    return NULL;

  curve->P = NULL;
  curve->width = NULL;
  curve->Cd = NULL;
  curve->uv = NULL;
  curve->indices = NULL;
  BOX3_SET(&curve->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

  curve->nverts = 0;
  curve->ncurves = 0;

  curve->split_depth = NULL;

  return curve;
}

void CrvFree(struct Curve *curve)
{
  if (curve == NULL)
    return;

  VecFree(curve->P);
  MEM_FREE(curve->width);
  ColFree(curve->Cd);
  TexCoordFree(curve->uv);
  MEM_FREE(curve->indices);
  MEM_FREE(curve->split_depth);

  MEM_FREE(curve);
}

void *CrvAllocateVertex(struct Curve *curve, const char *attr_name, int nverts)
{
  void *ret = NULL;

  if (curve->nverts != 0 && curve->nverts != nverts) {
    /* TODO error handling */
#if 0
    return NULL;
#endif
  }

  if (strcmp(attr_name, "P") == 0) {
    curve->P = VecRealloc(curve->P, nverts);
    ret = curve->P;
  }
  else if (strcmp(attr_name, "width") == 0) {
    curve->width = MEM_REALLOC_ARRAY(curve->width, double, nverts);
    ret = curve->width;
  }
  else if (strcmp(attr_name, "Cd") == 0) {
    curve->Cd = ColRealloc(curve->Cd, nverts);
    ret = curve->Cd;
  }
  else if (strcmp(attr_name, "uv") == 0) {
    curve->uv = TexCoordRealloc(curve->uv, nverts);
    ret = curve->uv;
  }

  if (ret == NULL) {
    curve->nverts = 0;
    return NULL;
  }

  curve->nverts = nverts;
  return ret;
}

void *CrvAllocateCurve(struct Curve *curve, const char *attr_name, int ncurves)
{
  void *ret = NULL;

  if (curve->ncurves != 0 && curve->ncurves != ncurves) {
    /* TODO error handling */
#if 0
    return NULL;
#endif
  }

  if (strcmp(attr_name, "indices") == 0) {
    curve->indices = MEM_REALLOC_ARRAY(curve->indices, int, ncurves);
    ret = curve->indices;
  }

  if (ret == NULL) {
    curve->ncurves = 0;
    return NULL;
  }

  curve->ncurves = ncurves;
  return ret;
}

void CrvComputeBounds(struct Curve *curve)
{
  int i;
  double max_radius = 0;

  BOX3_SET(&curve->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

  for (i = 0; i < curve->ncurves; i++) {
    struct Bezier3 bezier;
    struct Box bezier_bounds;
    double bezier_max_radius;

    get_bezier3(curve, i, &bezier);
    get_bezier3_bounds(&bezier, &bezier_bounds);
    BoxAddBox(&curve->bounds, &bezier_bounds);

    bezier_max_radius = get_bezier3_max_radius(&bezier);
    max_radius = MAX(max_radius, bezier_max_radius);
  }

  BOX3_EXPAND(&curve->bounds, max_radius);
}

void CrvGetPrimitiveSet(const struct Curve *curve, struct PrimitiveSet *primset)
{
  MakePrimitiveSet(primset,
      "Curve",
      curve,
      curve_ray_intersect,
      curve_bounds,
      curveset_bounds,
      curve_count);
}

static int curve_ray_intersect(const void *prim_set, int prim_id, double time,
    const struct Ray *ray, struct Intersection *isect)
{
  const struct Curve *curve = (const struct Curve *) prim_set;
  struct Bezier3 bezier;
  struct Matrix world_to_ray;

  /* for scaled ray */
  struct Ray nml_ray;
  double ray_scale;

  double ttmp = FLT_MAX;
  double v_hit;
  int depth;
  int hit;
  int i;

  nml_ray = *ray;
  ray_scale = VEC3_LEN(&nml_ray.dir);
  {
    const double inv_dist = 1. / ray_scale;
    nml_ray.dir.x *= inv_dist;
    nml_ray.dir.y *= inv_dist;
    nml_ray.dir.z *= inv_dist;
  }

  get_bezier3(curve, prim_id, &bezier);
  if (curve->split_depth == NULL) {
    cache_split_depth(curve);
  }
  depth = curve->split_depth[prim_id];

  compute_world_to_ray_matrix(&nml_ray, &world_to_ray);
  for (i = 0; i < 4; i++) {
    MatTransformPoint(&world_to_ray, &bezier.cp[i].P);
  }

  hit = converge_bezier3(&bezier, 0, 1, depth, &v_hit, &ttmp);
  if (hit) {
    struct Bezier3 original;
    struct Color *Cd_curve0;
    struct Color *Cd_curve1;
    int i0, i1;

    /* P */
    isect->t_hit = ttmp / ray_scale;
    POINT_ON_RAY(&isect->P, &ray->orig, &ray->dir, isect->t_hit);

    /* dPdt */
    get_bezier3(curve, prim_id, &original);
    derivative_bezier3(&isect->dPdt, original.cp, v_hit);

    /* Cd */
    i0 = curve->indices[prim_id];
    i1 = curve->indices[prim_id] + 2;
    Cd_curve0 = &curve->Cd[i0];
    Cd_curve1 = &curve->Cd[i1];
    COL_LERP(&isect->Cd, Cd_curve0, Cd_curve1, v_hit);
  }

  return hit;
}

static void curve_bounds(const void *prim_set, int prim_id, struct Box *bounds)
{
  const struct Curve *curve = (const struct Curve *) prim_set;
  struct Bezier3 bezier;

  get_bezier3(curve, prim_id, &bezier);
  get_bezier3_bounds(&bezier, bounds);
}

static void curveset_bounds(const void *prim_set, struct Box *bounds)
{
  const struct Curve *curve = (const struct Curve *) prim_set;
  *bounds = curve->bounds;
}

static int curve_count(const void *prim_set)
{
  const struct Curve *curve = (const struct Curve *) prim_set;
  return curve->ncurves;
}

static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst)
{
  struct Matrix translate, rotate;
  double d, d_inv;

  const double ox = ray->orig.x;
  const double oy = ray->orig.y;
  const double oz = ray->orig.z;
  const double lx = ray->dir.x;
  const double ly = ray->dir.y;
  const double lz = ray->dir.z;

  d = sqrt(lx*lx + lz*lz);
  if (d == 0) {
    /* TODO handle d == 0 */
  }
  d_inv = 1. / d;

  MatSet(&translate,
      1, 0, 0, -ox,
      0, 1, 0, -oy,
      0, 0, 1, -oz,
      0, 0, 0, 1);
  MatSet(&rotate,
      lz*d_inv, 0, -lx*d_inv, 0,
      -lx*ly*d_inv, d, -ly*lz*d_inv, 0,
      lx, ly, lz, 0,
      0, 0, 0, 1);

  MatMultiply(dst, &rotate, &translate);
}

/* Based on this algorithm:
   Koji Nakamaru and Yoshio Ono, RAY TRACING FOR CURVES PRIMITIVE, WSCG 2002.
   */
static int converge_bezier3(const struct Bezier3 *bezier,
    double v0, double vn, int depth,
    double *v_hit, double *P_hit)
{
  const struct ControlPoint *cp = bezier->cp;
  const double radius = get_bezier3_max_radius(bezier);
  struct Box bounds;

  get_bezier3_bounds(bezier, &bounds);

  if (bounds.min.x >= radius || bounds.max.x <= -radius ||
    bounds.min.y >= radius || bounds.max.y <= -radius ||
    bounds.min.z >= *P_hit || bounds.max.z <= 1e-6) {
    return 0;
  }

  if (depth == 0) {
    struct Vector dir;
    struct Vector dP0;
    struct Vector dPn;
    struct Vector vP;
    double v, w;
    double radius_w;

    VEC_SUB(&dir, &cp[3].P, &cp[0].P);
    VEC_SUB(&dP0, &cp[1].P, &cp[0].P);

#define DOT_XY(a,b) ((a)->x * (b)->x + (a)->y * (b)->y)
    /* not VEC3_DOT */
    if (DOT_XY(&dir, &dP0) < 0) {
      dP0.x *= -1;
      dP0.y *= -1;
      dP0.z *= -1;
    }
    /* not VEC3_DOT */
    if (-1 * DOT_XY(&dP0, &cp[0].P) < 0) {
      return 0;
    }

    VEC_SUB(&dPn, &cp[3].P, &cp[2].P);

    /* not VEC3_DOT */
    if (DOT_XY(&dir, &dPn) < 0) {
      dPn.x *= -1;
      dPn.y *= -1;
      dPn.z *= -1;
    }
    /* not VEC3_DOT */
    if (DOT_XY(&dPn, &cp[3].P) < 0) {
      return 0;
    }
#undef DOT_XY

    /* compute w on the line segment */
    w = dir.x * dir.x + dir.y * dir.y;
    if (ABS(w) < 1e-6) {
      return 0;
    }
    w = -(cp[0].P.x * dir.x + cp[0].P.y * dir.y) / w;
    w = CLAMP(w, 0, 1);

    /* compute v on the curve segment */
    v = v0 * (1-w) + vn * w;

    radius_w = .5 * get_bezier3_width(bezier, w);
    /* compare x-y distance */
    eval_bezier3(&vP, cp, w);
    if (vP.x * vP.x + vP.y * vP.y >= radius_w * radius_w) {
      return 0;
    }

    /* compare z distance */
    if (vP.z <= 1e-6 || *P_hit < vP.z) {
      return 0;
    }

    /* we found a new intersection */
    *P_hit = vP.z;
    *v_hit = v;

    return 1;
  }

  {
    const double vm = (v0 + vn) * .5;
    struct Bezier3 bezier_left;
    struct Bezier3 bezier_right;
    int hit_left, hit_right;
    double t_left, t_right;
    double v_left, v_right;

    split_bezier3(bezier, &bezier_left, &bezier_right);

    t_left  = FLT_MAX;
    t_right = FLT_MAX;
    hit_left  = converge_bezier3(&bezier_left,  v0, vm, depth-1, &v_left,  &t_left);
    hit_right = converge_bezier3(&bezier_right, vm, vn, depth-1, &v_right, &t_right);

    if (hit_left || hit_right) {
      if (t_left < t_right) {
        *P_hit = t_left;
        *v_hit = v_left;
      } else {
        *P_hit = t_right;
        *v_hit = v_right;
      }
    }

    return hit_left || hit_right;
  }
}

static void eval_bezier3(struct Vector *evalP, const struct ControlPoint *cp, double t)
{
  const double u = 1-t;
  const double a = u * u * u;
  const double b = 3 * u * u * t;
  const double c = 3 * u * t * t;
  const double d = t * t * t;

  evalP->x = a * cp[0].P.x + b * cp[1].P.x + c * cp[2].P.x + d * cp[3].P.x;
  evalP->y = a * cp[0].P.y + b * cp[1].P.y + c * cp[2].P.y + d * cp[3].P.y;
  evalP->z = a * cp[0].P.z + b * cp[1].P.z + c * cp[2].P.z + d * cp[3].P.z;
}

static void derivative_bezier3(struct Vector *deriv, const struct ControlPoint *cp, double t)
{
  const double u = 1-t;
  const double a = 2 * u * u;
  const double b = 4 * u * t;
  const double c = 2 * t * t;
  struct ControlPoint CP[3];

  VEC_SUB(&CP[0].P, &cp[1].P, &cp[0].P);
  VEC_SUB(&CP[1].P, &cp[2].P, &cp[1].P);
  VEC_SUB(&CP[2].P, &cp[3].P, &cp[2].P);

  deriv->x = a * CP[0].P.x + b * CP[1].P.x + c * CP[2].P.x;
  deriv->y = a * CP[0].P.y + b * CP[1].P.y + c * CP[2].P.y;
  deriv->z = a * CP[0].P.z + b * CP[1].P.z + c * CP[2].P.z;
}

static void split_bezier3(const struct Bezier3 *bezier,
    struct Bezier3 *left, struct Bezier3 *right)
{
  struct Vector midP;
  struct Vector midCP;

  eval_bezier3(&midP, bezier->cp, .5);
  mid_point(&midCP, &bezier->cp[1].P, &bezier->cp[2].P);

  left->cp[0].P = bezier->cp[0].P;
  mid_point(&left->cp[1].P, &bezier->cp[0].P, &bezier->cp[1].P);
  mid_point(&left->cp[2].P, &left->cp[1].P, &midCP);
  left->cp[3].P = midP;

  right->cp[3].P = bezier->cp[3].P;
  mid_point(&right->cp[2].P, &bezier->cp[3].P, &bezier->cp[2].P);
  mid_point(&right->cp[1].P, &right->cp[2].P, &midCP);
  right->cp[0].P = midP;

  left->width[0] = bezier->width[0];
  left->width[1] = (bezier->width[0] + bezier->width[1]) * .5;
  right->width[0] = left->width[1];
  right->width[1] = bezier->width[1];
}

static void mid_point(struct Vector *mid, const struct Vector *a, const struct Vector *b)
{
  mid->x = (a->x + b->x) * .5;
  mid->y = (a->y + b->y) * .5;
  mid->z = (a->z + b->z) * .5;
}

static void cache_split_depth(const struct Curve *curve)
{
  int i;
  struct Curve *mutable_curve = (struct Curve *) curve;
  assert(mutable_curve->split_depth == NULL);


  mutable_curve->split_depth = MEM_ALLOC_ARRAY(int, mutable_curve->ncurves);
  for (i = 0; i < mutable_curve->ncurves; i++) {
    struct Bezier3 bezier;
    int depth;

    get_bezier3(mutable_curve, i, &bezier);
    depth = compute_split_depth_limit(bezier.cp, 2*get_bezier3_max_radius(&bezier) / 20.);
    depth = CLAMP(depth, 1, 5);

    mutable_curve->split_depth[i] = depth;
  }
}

static int compute_split_depth_limit(const struct ControlPoint *cp, double epsilon)
{
  int i;
  int r0;
  const int N = 4;
  double L0;

  L0 = -1.;
  for (i = 0; i < N-2; i++) {
    const double x_val = fabs(cp[i].P.x - 2 * cp[i+1].P.x + cp[i+2].P.x);
    const double y_val = fabs(cp[i].P.y - 2 * cp[i+1].P.y + cp[i+2].P.y);
    const double max_val = MAX(x_val, y_val);
    L0 = MAX(L0, max_val);
  }

  r0 = (int) (log(sqrt(2.) * N * (N-1) * L0 / (8. * epsilon)) / log(4.));
  return r0;
}

static double get_bezier3_max_radius(const struct Bezier3 *bezier)
{
  return .5 * MAX(bezier->width[0], bezier->width[1]);
}

static double get_bezier3_width(const struct Bezier3 *bezier, double t)
{
  return LERP(bezier->width[0], bezier->width[1], t);
}

static void get_bezier3_bounds(const struct Bezier3 *bezier, struct Box *bounds)
{
  int i;
  double max_radius;

  BOX3_SET(bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (i = 0; i < 4; i++) {
    BoxAddPoint(bounds, &bezier->cp[i].P);
  }

  max_radius = get_bezier3_max_radius(bezier);
  BOX3_EXPAND(bounds, max_radius);
}

static void get_bezier3(const struct Curve *curve, int prim_id, struct Bezier3 *bezier)
{
  int i0, i1, i2, i3;

  i0 = curve->indices[prim_id];
  i1 = i0 + 1;
  i2 = i0 + 2;
  i3 = i0 + 3;

  bezier->cp[0].P = curve->P[i0];
  bezier->cp[1].P = curve->P[i1];
  bezier->cp[2].P = curve->P[i2];
  bezier->cp[3].P = curve->P[i3];

  bezier->width[0] = curve->width[4*prim_id + 0];
  bezier->width[1] = curve->width[4*prim_id + 3];
}

