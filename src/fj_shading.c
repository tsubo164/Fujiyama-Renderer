/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_shading.h"
#include "fj_volume_accelerator.h"
#include "fj_object_instance.h"
#include "fj_intersection.h"
#include "fj_object_group.h"
#include "fj_accelerator.h"
#include "fj_interval.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_shader.h"
#include "fj_volume.h"
#include "fj_light.h"
#include "fj_ray.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

static int has_reached_bounce_limit(const struct TraceContext *cxt);
static int shadow_ray_has_reached_opcity_limit(const struct TraceContext *cxt, float opac);
static void setup_ray(const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax,
    struct Ray *ray);
static void setup_surface_input(
    const struct Intersection *isect,
    const struct Ray *ray,
    struct SurfaceInput *in);

static int trace_surface(const struct TraceContext *cxt, const struct Ray *ray,
    struct Color4 *out_rgba, double *t_hit);
static int raymarch_volume(const struct TraceContext *cxt, const struct Ray *ray,
    struct Color4 *out_rgba);

double SlFresnel(const struct Vector *I, const struct Vector *N, double ior)
{
  double k2 = 0;
  double F0 = 0;
  double eta = 0;
  double cos = 0;

  /* dot(-I, N) */
  cos = -1 * VEC3_DOT(I, N);
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
  struct Vector Lrefl = {0, 0, 0};

  SlReflect(L, N, &Lrefl);

  spec = VEC3_DOT(I, &Lrefl);
  spec = MAX(0, spec);
  spec = pow(spec, 1/MAX(.001, roughness));

  return spec;
}

void SlReflect(const struct Vector *I, const struct Vector *N, struct Vector *R)
{
  /* dot(-I, N) */
  const double cos = -1 * VEC3_DOT(I, N);

  R->x = I->x + 2 * cos * N->x;
  R->y = I->y + 2 * cos * N->y;
  R->z = I->z + 2 * cos * N->z;
}

void SlRefract(const struct Vector *I, const struct Vector *N, double ior,
    struct Vector *T)
{
  struct Vector n = {0, 0, 0};
  double radicand = 0;
  double ncoeff = 0;
  double cos1 = 0;
  double eta = 0;

  /* dot(-I, N) */
  cos1 = -1 * VEC3_DOT(I, N);
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
    /* total internal reflection */
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
  struct Ray ray = {{0}};
  struct Color4 surface_color = {0, 0, 0, 0};
  struct Color4 volume_color = {0, 0, 0, 0};
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

  hit_surface = trace_surface(cxt, &ray, &surface_color, t_hit);

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
  struct Intersection isect = {{0}};
  struct Ray ray = {{0}};
  int hit = 0;

  setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);
  acc = ObjGroupGetSurfaceAccelerator(cxt->trace_target),
  hit = AccIntersect(acc, cxt->time, &ray, &isect);

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
  refl_cxt.trace_target = ObjGetReflectTarget(obj);

  return refl_cxt;
}

struct TraceContext SlRefractContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext refr_cxt = *cxt;

  refr_cxt.refract_depth++;
  refr_cxt.ray_context = CXT_REFRACT_RAY;
  refr_cxt.trace_target = ObjGetRefractTarget(obj);

  return refr_cxt;
}

struct TraceContext SlShadowContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext shad_cxt = *cxt;

  shad_cxt.ray_context = CXT_SHADOW_RAY;
  /* turn off the secondary trance on occluding objects */
  shad_cxt.max_reflect_depth = 0;
  shad_cxt.max_refract_depth = 0;
  shad_cxt.trace_target = ObjGetShadowTarget(obj);

  return shad_cxt;
}

struct TraceContext SlSelfHitContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj)
{
  struct TraceContext self_cxt = *cxt;

  self_cxt.trace_target = ObjGetSelfHitTarget(obj);

  return self_cxt;
}

int SlGetLightCount(const struct SurfaceInput *in)
{
  return ObjGetLightCount(in->shaded_object);
}

int SlIlluminance(const struct TraceContext *cxt, const struct LightSample *sample,
    const struct Vector *Ps, const struct Vector *axis, double angle,
    const struct SurfaceInput *in, struct LightOutput *out)
{
  double cosangle = 0.;
  struct Vector nml_axis = {0, 0, 0};
  struct Color light_color = {0, 0, 0};

  out->Cl.r = 0;
  out->Cl.g = 0;
  out->Cl.b = 0;

  out->Ln.x = sample->P.x - Ps->x;
  out->Ln.y = sample->P.y - Ps->y;
  out->Ln.z = sample->P.z - Ps->z;

  out->distance = VEC3_LEN(&out->Ln);
  if (out->distance > 0) {
    const double inv_dist = 1. / out->distance;
    out->Ln.x *= inv_dist;
    out->Ln.y *= inv_dist;
    out->Ln.z *= inv_dist;
  }

  nml_axis = *axis;
  VEC3_NORMALIZE(&nml_axis);
  cosangle = VEC3_DOT(&nml_axis, &out->Ln);
  if (cosangle < cos(angle)) {
    return 0;
  }

  LgtIlluminate(sample, Ps, &light_color);
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
    struct Color4 C_occl = {0, 0, 0, 0};
    double t_hit = FLT_MAX;
    int hit = 0;

    shad_cxt = SlShadowContext(cxt, in->shaded_object);
    hit = SlTrace(&shad_cxt, Ps, &out->Ln, .0001, out->distance, &C_occl, &t_hit);

    if (hit) {
      /* return 0; */
      /* TODO handle light_color for shadow ray */
      const float alpha_complement = 1 - C_occl.a;
      light_color.r *= alpha_complement;
      light_color.g *= alpha_complement;
      light_color.b *= alpha_complement;
    }
  }

  out->Cl = light_color;
  return 1;
}

  /* TODO temp solution compute before rendering */
