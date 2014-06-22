// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shading.h"
#include "fj_volume_accelerator.h"
#include "fj_object_instance.h"
#include "fj_intersection.h"
#include "fj_object_group.h"
#include "fj_accelerator.h"
#include "fj_interval.h"
#include "fj_numeric.h"
#include "fj_shader.h"
#include "fj_volume.h"
#include "fj_light.h"
#include "fj_ray.h"

#include <cassert>
#include <cstdio>
#include <cfloat>
#include <cmath>

namespace fj {

static int has_reached_bounce_limit(const struct TraceContext *cxt);
static int shadow_ray_has_reached_opcity_limit(const struct TraceContext *cxt, float opac);
static void setup_ray(const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax,
    struct Ray *ray);
static void setup_surface_input(
    const struct Intersection *isect,
    const struct Ray *ray,
    struct SurfaceInput *in);

static int trace_surface(const struct TraceContext *cxt, const struct Ray &ray,
    struct Color4 *out_rgba, double *t_hit);
static int raymarch_volume(const struct TraceContext *cxt, const struct Ray *ray,
    struct Color4 *out_rgba);

void SlFaceforward(const struct Vector *I, const struct Vector *N, struct Vector *Nf)
{
  if (Dot(*I, *N) < 0) {
    *Nf = *N;
    return;
  }
  Nf->x = -N->x;
  Nf->y = -N->y;
  Nf->z = -N->z;
}

double SlFresnel(const struct Vector *I, const struct Vector *N, double ior)
{
  double k2 = 0;
  double F0 = 0;
  double eta = 0;
  double cos = 0;

  // dot(-I, N)
  cos = -1 * Dot(*I, *N);
  if (cos > 0) {
    eta = ior;
  } else {
    eta = 1./ior;
    cos *= -1;
  }

  k2 = .0;
  F0 = ((1.-eta) * (1.-eta) + k2) / ((1.+eta) * (1.+eta) + k2);

  return F0 + (1. - F0) * pow(1. - cos, 5.);
}

double SlPhong(const struct Vector *I, const struct Vector *N, const struct Vector *L,
    double roughness)
{
  double spec = 0;
  struct Vector Lrefl;

  SlReflect(L, N, &Lrefl);

  spec = Dot(*I, Lrefl);
  spec = Max(0., spec);
  spec = pow(spec, 1/Max(.001, roughness));

  return spec;
}

void SlReflect(const struct Vector *I, const struct Vector *N, struct Vector *R)
{
  // dot(-I, N)
  const double cos = -1 * Dot(*I, *N);

  R->x = I->x + 2 * cos * N->x;
  R->y = I->y + 2 * cos * N->y;
  R->z = I->z + 2 * cos * N->z;
}

void SlRefract(const struct Vector *I, const struct Vector *N, double ior,
    struct Vector *T)
{
  struct Vector n;
  double radicand = 0;
  double ncoeff = 0;
  double cos1 = 0;
  double eta = 0;

  // dot(-I, N)
  cos1 = -1 * Dot(*I, *N);
  if (cos1 < 0) {
    cos1 *= -1;
    eta = 1/ior;
    n.x = -N->x;
    n.y = -N->y;
    n.z = -N->z;
  } else {
    eta = ior;
    n = *N;
  }

  radicand = 1 - eta*eta * (1 - cos1*cos1);

  if (radicand < 0.) {
    // total internal reflection
    n.x = -N->x;
    n.y = -N->y;
    n.z = -N->z;
    SlReflect(I, N, T);
    return;
  }

  ncoeff = eta * cos1 - sqrt(radicand);

