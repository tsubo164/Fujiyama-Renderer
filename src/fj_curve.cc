// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_curve.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_transform.h"
#include "fj_numeric.h"
#include "fj_matrix.h"
#include "fj_vector.h"
#include "fj_ray.h"
#include "fj_box.h"
#include <string.h>
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Vertex, Vector,   P_,        Position) \
  ATTR(Vertex, Color,    Cd_,       Color) \
  ATTR(Vertex, TexCoord, uv_,       Texture) \
  ATTR(Vertex, Vector,   velocity_, Velocity) \
  ATTR(Vertex, Real,     width_,    Width) \
  ATTR(Curve,  int,      indices_,  Indices)

namespace fj {

#define ATTR(Class, Type, Name, Label) \
void Curve::Add##Class##Label() \
{ \
  Name.resize(Get##Class##Count()); \
} \
Type Curve::Get##Class##Label(int idx) const \
{ \
  if (idx < 0 || idx >= static_cast<int>(Name.size())) { \
    return Type(); \
  } \
  return Name[idx]; \
} \
void Curve::Set##Class##Label(int idx, const Type &value) \
{ \
  if (idx < 0 || idx >= static_cast<int>(Name.size())) \
    return; \
  Name[idx] = value; \
} \
bool Curve::Has##Class##Label() const \
{ \
  return !Name.empty(); \
}
  ATTRIBUTE_LIST(ATTR)
#undef ATTR

class ControlPoint {
public:
  ControlPoint() {}
  ~ControlPoint() {}

public:
  Vector P;
};

class Bezier3 {
public:
  Bezier3() : cp(), width(), velocity() {}
  ~Bezier3() {}

public:
  ControlPoint cp[4];
  Real width[2];
  Vector velocity[4];
};

/* bezier curve interfaces */
static Real get_bezier3_max_radius(const Bezier3 &bezier);
static Real get_bezier3_width(const Bezier3 &bezier, Real t);
static void get_bezier3_bounds(const Bezier3 &bezier, Box *bounds);
static void get_bezier3(const Curve *curve, int prim_id, Bezier3 *bezier);
static Vector eval_bezier3(const ControlPoint *cp, Real t);
static Vector derivative_bezier3(const ControlPoint *cp, Real t);
static void split_bezier3(const Bezier3 &bezier,
    Bezier3 *left, Bezier3 *right);
static int converge_bezier3(const Bezier3 &bezier,
    Real v0, Real vn, int depth,
    Real *v_hit, Real *P_hit);
static void time_sample_bezier3(Bezier3 *bezier, Real time);

static bool box_bezier3_intersect_recursive(const Box &box, const Bezier3 &bezier, int depth);

/* helper functions */
static inline Vector mid_point(const Vector &a, const Vector &b)
{
  return (a + b) * .5;
}

static int compute_split_depth_limit(const ControlPoint *cp, Real epsilon);
static void compute_world_to_ray_matrix(const Ray &ray, Matrix *dst);

static inline Real dot_xy(const Vector &a, const Vector &b)
{
  return a.x * b.x + a.y * b.y;
}

Curve::Curve() : nverts_(0), ncurves_(0)
{
}

Curve::~Curve()
{
}

int Curve::GetVertexCount() const
{
  return nverts_;
}

int Curve::GetCurveCount() const
{
  return ncurves_;
}

void Curve::SetVertexCount(int count)
{
  nverts_ = count;
}

void Curve::SetCurveCount(int count)
{
  ncurves_ = count;
}

const Box &Curve::GetBounds() const
{
  return bounds_;
}

void Curve::ComputeBounds()
{
  Real max_radius = 0;

  bounds_.ReverseInfinite();

  for (int i = 0; i < GetCurveCount(); i++) {
    Box bezier_bounds;

    GetPrimitiveBounds(i, &bezier_bounds);
    bounds_.AddBox(bezier_bounds);

    Bezier3 bezier;
    get_bezier3(this, i, &bezier);

    const Real bezier_max_radius = get_bezier3_max_radius(bezier);
    max_radius = Max(max_radius, bezier_max_radius);
  }

  bounds_.Expand(max_radius);

  /* TODO find a better place to put this */
  cache_split_depth();
}

void Curve::cache_split_depth()
{
  assert(split_depth_.empty());

  const int NCURVES = GetCurveCount();

  split_depth_.resize(NCURVES);
  for (int i = 0; i < NCURVES; i++) {
    Bezier3 bezier;
    // TODO REMOVE 'this' POINTER
    get_bezier3(this, i, &bezier);

    int depth = compute_split_depth_limit(bezier.cp, 2*get_bezier3_max_radius(bezier) / 20.);
    depth = Clamp(depth, 1, 5);

    split_depth_[i] = depth;
  }
}

bool Curve::ray_intersect(Index prim_id, const Ray &ray,
    Real time, Intersection *isect) const
{
  Matrix world_to_ray;
  Bezier3 bezier;
  Ray nml_ray;

  // for scaled ray
  const Real ray_scale = Length(ray.dir);
  nml_ray = ray;
  nml_ray.dir /= ray_scale;

  get_bezier3(this, prim_id, &bezier);
  const int depth = split_depth_[prim_id];
  time_sample_bezier3(&bezier, time);

  compute_world_to_ray_matrix(nml_ray, &world_to_ray);
  for (int i = 0; i < 4; i++) {
    MatTransformPoint(world_to_ray, &bezier.cp[i].P);
  }

  Real ttmp = FLT_MAX;
  Real v_hit = FLT_MAX;

  const bool hit = converge_bezier3(bezier, 0, 1, depth, &v_hit, &ttmp);
  if (hit) {
    // P
    isect->t_hit = ttmp / ray_scale;
    isect->P = RayPointAt(ray, isect->t_hit);

    // dPdv
    Bezier3 original;
    get_bezier3(this, prim_id, &original);
    time_sample_bezier3(&original, time);
    isect->dPdv = derivative_bezier3(original.cp, v_hit);

    // Cd
    const int i0 = GetCurveIndices(prim_id);
    const int i1 = GetCurveIndices(prim_id) + 3;
    const Color Cd_curve0 = GetVertexColor(i0);
    const Color Cd_curve1 = GetVertexColor(i1);
    isect->Cd = ColLerp(Cd_curve0, Cd_curve1, v_hit);
  }

  return hit;
}

bool Curve::box_intersect(Index prim_id, const Box &box) const
{
  const int recursive_depth = 5;
  Bezier3 bezier;
  get_bezier3(this, prim_id, &bezier);
  const bool hit = box_bezier3_intersect_recursive(box, bezier, recursive_depth);

  return hit;
}

void Curve::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  Bezier3 bezier;
  get_bezier3(this, prim_id, &bezier);
  get_bezier3_bounds(bezier, bounds);

  /* TODO need to pass max time sample instead of 1. */
  time_sample_bezier3(&bezier, 1);

  Box bounds_shutter_close;
  get_bezier3_bounds(bezier, &bounds_shutter_close);
  bounds->AddBox(bounds_shutter_close);
}

void Curve::get_bounds(Box *bounds) const
{
  *bounds = GetBounds();
}

Index Curve::get_primitive_count() const
{
  return GetCurveCount();
}

static void compute_world_to_ray_matrix(const Ray &ray, Matrix *dst)
{
  const Real ox = ray.orig.x;
  const Real oy = ray.orig.y;
  const Real oz = ray.orig.z;
  const Real lx = ray.dir.x;
  const Real ly = ray.dir.y;
  const Real lz = ray.dir.z;

  const Real d = sqrt(lx*lx + lz*lz);
  if (d == 0) {
    /* TODO handle d == 0 */
  }
  const Real d_inv = 1. / d;

  const Matrix translate(
      1, 0, 0, -ox,
      0, 1, 0, -oy,
      0, 0, 1, -oz,
      0, 0, 0, 1);
  const Matrix rotate(
      lz*d_inv, 0, -lx*d_inv, 0,
      -lx*ly*d_inv, d, -ly*lz*d_inv, 0,
      lx, ly, lz, 0,
      0, 0, 0, 1);

  MatMultiply(dst, rotate, translate);
}

/* Based on this algorithm:
   Koji Nakamaru and Yoshio Ono, RAY TRACING FOR CURVES PRIMITIVE, WSCG 2002.
   */
static int converge_bezier3(const Bezier3 &bezier,
    Real v0, Real vn, int depth,
    Real *v_hit, Real *P_hit)
{
  const ControlPoint *cp = bezier.cp;
  const Real radius = get_bezier3_max_radius(bezier);
  Box bounds;

  get_bezier3_bounds(bezier, &bounds);

  if (bounds.min.x >= radius || bounds.max.x <= -radius ||
    bounds.min.y >= radius || bounds.max.y <= -radius ||
    bounds.min.z >= *P_hit || bounds.max.z <= 1e-6) {
    return 0;
  }

  if (depth == 0) {
    const Vector dir = cp[3].P - cp[0].P;
    Vector dP0 = cp[1].P - cp[0].P;

    if (dot_xy(dir, dP0) < 0) {
      dP0 *= -1;
    }
    if (-1 * dot_xy(dP0, cp[0].P) < 0) {
      return 0;
    }

    Vector dPn = cp[3].P - cp[2].P;

    if (dot_xy(dir, dPn) < 0) {
      dPn *= -1;
    }
    if (dot_xy(dPn, cp[3].P) < 0) {
      return 0;
    }

    // compute w on the line segment
    Real w = dir.x * dir.x + dir.y * dir.y;
    if (Abs(w) < 1e-6) {
      return 0;
    }
    w = -(cp[0].P.x * dir.x + cp[0].P.y * dir.y) / w;
    w = Clamp(w, 0, 1);

    // compute v on the curve segment
    const Real v = v0 * (1-w) + vn * w;

    const Real radius_w = .5 * get_bezier3_width(bezier, w);
    // compare x-y distance
    const Vector vP = eval_bezier3(cp, w);
    if (vP.x * vP.x + vP.y * vP.y >= radius_w * radius_w) {
      return 0;
    }

    // compare z distance
    if (vP.z <= 1e-6 || *P_hit < vP.z) {
      return 0;
    }

    // we found a new intersection
    *P_hit = vP.z;
    *v_hit = v;

    return 1;
  }

  const Real vm = (v0 + vn) * .5;
  Bezier3 bezier_left;
  Bezier3 bezier_right;

  split_bezier3(bezier, &bezier_left, &bezier_right);

  Real v_left  = FLT_MAX;
  Real v_right = FLT_MAX;
  Real t_left  = FLT_MAX;
  Real t_right = FLT_MAX;
  const bool hit_left  = converge_bezier3(bezier_left,  v0, vm, depth-1, &v_left,  &t_left);
  const bool hit_right = converge_bezier3(bezier_right, vm, vn, depth-1, &v_right, &t_right);

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

static void time_sample_bezier3(Bezier3 *bezier, Real time)
{
  for (int i = 0; i < 4; i++) {
    bezier->cp[i].P += time * bezier->velocity[i];
  }
}

static bool box_bezier3_intersect(const Box &box, const Bezier3 &bezier)
{
  const int N_STEPS = 1;
  const Vector step0 = bezier.velocity[0] / N_STEPS;
  const Vector step1 = bezier.velocity[1] / N_STEPS;
  const Vector step2 = bezier.velocity[2] / N_STEPS;
  const Vector step3 = bezier.velocity[3] / N_STEPS;

  for (int i = 0; i < N_STEPS; i++) {
    const Vector P0 = bezier.cp[0].P + i * step0;
    const Vector P1 = bezier.cp[1].P + i * step1;
    const Vector P2 = bezier.cp[2].P + i * step2;
    const Vector P3 = bezier.cp[3].P + i * step3;

    Box segment_bounds(P0, P1);
    segment_bounds.AddPoint(P2);
    segment_bounds.AddPoint(P3);

    segment_bounds.AddPoint(P0 + step0);
    segment_bounds.AddPoint(P1 + step1);
    segment_bounds.AddPoint(P2 + step2);
    segment_bounds.AddPoint(P3 + step3);

    if (BoxBoxIntersect(segment_bounds, box)) {
      return true;
    }
  }
  return false;
}

static bool box_bezier3_intersect_recursive(const Box &box, const Bezier3 &bezier, int depth)
{
  if (depth == 0) {
    return box_bezier3_intersect(box, bezier);
  }

  Bezier3 bezier_left;
  Bezier3 bezier_right;

  split_bezier3(bezier, &bezier_left, &bezier_right);

  const Vector middle_velocity = Lerp(bezier.velocity[1], bezier.velocity[2], .5);
  bezier_left.velocity[0]  = bezier.velocity[0];
  bezier_left.velocity[1]  = Lerp(bezier.velocity[0], bezier.velocity[1], .5);
  bezier_left.velocity[2]  = Lerp(bezier.velocity[1], middle_velocity,    .5);
  bezier_left.velocity[3]  = middle_velocity;
  bezier_right.velocity[0] = middle_velocity;
  bezier_right.velocity[1] = Lerp(middle_velocity,    bezier.velocity[2], .5);
  bezier_right.velocity[2] = Lerp(bezier.velocity[2], bezier.velocity[2], .5);
  bezier_right.velocity[3] = bezier.velocity[3];

  const bool hit_left  = box_bezier3_intersect_recursive(box, bezier_left,  depth - 1); 
  if (hit_left) {
    return true;
  }

  const bool hit_right = box_bezier3_intersect_recursive(box, bezier_right, depth - 1); 
  if (hit_right) {
    return true;
  }

  return false;
}

static Vector eval_bezier3(const ControlPoint *cp, Real t)
{
  const Real u = 1 - t;
  const Real a = u * u * u;
  const Real b = 3 * u * u * t;
  const Real c = 3 * u * t * t;
  const Real d = t * t * t;

  return a * cp[0].P + b * cp[1].P + c * cp[2].P + d * cp[3].P;
}

static Vector derivative_bezier3(const ControlPoint *cp, Real t)
{
  const Real u = 1 - t;
  const Real a = 2 * u * u;
  const Real b = 4 * u * t;
  const Real c = 2 * t * t;

  return
      a * (cp[1].P - cp[0].P) +
      b * (cp[2].P - cp[1].P) +
      c * (cp[3].P - cp[2].P);
}

static void split_bezier3(const Bezier3 &bezier,
    Bezier3 *left, Bezier3 *right)
{
  const Vector midP = eval_bezier3(bezier.cp, .5);
  const Vector midCP = mid_point(bezier.cp[1].P, bezier.cp[2].P);

  left->cp[0].P = bezier.cp[0].P;
  left->cp[1].P = mid_point(bezier.cp[0].P, bezier.cp[1].P);
  left->cp[2].P = mid_point(left->cp[1].P, midCP);
  left->cp[3].P = midP;

  right->cp[3].P = bezier.cp[3].P;
  right->cp[2].P = mid_point(bezier.cp[3].P, bezier.cp[2].P);
  right->cp[1].P = mid_point(right->cp[2].P, midCP);
  right->cp[0].P = midP;

  left->width[0] = bezier.width[0];
  left->width[1] = (bezier.width[0] + bezier.width[1]) * .5;
  right->width[0] = left->width[1];
  right->width[1] = bezier.width[1];
}

static int compute_split_depth_limit(const ControlPoint *cp, Real epsilon)
{
  const int N = 4;
  Real L0 = -1.;

  for (int i = 0; i < N-2; i++) {
    const Real x_val = fabs(cp[i].P.x - 2 * cp[i+1].P.x + cp[i+2].P.x);
    const Real y_val = fabs(cp[i].P.y - 2 * cp[i+1].P.y + cp[i+2].P.y);
    const Real max_val = Max(x_val, y_val);
    L0 = Max(L0, max_val);
  }

  const int r0 = (int) (log(sqrt(2.) * N * (N-1) * L0 / (8. * epsilon)) / log(4.));
  return r0;
}

static Real get_bezier3_max_radius(const Bezier3 &bezier)
{
  return .5 * Max(bezier.width[0], bezier.width[1]);
}

static Real get_bezier3_width(const Bezier3 &bezier, Real t)
{
  return Lerp(bezier.width[0], bezier.width[1], t);
}

static void get_bezier3_bounds(const Bezier3 &bezier, Box *bounds)
{
  bounds->ReverseInfinite();

  for (int i = 0; i < 4; i++) {
    bounds->AddPoint(bezier.cp[i].P);
  }

  const Real max_radius = get_bezier3_max_radius(bezier);
  bounds->Expand(max_radius);
}

static void get_bezier3(const Curve *curve, int prim_id, Bezier3 *bezier)
{
  const int i0 = curve->GetCurveIndices(prim_id);
  const int i1 = i0 + 1;
  const int i2 = i0 + 2;
  const int i3 = i0 + 3;

  bezier->cp[0].P = curve->GetVertexPosition(i0);
  bezier->cp[1].P = curve->GetVertexPosition(i1);
  bezier->cp[2].P = curve->GetVertexPosition(i2);
  bezier->cp[3].P = curve->GetVertexPosition(i3);

  bezier->width[0] = curve->GetVertexWidth(i0);
  bezier->width[1] = curve->GetVertexWidth(i3);

  bezier->velocity[0] = curve->GetVertexVelocity(i0);
  bezier->velocity[1] = curve->GetVertexVelocity(i1);
  bezier->velocity[2] = curve->GetVertexVelocity(i2);
  bezier->velocity[3] = curve->GetVertexVelocity(i3);
}

} // namespace xxx