int SlGetLightSampleCount(const struct SurfaceInput *in)
{
  const struct Light **lights = ObjGetLightList(in->shaded_object);
  const int nlights = SlGetLightCount(in);
  int nsamples = 0;
  int i;

  if (lights == NULL) {
    return 0;
  }

  for (i = 0; i < nlights; i++) {
    nsamples += LgtGetSampleCount(lights[i]);
  }

  return nsamples;
}

struct LightSample *SlNewLightSamples(const struct SurfaceInput *in)
{
  const struct Light **lights = ObjGetLightList(in->shaded_object);
  const int nlights = SlGetLightCount(in);
  const int nsamples = SlGetLightSampleCount(in);
  int i;

  struct LightSample *samples = NULL;
  struct LightSample *sample = NULL;

  if (nsamples == 0) {
    /* TODO handling */
    return NULL;
  }

  samples = FJ_MEM_ALLOC_ARRAY(struct LightSample, nsamples);
  sample = samples;
  for (i = 0; i < nlights; i++) {
    const int nsmp = LgtGetSampleCount(lights[i]);
    LgtGetSamples(lights[i], sample, nsmp);
    sample += nsmp;
  }

  return samples;
}

void SlFreeLightSamples(struct LightSample * samples)
{
  if (samples == NULL)
    return;
  FJ_MEM_FREE(samples);
}

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

  in->dPds = isect->dPds;
  in->dPdt = isect->dPdt;
}

static int trace_surface(const struct TraceContext *cxt, const struct Ray *ray,
    struct Color4 *out_rgba, double *t_hit)
{
  const struct Accelerator *acc = NULL;
  struct Intersection isect = {{0}};
  int hit = 0;

  out_rgba->r = 0;
  out_rgba->g = 0;
  out_rgba->b = 0;
  out_rgba->a = 0;
  acc = ObjGroupGetSurfaceAccelerator(cxt->trace_target),
  hit = AccIntersect(acc, cxt->time, ray, &isect);

  /* TODO handle shadow ray for surface geometry */
  /*
  if (cxt->ray_context == CXT_SHADOW_RAY) {
    return hit;
  }
  */

  if (hit) {
    struct SurfaceInput in;
    struct SurfaceOutput out;

    setup_surface_input(&isect, ray, &in);
    ShdEvaluate(ObjGetShader(isect.object), cxt, &in, &out);

    out.Os = CLAMP(out.Os, 0, 1);
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
  struct IntervalList *intervals = NULL;
  int hit = 0;

  out_rgba->r = 0;
  out_rgba->g = 0;
  out_rgba->b = 0;
  out_rgba->a = 0;

  intervals = IntervalListNew();
  if (intervals == NULL) {
    /* TODO error handling */
    return 0;
  }

  acc = ObjGroupGetVolumeAccelerator(cxt->trace_target);
  hit = VolumeAccIntersect(acc, cxt->time, ray, intervals);

  if (!hit) {
    IntervalListFree(intervals);
    return 0;
  }

  {
    struct Vector P = {0, 0, 0};
    struct Vector ray_delta = {0, 0, 0};
    double t = 0, t_start = 0, t_delta = 0, t_limit = 0;
    const float opacity_threshold = cxt->opacity_threshold;

    /* t properties */
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
    t_limit = IntervalListGetMaxT(intervals);
    t_limit = MIN(t_limit, ray->tmax);

    t_start = IntervalListGetMinT(intervals);
    if (t_start < 0) {
      t_start = t_delta;
    }
    else {
      t_start = t_start - fmod(t_start, t_delta) + t_delta;
    }

    POINT_ON_RAY(&P, &ray->orig, &ray->dir, t_start);
    ray_delta.x = t_delta * ray->dir.x;
    ray_delta.y = t_delta * ray->dir.y;
    ray_delta.z = t_delta * ray->dir.z;
    t = t_start;

    /* raymarch */
    while (t <= t_limit && out_rgba->a < opacity_threshold) {
      const struct Interval *interval = IntervalListGetHead(intervals);
      struct Color color = {0, 0, 0};
      float opacity = 0;

      /* loop over volume candidates at this sample point */
      for (; interval != NULL; interval = interval->next) {
        struct VolumeSample sample;
        ObjGetVolumeSample(interval->object, cxt->time, &P, &sample);

        /* merge volume with max density */
        opacity = MAX(opacity, t_delta * sample.density);

        if (cxt->ray_context != CXT_SHADOW_RAY) {
          struct SurfaceInput in;
          struct SurfaceOutput out;

          in.shaded_object = interval->object;
          in.P = P;
          VEC3_SET(&in.N, 0, 0, 0);
          ShdEvaluate(ObjGetShader(interval->object), cxt, &in, &out);

          color.r = out.Cs.r * opacity;
          color.g = out.Cs.g * opacity;
          color.b = out.Cs.b * opacity;
        }
      }

      /* composite color */
      out_rgba->r = out_rgba->r + color.r * (1-out_rgba->a);
      out_rgba->g = out_rgba->g + color.g * (1-out_rgba->a);
      out_rgba->b = out_rgba->b + color.b * (1-out_rgba->a);
      out_rgba->a = out_rgba->a + CLAMP(opacity, 0, 1) * (1-out_rgba->a);

      /* advance sample point */
      P.x += ray_delta.x;
      P.y += ray_delta.y;
      P.z += ray_delta.z;
      t += t_delta;
    }
    if (out_rgba->a >= opacity_threshold) {
      out_rgba->a = 1;
    }
  }
  out_rgba->a = CLAMP(out_rgba->a, 0, 1);

  IntervalListFree(intervals);
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