  T->x = eta * I->x + ncoeff * n.x;
  T->y = eta * I->y + ncoeff * n.y;
  T->z = eta * I->z + ncoeff * n.z;
}

int SlTrace(const struct TraceContext *cxt,
    const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax, struct Color4 *out_rgba, double *t_hit)
{
  struct Ray ray;
  struct Color4 surface_color;
  struct Color4 volume_color;
  int hit_surface = 0;
  int hit_volume = 0;

  out_rgba->r = 0;
  out_rgba->g = 0;
  out_rgba->b = 0;
  out_rgba->a = 0;
  if (has_reached_bounce_limit(cxt)) {
    return 0;
  }

  setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);

  hit_surface = trace_surface(cxt, ray, &surface_color, t_hit);

  if (shadow_ray_has_reached_opcity_limit(cxt, surface_color.a)) {
    *out_rgba = surface_color;
    return 1;
  }

  if (hit_surface) {
    ray.tmax = *t_hit;
  }

  hit_volume = raymarch_volume(cxt, &ray, &volume_color);

  out_rgba->r = volume_color.r + surface_color.r * (1 - volume_color.a);
  out_rgba->g = volume_color.g + surface_color.g * (1 - volume_color.a);
  out_rgba->b = volume_color.b + surface_color.b * (1 - volume_color.a);
  out_rgba->a = volume_color.a + surface_color.a * (1 - volume_color.a);

  return hit_surface || hit_volume;
}

int SlSurfaceRayIntersect(const struct TraceContext *cxt,
    const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax,
    struct Vector *P_hit, struct Vector *N_hit, double *t_hit)
{
  const struct Accelerator *acc = NULL;
  struct Intersection isect;
  struct Ray ray;
  int hit = 0;

  setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);
  acc = cxt->trace_target->GetSurfaceAccelerator();
  hit = acc->Intersect(ray, cxt->time, &isect);

  if (hit) {
    *P_hit = isect.P;
    *N_hit = isect.N;
    *t_hit = isect.t_hit;
  }

  return hit;
}

struct TraceContext SlCameraContext(const struct ObjectGroup *target)
{
  struct TraceContext cxt;

  cxt.ray_context = CXT_CAMERA_RAY;
  cxt.reflect_depth = 0;
  cxt.refract_depth = 0;
  cxt.max_reflect_depth = 5;
  cxt.max_refract_depth = 5;
  cxt.cast_shadow = 1;
  cxt.trace_target = target;

  cxt.time = 0;

  cxt.opacity_threshold = .995f;
  cxt.raymarch_step = .05;
  cxt.raymarch_shadow_step = .05;
  cxt.raymarch_reflect_step = .05;
  cxt.raymarch_refract_step = .05;

  return cxt;
}

struct TraceContext SlReflectContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext refl_cxt = *cxt;

  refl_cxt.reflect_depth++;
  refl_cxt.ray_context = CXT_REFLECT_RAY;
  refl_cxt.trace_target = obj->GetReflectTarget();

  return refl_cxt;
}

struct TraceContext SlRefractContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext refr_cxt = *cxt;

  refr_cxt.refract_depth++;
  refr_cxt.ray_context = CXT_REFRACT_RAY;
  refr_cxt.trace_target = obj->GetRefractTarget();

  return refr_cxt;
}

struct TraceContext SlShadowContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext shad_cxt = *cxt;

  shad_cxt.ray_context = CXT_SHADOW_RAY;
  // turn off the secondary trance on occluding objects
  shad_cxt.max_reflect_depth = 0;
  shad_cxt.max_refract_depth = 0;
  shad_cxt.trace_target = obj->GetShadowTarget();

  return shad_cxt;
}

struct TraceContext SlSelfHitContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext self_cxt = *cxt;

  self_cxt.trace_target = obj->GetSelfHitTarget();

  return self_cxt;
}

int SlGetLightCount(const struct SurfaceInput *in)
{
  return in->shaded_object->GetLightCount();
}

