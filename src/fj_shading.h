/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_SHADING_H
#define FJ_SHADING_H

#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;
struct ObjectGroup;
struct Texture;

enum RayContext {
  CXT_CAMERA_RAY = 0,
  CXT_SHADOW_RAY,
  CXT_REFLECT_RAY,
  CXT_REFRACT_RAY
};

struct TraceContext {
  int ray_context;
  int reflect_depth;
  int refract_depth;
  int max_reflect_depth;
  int max_refract_depth;
  int cast_shadow;

  double time;

  float opacity_threshold;
  double raymarch_step;
  double raymarch_shadow_step;
  double raymarch_reflect_step;
  double raymarch_refract_step;

  const struct ObjectGroup *trace_target;
};

struct SurfaceInput {
  struct Vector P;
  struct Vector N;
  struct Color Cd;
  struct TexCoord uv;
  float Alpha;

  struct Vector Ng;
  struct Vector I;

  struct Vector dPdu;
  struct Vector dPdv;

  const struct ObjectInstance *shaded_object;
};

struct SurfaceOutput {
  struct Color Cs;
  float Os;
};

struct LightOutput {
  struct Color Cl;
  struct Color Ol;
  struct Vector Ln;
  double distance;
};

/* tracing functions */
extern void SlFaceforward(const struct Vector *I, const struct Vector *N, struct Vector *Nf);
extern double SlFresnel(const struct Vector *I, const struct Vector *N, double ior);
extern double SlPhong(const struct Vector *I, const struct Vector *N, const struct Vector *L,
    double roughness);

extern void SlReflect(const struct Vector *I, const struct Vector *N, struct Vector *R);
extern void SlRefract(const struct Vector *I, const struct Vector *N, double ior,
    struct Vector *T);

extern int SlTrace(const struct TraceContext *cxt,
    const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax, struct Color4 *out_color, double *t_hit);
extern int SlSurfaceRayIntersect(const struct TraceContext *cxt,
    const struct Vector *ray_orig, const struct Vector *ray_dir,
    double ray_tmin, double ray_tmax,
    struct Vector *P_hit, struct Vector *N_hit, double *t_hit);

extern struct TraceContext SlCameraContext(const struct ObjectGroup *target);
extern struct TraceContext SlReflectContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj);
extern struct TraceContext SlRefractContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj);
extern struct TraceContext SlShadowContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj);
extern struct TraceContext SlSelfHitContext(const struct TraceContext *cxt,
    const struct ObjectInstance *obj);

/* lighting functions */
struct LightSample;

extern int SlIlluminance(const struct TraceContext *cxt, const struct LightSample *sample,
    const struct Vector *Ps, const struct Vector *axis, double angle,
    const struct SurfaceInput *in, struct LightOutput *out);

extern int SlGetLightCount(const struct SurfaceInput *in);
extern int SlGetLightSampleCount(const struct SurfaceInput *in);
extern struct LightSample *SlNewLightSamples(const struct SurfaceInput *in);
extern void SlFreeLightSamples(struct LightSample * samples);

/* texture functions */
extern void SlBumpMapping(const struct Texture *bump_map,
    const struct Vector *dPdu, const struct Vector *dPdv,
    const struct TexCoord *texcoord, double amplitude,
    const struct Vector *N, struct Vector *N_bump);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
