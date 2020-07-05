// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SHADING_H
#define FJ_SHADING_H

#include "fj_compatibility.h"
#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"

namespace fj {

class ObjectInstance;
class ObjectGroup;
class Texture;

enum RayContext {
  CXT_CAMERA_RAY = 0,
  CXT_SHADOW_RAY,
  CXT_DIFFUSE_RAY,
  CXT_REFLECT_RAY,
  CXT_REFRACT_RAY
};

class FJ_API TraceContext {
public:
  int ray_context;
  int diffuse_depth;
  int reflect_depth;
  int refract_depth;
  int max_diffuse_depth;
  int max_reflect_depth;
  int max_refract_depth;
  int cast_shadow;

  double time;

  float opacity_threshold;
  double raymarch_step;
  double raymarch_shadow_step;
  double raymarch_diffuse_step;
  double raymarch_reflect_step;
  double raymarch_refract_step;

  const ObjectGroup *trace_target;
};

class FJ_API SurfaceInput {
public:
  Vector P;
  Vector N;
  Color Cd;
  TexCoord uv;
  float Alpha;

  Vector Ng;
  Vector I;

  Vector dPdu;
  Vector dPdv;

  const ObjectInstance *shaded_object;
};

class FJ_API SurfaceOutput {
public:
  Color Cs;
  float Os;
};

class FJ_API LightOutput {
public:
  Color Cl;
  Color Ol;
  Vector Ln;
  double distance;
};

// tracing functions
FJ_API void SlFaceforward(const Vector *I, const Vector *N, Vector *Nf);
FJ_API double SlFresnel(const Vector *I, const Vector *N, double ior);
FJ_API double SlPhong(const Vector *I, const Vector *N, const Vector *L,
    double roughness);

FJ_API void SlReflect(const Vector *I, const Vector *N, Vector *R);
FJ_API void SlRefract(const Vector *I, const Vector *N, double ior,
    Vector *T);

FJ_API int SlTrace(const TraceContext *cxt,
    const Vector *ray_orig, const Vector *ray_dir,
    double ray_tmin, double ray_tmax, Color4 *out_color, double *t_hit);
FJ_API int SlSurfaceRayIntersect(const TraceContext *cxt,
    const Vector *ray_orig, const Vector *ray_dir,
    double ray_tmin, double ray_tmax,
    Vector *P_hit, Vector *N_hit, double *t_hit);

FJ_API TraceContext SlCameraContext(const ObjectGroup *target);
FJ_API TraceContext SlDiffuseContext(const TraceContext *cxt,
    const ObjectInstance *obj);
FJ_API TraceContext SlReflectContext(const TraceContext *cxt,
    const ObjectInstance *obj);
FJ_API TraceContext SlRefractContext(const TraceContext *cxt,
    const ObjectInstance *obj);
FJ_API TraceContext SlShadowContext(const TraceContext *cxt,
    const ObjectInstance *obj);
FJ_API TraceContext SlSelfHitContext(const TraceContext *cxt,
    const ObjectInstance *obj);

// lighting functions
class LightSample;

FJ_API int SlIlluminance(const TraceContext *cxt, const LightSample *sample,
    const Vector *Ps, const Vector *axis, double angle,
    const SurfaceInput *in, LightOutput *out);

FJ_API int SlGetLightCount(const SurfaceInput *in);
FJ_API int SlGetLightSampleCount(const SurfaceInput *in);
FJ_API LightSample *SlNewLightSamples(const SurfaceInput *in);
FJ_API void SlFreeLightSamples(LightSample * samples);

// texture functions
FJ_API void SlBumpMapping(const Texture *bump_map,
    const Vector *dPdu, const Vector *dPdv,
    const TexCoord *texcoord, double amplitude,
    const Vector *N, Vector *N_bump);

} // namespace xxx

#endif // FJ_XXX_H