int SlIlluminance(const struct TraceContext *cxt, const struct LightSample *sample,
    const struct Vector *Ps, const struct Vector *axis, double angle,
    const struct SurfaceInput *in, struct LightOutput *out)
{
  double cosangle = 0.;
  struct Vector nml_axis;
  struct Color light_color;

  out->Cl.r = 0;
  out->Cl.g = 0;
  out->Cl.b = 0;

  out->Ln.x = sample->P.x - Ps->x;
  out->Ln.y = sample->P.y - Ps->y;
  out->Ln.z = sample->P.z - Ps->z;

  out->distance = Length(out->Ln);
  if (out->distance > 0) {
    const double inv_dist = 1. / out->distance;
    out->Ln.x *= inv_dist;
    out->Ln.y *= inv_dist;
    out->Ln.z *= inv_dist;
  }

  nml_axis = *axis;
  Normalize(&nml_axis);
  cosangle = Dot(nml_axis, out->Ln);
  if (cosangle < cos(angle)) {
    return 0;
  }

  light_color = sample->light->Illuminate(*sample, *Ps);
  if (light_color.r < .0001 &&
    light_color.g < .0001 &&
    light_color.b < .0001) {
    return 0;
  }

  if (cxt->ray_context == CXT_SHADOW_RAY) {
    return 0;
  }

  if (cxt->cast_shadow) {
    struct TraceContext shad_cxt;
    struct Color4 C_occl;
    double t_hit = FLT_MAX;
    int hit = 0;

    shad_cxt = SlShadowContext(cxt, in->shaded_object);
    hit = SlTrace(&shad_cxt, Ps, &out->Ln, .0001, out->distance, &C_occl, &t_hit);

    if (hit) {
      // return 0;
      // TODO handle light_color for shadow ray
      const float alpha_complement = 1 - C_occl.a;
      light_color.r *= alpha_complement;
      light_color.g *= alpha_complement;
      light_color.b *= alpha_complement;
    }
  }

  out->Cl = light_color;
  return 1;
}

  // TODO temp solution compute before rendering
int SlGetLightSampleCount(const struct SurfaceInput *in)
{
  const struct Light **lights = in->shaded_object->GetLightList();
  const int nlights = SlGetLightCount(in);
  int nsamples = 0;
  int i;

  if (lights == NULL) {
    return 0;
  }

  for (i = 0; i < nlights; i++) {
    nsamples += lights[i]->GetSampleCount();
  }

  return nsamples;
}

struct LightSample *SlNewLightSamples(const struct SurfaceInput *in)
{
  const struct Light **lights = in->shaded_object->GetLightList();
  const int nlights = SlGetLightCount(in);
  const int nsamples = SlGetLightSampleCount(in);
  int i;

  struct LightSample *samples = NULL;
  struct LightSample *sample = NULL;

  if (nsamples == 0) {
    // TODO handling
    return NULL;
  }

  samples = new LightSample[nsamples];
  sample = samples;
  for (i = 0; i < nlights; i++) {
    const int nsmp = lights[i]->GetSampleCount();
    lights[i]->GetSamples(sample, nsmp);
    sample += nsmp;
  }

  return samples;
}

void SlFreeLightSamples(struct LightSample * samples)
{
  if (samples == NULL)
    return;
  delete [] samples;
}

#define MUL(a,val) do { \
  (a)->x *= (val); \
  (a)->y *= (val); \
  (a)->z *= (val); \
  } while(0)
