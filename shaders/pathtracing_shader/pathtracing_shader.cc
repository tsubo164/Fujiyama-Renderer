// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_multi_thread.h"

using namespace fj;

class PathtracingShader : public Shader {
public:
  PathtracingShader() {}
  virtual ~PathtracingShader() {}

public:
  Color emission;
  Color diffuse;
  Color specular;
  Color ambient;
  Color transmit;
  float roughness;

  Color reflect;
  Color refract;
  float ior;

  float opacity;

  Texture *diffuse_map;
  Texture *bump_map;
  float bump_amplitude;

  bool do_color_filter;
private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  Color integrate_diffuse(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  Color integrate_reflect(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  Color integrate_refract(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  // TODO TEST
  Color direct_lighting(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;

  mutable XorShift rng[64];
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "PathtracingShader";

static int set_emission(void *self, const PropertyValue &value);
static int set_diffuse(void *self, const PropertyValue &value);
static int set_specular(void *self, const PropertyValue &value);
static int set_ambient(void *self, const PropertyValue &value);
static int set_transmit(void *self, const PropertyValue &value);
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
  Property("specular",       PropVector3(0, 0, 0),    set_specular),
  Property("ambient",        PropVector3(1, 1, 1),    set_ambient),
  Property("transmit",       PropVector3(1, 1, 1),    set_transmit),
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
  // TODO come up with the best way to pass attributes to integrators
  SurfaceInput in_modified = in;

  // diffuse map
  if (diffuse_map != NULL) {
    const Color4 C_diff_map = diffuse_map->Lookup(in.uv.u, in.uv.v);
    in_modified.Cd *= ToColor(C_diff_map);
  }
  // bump map
  if (bump_map != NULL) {
    Vector N_bump;
    SlBumpMapping(bump_map,
        &in.dPdu, &in.dPdv,
        &in.uv, bump_amplitude,
        &in.N, &N_bump);
    in_modified.N = N_bump;
  }

  Color Lo, Le, /*L_direct,*/ L_diffuse, L_reflect, L_refract;

  Le = emission;

  // TODO TEST
  //L_direct = direct_lighting(cxt, in_modified, out);

  if (Luminance(diffuse) > 0.) {
    L_diffuse = integrate_diffuse(cxt, in_modified, out);
  }
  if (Luminance(reflect) > 0.) {
    L_reflect = integrate_reflect(cxt, in_modified, out);
  }
  if (Luminance(refract) > 0.) {
    L_refract = integrate_refract(cxt, in_modified, out);
  }

  Lo = Le + /*L_direct +*/ L_diffuse + L_reflect + L_refract;

  out->Cs = Lo;
  out->Os = 1;
}

// TODO TEST NEW FUNCTIONS
#if 0
Vector ReflectionDir(const Vector &I, const Vector &N);
Vector RefractionDir(const Vector &I, const Vector &N, Real ior,
    bool *is_total_internal_reflection);
#endif

Color PathtracingShader::integrate_diffuse(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  Vector u, v, w;
  w = in.N;
  u = Abs(w.x) > .001 ? Vector(0, 1, 0) : Vector(1, 0, 0);
  u = Cross(u, w);
  u = Normalize(u);
  v = Cross(w, u);

  const int id = MtGetThreadID();
  const Real r1 = 2. * PI * rng[id].NextFloat01();
  const Real r2 = rng[id].NextFloat01();
  const Real r2sqrt = Sqrt(r2);

  const Vector D = Normalize(
    u * cos(r1) * r2sqrt +
    v * sin(r1) * r2sqrt +
    w * sqrt(1. - r2));

  const Real Kd = Dot(in.N, D);

  Color4 C_diff;
  Real t_hit = REAL_MAX;
  const TraceContext refl_cxt = SlDiffuseContext(&cxt, in.shaded_object);

  SlTrace(&refl_cxt, &in.P, &D, .001, 1000, &C_diff, &t_hit);

  out->Cs = in.Cd * Kd * diffuse * ToColor(C_diff);
  out->Os = 1.0;

  return out->Cs;
}

Color PathtracingShader::integrate_reflect(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  Vector R;
  SlReflect(&in.I, &in.N, &R);
  R = Normalize(R);

  const Real Kr = SlFresnel(&in.I, &in.N, 1./ior);

  Color4 C_refl;
  Real t_hit = REAL_MAX;
  const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);

  SlTrace(&refl_cxt, &in.P, &R, .001, 1000, &C_refl, &t_hit);

  out->Cs = Kr * reflect * ToColor(C_refl);
  out->Os = 1.0;

  return out->Cs;
}

Color PathtracingShader::integrate_refract(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  Vector T;
  SlRefract(&in.I, &in.N, 1./ior, &T);
  T = Normalize(T);

  const Real Kr = SlFresnel(&in.I, &in.N, 1/ior);
  const Real Kt = 1 - Kr;

  Color4 C_refr;
  Real t_hit = REAL_MAX;
  const TraceContext refr_cxt = SlRefractContext(&cxt, in.shaded_object);

  SlTrace(&refr_cxt, &in.P, &T, .0001, 1000, &C_refr, &t_hit);

  if (do_color_filter && Dot(in.I, in.N) < 0) {
    C_refr.r *= pow(transmit.r, t_hit);
    C_refr.g *= pow(transmit.g, t_hit);
    C_refr.b *= pow(transmit.b, t_hit);
  }

  out->Cs = Kt * refract * ToColor(C_refr);
  out->Os = 1;

  return out->Cs;
}

Color PathtracingShader::direct_lighting(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const
{
  // allocate samples
  LightSample *samples = SlNewLightSamples(&in);
  const Vector D = Normalize(samples[0].P - in.P);
  // free samples
  SlFreeLightSamples(samples);
  //----------------------------------------------------------

  const Real Kd = Dot(in.N, D);

  Color4 C_diff;
  Real t_hit = REAL_MAX;
  const TraceContext refl_cxt = SlDiffuseContext(&cxt, in.shaded_object);

  SlTrace(&refl_cxt, &in.P, &D, .001, 1000, &C_diff, &t_hit);

  out->Cs = in.Cd * Kd * diffuse * ToColor(C_diff);
  out->Os = 1.0;

  return out->Cs;
#if 0
  Vector Nf;
  SlFaceforward(&in.I, &in.N, &Nf);

  // allocate samples
  LightSample *samples = SlNewLightSamples(&in);
  const int nsamples = SlGetLightSampleCount(&in);

  Color C_direct;
  for (int i = 0; i < nsamples; i++) {
    LightOutput Lout;
    SlIlluminance(&cxt, &samples[i], &in.P, &Nf, PI / 2., &in, &Lout);

    const float Ks = SlPhong(&in.I, &Nf, &Lout.Ln, .05) * 0;
    const float Kd = Max(0, Dot(Nf, Lout.Ln));

    C_direct += in.Cd * Kd * diffuse * Lout.Cl + Ks * specular * Lout.Cl;
  }
  // free samples
  SlFreeLightSamples(samples);

  return C_direct;
#endif
}

static int set_emission(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color emission;

  emission.r = Max(0, value.vector[0]);
  emission.g = Max(0, value.vector[1]);
  emission.b = Max(0, value.vector[2]);
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

static int set_transmit(void *self, const PropertyValue &value)
{
  PathtracingShader *pathtracing = (PathtracingShader *) self;
  Color transmit;

  transmit.r = Max(.001, value.vector[0]);
  transmit.g = Max(.001, value.vector[1]);
  transmit.b = Max(.001, value.vector[2]);
  pathtracing->transmit = transmit;

  if (pathtracing->transmit.r == 1 &&
    pathtracing->transmit.g == 1 &&
    pathtracing->transmit.b == 1) {
    pathtracing->do_color_filter = 0;
  }
  else {
    pathtracing->do_color_filter = 1;
  }

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

// TODO TEST NEW FUNCTIONS
#if 0
Vector ReflectionDir(const Vector &I, const Vector &N)
{
  const Real cos = Dot(-I, N);
  return Normalize(I + 2 * cos * N);
}

Vector RefractionDir(const Vector &I, const Vector &N, Real ior,
    bool *is_total_internal_reflection)
{
  Vector Nf = N;
  Real eta = ior;
  Real cos1 = 0;

  cos1 = Dot(-I, N);
  if (cos1 < 0) {
    cos1 *= -1;
    eta = 1./eta;
    Nf *= -1;
  }

  // check total internal reflection
  const Real radicand = 1 - eta*eta * (1 - cos1*cos1);
  if (radicand < 0.) {
    *is_total_internal_reflection = true;
    return ReflectionDir(I, -N);
    //return Vector();
  } else {
    *is_total_internal_reflection = false;
  }

  const Real ncoeff = eta * cos1 - Sqrt(radicand);
  return Normalize(eta * I + ncoeff * Nf);
}
#endif
