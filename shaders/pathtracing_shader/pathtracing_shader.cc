// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_multi_thread.h"

using namespace fj;

enum SurfaceType {
  SURFACE_EMISSION,
  SURFACE_DIFFUSE,
  SURFACE_SPECULAR,
  SURFACE_REFRACT
};

class PathtracingShader : public Shader {
public:
  PathtracingShader() : surface_type(SURFACE_DIFFUSE) {}
  virtual ~PathtracingShader() {}

public:
  int surface_type;
  Color emission;
  Color diffuse;
  Color specular;
  Color ambient;
  float roughness;

  Color reflect;
  Color refract;
  float ior;

  float opacity;

  int do_reflect;

  Texture *diffuse_map;
  Texture *bump_map;
  float bump_amplitude;

private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  void evaluate_diffuse(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  void evaluate_specular(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  void evaluate_refract(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  mutable XorShift rng[16];
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "PathtracingShader";

static int set_emission(void *self, const PropertyValue &value);
static int set_diffuse(void *self, const PropertyValue &value);
static int set_specular(void *self, const PropertyValue &value);
static int set_ambient(void *self, const PropertyValue &value);
static int set_roughness(void *self, const PropertyValue &value);
static int set_reflect(void *self, const PropertyValue &value);
static int set_refract(void *self, const PropertyValue &value);
static int set_ior(void *self, const PropertyValue &value);
static int set_opacity(void *self, const PropertyValue &value);
static int set_diffuse_map(void *self, const PropertyValue &value);
static int set_bump_map(void *self, const PropertyValue &value);
static int set_bump_amplitude(void *self, const PropertyValue &value);

static const Property MyPropertyList[] = {
  Property("emission",       PropVector3(0, 0, 0),    set_emission),
  Property("diffuse",        PropVector3(.8, .8, .8), set_diffuse),
  Property("specular",       PropVector3(1, 1, 1),    set_specular),
  Property("ambient",        PropVector3(1, 1, 1),    set_ambient),
  Property("roughness",      PropScalar(.1),          set_roughness),
  Property("reflect",        PropVector3(0, 0, 0),    set_reflect),
  Property("refract",        PropVector3(0, 0, 0),    set_refract),
  Property("ior",            PropScalar(1.4),         set_ior),
  Property("opacity",        PropScalar(1),           set_opacity),
  Property("diffuse_map",    PropTexture(NULL),       set_diffuse_map),
  Property("bump_map",       PropTexture(NULL),       set_bump_map),
  Property("bump_amplitude", PropScalar(1),           set_bump_amplitude),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A pathtracing shader."},
  {"plugin_type", "Shader"},
  {NULL, NULL}
};

extern "C" {
FJ_PLUGIN_API int Initialize(PluginInfo *info)
{
  return PlgSetupInfo(info,
      PLUGIN_API_VERSION,
      SHADER_PLUGIN_TYPE,
      MyPluginName,
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  PathtracingShader *pathtracing = new PathtracingShader();
  PropSetAllDefaultValues(pathtracing, MyPropertyList);

  return pathtracing;
}

static void MyDeleteFunction(void *self)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  if (pathtracing == NULL) {
    return;
  }
  delete pathtracing;
}

void PathtracingShader::evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
#if 0
  Color diff;
  Color spec;
  Color4 diff_map(1, 1, 1, 1);
  Vector Nf;
  int i = 0;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(&in);

  SlFaceforward(&in.I, &in.N, &Nf);

  // bump map
  if (bump_map != NULL) {
    Vector N_bump;
    SlBumpMapping(bump_map,
        &in.dPdu, &in.dPdv,
        &in.uv, bump_amplitude,
        &Nf, &N_bump);
    Nf = N_bump;
  }

  // allocate samples
  samples = SlNewLightSamples(&in);

  for (i = 0; i < nsamples; i++) {
    LightOutput Lout;
    float Kd = 0;
    SlIlluminance(&cxt, &samples[i], &in.P, &Nf, PI / 2., &in, &Lout);
    // spec
#if 0
    Ks = SlPhong(in.I, Nf, Ln, .05);
#endif

    // diff
    Kd = Dot(Nf, Lout.Ln);
    Kd = Max(0, Kd);
    diff.r += Kd * Lout.Cl.r;
    diff.g += Kd * Lout.Cl.g;
    diff.b += Kd * Lout.Cl.b;
  }

  // free samples
  SlFreeLightSamples(samples);

  // diffuse map
  if (diffuse_map != NULL) {
    diff_map = diffuse_map->Lookup(in.uv.u, in.uv.v);
  }

  // Cs
  out->Cs.r = diff.r * diffuse.r * diff_map.r + spec.r;
  out->Cs.g = diff.g * diffuse.g * diff_map.g + spec.g;
  out->Cs.b = diff.b * diffuse.b * diff_map.b + spec.b;

  // reflect
  if (do_reflect) {
    Color4 C_refl;
    Vector R;
    double t_hit = REAL_MAX;
    double Kr = 0;

    const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);

    SlReflect(&in.I, &Nf, &R);
    Normalize(&R);
    SlTrace(&refl_cxt, &in.P, &R, .001, 1000, &C_refl, &t_hit);

    Kr = SlFresnel(&in.I, &Nf, 1/ior);
    out->Cs.r += Kr * C_refl.r * reflect.r;
    out->Cs.g += Kr * C_refl.g * reflect.g;
    out->Cs.b += Kr * C_refl.b * reflect.b;
  }

  out->Os = 1;
  out->Os = opacity;
#endif

  switch (surface_type) {
  case SURFACE_EMISSION:
    out->Cs = emission;
    out->Os = 1;
    break;
  case SURFACE_DIFFUSE:
    evaluate_diffuse(cxt, in, out);
    break;
  case SURFACE_SPECULAR:
    evaluate_specular(cxt, in, out);
    break;
  case SURFACE_REFRACT:
    evaluate_refract(cxt, in, out);
    break;
  default:
    break;
  }

  return;
}

void PathtracingShader::evaluate_diffuse(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  Vector u, v, w;
  w = in.N;
  u = Abs(w.x) > .001 ? Vector(0, 1, 0) : Vector(1, 0, 0);
  u = Cross(u, w);
  u = Normalize(&u); // TODO fix Normalize(const Vector *)
  v = Cross(w, u);

  const int id = MtGetThreadID();
  const Real r1 = 2. * PI * rng[id].NextFloat01();
  const Real r2 = rng[id].NextFloat01();
  const Real r2sqrt = Sqrt(r2);
  const Real prob = Luminance(diffuse);

  Vector dir = 
    u * cos(r1) * r2sqrt +
    v * sin(r1) * r2sqrt +
    w * sqrt(1. - r2);
  dir = Normalize(&dir); // TODO fix Normalize(const Vector *)

  const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);
  Color4 Lo;
  Real t_hit = REAL_MAX;

  SlTrace(&refl_cxt, &in.P, &dir, .001, 1000, &Lo, &t_hit);
  const Color weight = diffuse / prob;

  out->Cs = emission + weight * ToColor(Lo);
  out->Os = 1.0;
}

void PathtracingShader::evaluate_specular(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  const Real prob = Luminance(reflect);
  Vector dir;
  SlReflect(&in.I, &in.N, &dir);
  Normalize(&dir);

  const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);
  Color4 Lo;
  Real t_hit = REAL_MAX;

  SlTrace(&refl_cxt, &in.P, &dir, .001, 1000, &Lo, &t_hit);
  const Color weight = reflect / prob;

  out->Cs = emission + weight * ToColor(Lo);
  out->Os = 1.0;
}

void PathtracingShader::evaluate_refract(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  /*
  const Real nc = 1.;
  const Real nt = ior;
  */
  const int id = MtGetThreadID();
  const Real prob = Luminance(refract);
  
  Vector R, T;
  SlReflect(&in.I, &in.N, &R);
  Normalize(&R);
  SlRefract(&in.I, &in.N, 1/ior, &T);
  Normalize(&T);

  Real t_hit = REAL_MAX;
  Color4 C_refl;
  Color4 C_refr;

  double Kr, Kt;
  Kr = SlFresnel(&in.I, &in.N, 1/ior);
  Kt = 1 - Kr;

  if ((cxt.reflect_depth == 0 && cxt.refract_depth == 0) || false) {
    // reflect
    const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);
    SlTrace(&refl_cxt, &in.P, &R, .0001, 1000, &C_refl, &t_hit);
    // refract
    const TraceContext refr_cxt = SlRefractContext(&cxt, in.shaded_object);
    SlTrace(&refr_cxt, &in.P, &T, .0001, 1000, &C_refr, &t_hit);

    const Color4 Lo = Kr * C_refl + Kt * C_refr;
    const Color weight = refract / prob;

    out->Cs = emission + weight * ToColor(Lo);
    out->Os = 1.0;
  } else {
    const Real p = .25 + .5 * Kr;
    if (rng[id].NextFloat01() < p) {
      // reflect
      const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);
      SlTrace(&refl_cxt, &in.P, &R, .0001, 1000, &C_refl, &t_hit);
      const Color weight = reflect / (p * prob);
      const Color4 Lo = Kr * C_refl;
      out->Cs = emission + weight * ToColor(Lo);
      out->Os = 1.0;
    } else {
      // refract
      const TraceContext refr_cxt = SlRefractContext(&cxt, in.shaded_object);
      SlTrace(&refr_cxt, &in.P, &T, .0001, 1000, &C_refr, &t_hit);
      const Color weight = refract / ((1 - p) * prob);
      const Color4 Lo = Kt * C_refr;
      out->Cs = emission + weight * ToColor(Lo);
      out->Os = 1.0;
    }
  }
}