void SlBumpMapping(const struct Texture *bump_map,
    const struct Vector *dPdu, const struct Vector *dPdv,
    const struct TexCoord *texcoord, double amplitude,
    const struct Vector *N, struct Vector *N_bump)
{
  struct Color4 C_tex0(0, 0, 0, 1);
  struct Color4 C_tex1(0, 0, 0, 1);
  struct Vector N_dPdu;
  struct Vector N_dPdv;
  float Bu, Bv;
  float du, dv;
  float val0, val1;
  const int xres = bump_map->GetWidth();
  const int yres = bump_map->GetHeight();
  
  if (xres == 0 || yres == 0) {
    return;
  }

  du = 1. / xres;
  dv = 1. / yres;

  // Bu = B(u - du, v) - B(v + du, v) / (2 * du)
  C_tex0 = bump_map->Lookup(texcoord->u - du, texcoord->v);
  C_tex1 = bump_map->Lookup(texcoord->u + du, texcoord->v);
  val0 = Luminance4(C_tex0);
  val1 = Luminance4(C_tex1);
  Bu = (val0 - val1) / (2 * du);

  // Bv = B(u, v - dv) - B(v, v + dv) / (2 * dv)
  C_tex0 = bump_map->Lookup(texcoord->u, texcoord->v - dv);
  C_tex1 = bump_map->Lookup(texcoord->u, texcoord->v + dv);
  val0 = Luminance4(C_tex0);
  val1 = Luminance4(C_tex1);
  Bv = (val0 - val1) / (2 * dv);

  // N ~= N + Bv(N x Pu) + Bu(N x Pv)
  N_dPdu = Cross(*N, *dPdu);
  N_dPdv = Cross(*N, *dPdv);
  MUL(&N_dPdu, du);
  MUL(&N_dPdv, du);
  N_bump->x = N->x + amplitude * (Bv * N_dPdu.x - Bu * N_dPdv.x);
  N_bump->y = N->y + amplitude * (Bv * N_dPdu.y - Bu * N_dPdv.y);
  N_bump->z = N->z + amplitude * (Bv * N_dPdu.z - Bu * N_dPdv.z);

  Normalize(N_bump);
}
#undef MUL

static int has_reached_bounce_limit(const struct TraceContext *cxt)
{
  int current_depth = 0;
  int max_depth = 0;

  switch (cxt->ray_context) {
  case CXT_CAMERA_RAY:
    current_depth = 0;
    max_depth = 1;
    break;
  case CXT_SHADOW_RAY:
    current_depth = 0;
    max_depth = 1;
    break;
  case CXT_REFLECT_RAY:
    current_depth = cxt->reflect_depth;
    max_depth = cxt->max_reflect_depth;
    break;
  case CXT_REFRACT_RAY:
    current_depth = cxt->refract_depth;
    max_depth = cxt->max_refract_depth;
    break;
  default:
    assert(!"invalid ray type");
    break;
  }

  return current_depth > max_depth;
}

static void setup_ray(const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax,
    struct Ray *ray)
{
  ray->orig = *ray_orig;
  ray->dir = *ray_dir;
  ray->tmin = ray_tmin;
  ray->tmax = ray_tmax;
}

static void setup_surface_input(
    const struct Intersection *isect,
    const struct Ray *ray,
    struct SurfaceInput *in)
{
  in->shaded_object = isect->object;
  in->P  = isect->P;
  in->N  = isect->N;
  in->Cd = isect->Cd;
  in->uv = isect->uv;
  in->I  = ray->dir;

  in->dPdu = isect->dPdu;
  in->dPdv = isect->dPdv;
}

static int trace_surface(const struct TraceContext *cxt, const struct Ray &ray,
    struct Color4 *out_rgba, double *t_hit)
{
  const struct Accelerator *acc = NULL;
  struct Intersection isect;
  int hit = 0;

  out_rgba->r = 0;
  out_rgba->g = 0;
  out_rgba->b = 0;
  out_rgba->a = 0;
  acc = cxt->trace_target->GetSurfaceAccelerator();
  hit = acc->Intersect(ray, cxt->time, &isect);

  // TODO handle shadow ray for surface geometry
#if 0
  if (cxt->ray_context == CXT_SHADOW_RAY) {
    return hit;
  }
#endif

  if (hit) {
    struct SurfaceInput in;
    struct SurfaceOutput out;

    setup_surface_input(&isect, &ray, &in);
    isect.object->GetShader()->Evaluate(*cxt, in, &out);

    out.Os = Clamp(out.Os, 0, 1);
    out_rgba->r = out.Cs.r;
    out_rgba->g = out.Cs.g;
    out_rgba->b = out.Cs.b;
    out_rgba->a = out.Os;

    *t_hit = isect.t_hit;
  }

  return hit;
}

static int raymarch_volume(const struct TraceContext *cxt, const struct Ray *ray,
    struct Color4 *out_rgba)
{
  const struct VolumeAccelerator *acc = NULL;
  IntervalList intervals;
  int hit = 0;

  out_rgba->r = 0;
  out_rgba->g = 0;
  out_rgba->b = 0;
  out_rgba->a = 0;

  acc = cxt->trace_target->GetVolumeAccelerator();
  hit = VolumeAccIntersect(acc, cxt->time, ray, &intervals);

  if (!hit) {
    return 0;
  }

  {
    struct Vector P;
    struct Vector ray_delta;
    double t = 0, t_start = 0, t_delta = 0, t_limit = 0;
    const float opacity_threshold = cxt->opacity_threshold;

    // t properties
    switch (cxt->ray_context) {
    case CXT_CAMERA_RAY:
      t_delta = cxt->raymarch_step;
      break;
    case CXT_SHADOW_RAY:
      t_delta = cxt->raymarch_shadow_step;
      break;
    case CXT_REFLECT_RAY:
      t_delta = cxt->raymarch_reflect_step;
      break;
    case CXT_REFRACT_RAY:
      t_delta = cxt->raymarch_refract_step;
      break;
    default:
      t_delta = cxt->raymarch_step;
      break;
    }
    t_limit = intervals.GetMaxT();
    t_limit = Min(t_limit, ray->tmax);

    t_start = intervals.GetMinT();
    if (t_start < 0) {
      t_start = t_delta;
    }
    else {
      t_start = t_start - fmod(t_start, t_delta) + t_delta;
    }

    P = RayPointAt(*ray, t_start);
    ray_delta.x = t_delta * ray->dir.x;
    ray_delta.y = t_delta * ray->dir.y;
    ray_delta.z = t_delta * ray->dir.z;
    t = t_start;

    // raymarch
    while (t <= t_limit && out_rgba->a < opacity_threshold) {
      const struct Interval *interval = intervals.GetHead();
      struct Color color;
      float opacity = 0;

      // loop over volume candidates at this sample point
      for (; interval != NULL; interval = interval->next) {
        struct VolumeSample sample;
        interval->object->GetVolumeSample(P, cxt->time, &sample);

        // merge volume with max density
        opacity = Max(opacity, t_delta * sample.density);

        if (cxt->ray_context != CXT_SHADOW_RAY) {
          struct SurfaceInput in;
          struct SurfaceOutput out;

          in.shaded_object = interval->object;
          in.P = P;
          in.N = Vector(0, 0, 0);
          interval->object->GetShader()->Evaluate(*cxt, in, &out);

          color.r = out.Cs.r * opacity;
          color.g = out.Cs.g * opacity;
          color.b = out.Cs.b * opacity;
        }
      }

      // composite color
      out_rgba->r = out_rgba->r + color.r * (1-out_rgba->a);
      out_rgba->g = out_rgba->g + color.g * (1-out_rgba->a);
      out_rgba->b = out_rgba->b + color.b * (1-out_rgba->a);
      out_rgba->a = out_rgba->a + Clamp(opacity, 0, 1) * (1-out_rgba->a);

      // advance sample point
      P.x += ray_delta.x;
      P.y += ray_delta.y;
      P.z += ray_delta.z;
      t += t_delta;
    }
    if (out_rgba->a >= opacity_threshold) {
      out_rgba->a = 1;
    }
  }
  out_rgba->a = Clamp(out_rgba->a, 0, 1);

  return hit;
}

static int shadow_ray_has_reached_opcity_limit(const struct TraceContext *cxt, float opac)
{
  if (cxt->ray_context == CXT_SHADOW_RAY && opac > cxt->opacity_threshold) {
    return 1;
  } else {
    return 0;
  }
}

} // namespace xxx