static int set_emission(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color emission;

  emission.r = Max(0, value.vector[0]);
  emission.g = Max(0, value.vector[1]);
  emission.b = Max(0, value.vector[2]);

  if (emission.r >= 1 || emission.g >= 1 || emission.b >= 1) {
    pathtracing->surface_type = SURFACE_EMISSION;
  } else {
    pathtracing->surface_type = SURFACE_DIFFUSE;
  }
  pathtracing->emission = emission;

  return 0;
}

static int set_diffuse(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value.vector[0]);
  diffuse.g = Max(0, value.vector[1]);
  diffuse.b = Max(0, value.vector[2]);
  pathtracing->surface_type = SURFACE_DIFFUSE;
  pathtracing->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color specular;

  specular.r = Max(0, value.vector[0]);
  specular.g = Max(0, value.vector[1]);
  specular.b = Max(0, value.vector[2]);
  pathtracing->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color ambient;

  ambient.r = Max(0, value.vector[0]);
  ambient.g = Max(0, value.vector[1]);
  ambient.b = Max(0, value.vector[2]);
  pathtracing->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  float roughness = value.vector[0];

  roughness = Max(0, roughness);
  pathtracing->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color reflect;

  reflect.r = Max(0, value.vector[0]);
  reflect.g = Max(0, value.vector[1]);
  reflect.b = Max(0, value.vector[2]);

  if (reflect.r > 0 || reflect.g > 0 || reflect.b > 0 ) {
    pathtracing->surface_type = SURFACE_SPECULAR;
  }
  else {
    pathtracing->surface_type = SURFACE_DIFFUSE;
  }
  pathtracing->reflect = reflect;

  return 0;
}

static int set_refract(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color refract;

  refract.r = Max(0, value.vector[0]);
  refract.g = Max(0, value.vector[1]);
  refract.b = Max(0, value.vector[2]);

  if (refract.r > 0 || refract.g > 0 || refract.b > 0 ) {
    pathtracing->surface_type = SURFACE_REFRACT;
  }
  else {
    pathtracing->surface_type = SURFACE_DIFFUSE;
  }
  pathtracing->refract = refract;

  return 0;
}

static int set_ior(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  float ior = value.vector[0];

  ior = Max(.001, ior);
  pathtracing->ior = ior;

  return 0;
}

static int set_opacity(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  float opacity = value.vector[0];

  opacity = Clamp(opacity, 0, 1);
  pathtracing->opacity = opacity;

  return 0;
}

static int set_diffuse_map(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;

  pathtracing->diffuse_map = value.texture;

  return 0;
}

static int set_bump_map(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;

  pathtracing->bump_map = value.texture;

  return 0;
}

static int set_bump_amplitude(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  const float amp = value.vector[0];

  pathtracing->bump_amplitude = amp;

  return 0;
}
