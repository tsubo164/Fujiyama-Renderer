// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"
#include "fj_numeric.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_color.h"

#include <cstring>
#include <cstdio>
#include <cfloat>

#define COPY3(dst,src) do { \
  (dst)[0] = (src)[0]; \
  (dst)[1] = (src)[1]; \
  (dst)[2] = (src)[2]; \
  } while(0)

using namespace fj;

class SSSShader {
public:
  SSSShader() {}
  ~SSSShader() {}

public:
  Color diffuse;
  Color specular;
  Color ambient;
  float roughness;

  Color reflect;
  float ior;

  float opacity;

  int do_reflect;

  Texture *diffuse_map;

  int enable_single_scattering;
  int enable_multiple_scattering;

  XorShift rng;

  float scattering_coeff[3];
  float absorption_coeff[3];
  float extinction_coeff[3];
  float scattering_phase;

  float single_scattering_intensity;
  float multiple_scattering_intensity;

  int single_scattering_samples;
  int multiple_scattering_samples;

  float reduced_scattering_coeff[3];
  float reduced_extinction_coeff[3];
  float effective_extinction_coeff[3];

  float diffuse_fresnel_reflectance;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out);

static const char MyPluginName[] = "SSSShader";
static const ShaderFunctionTable MyFunctionTable = {
  MyEvaluate
};

static void update_sss_properties(SSSShader *sss);
static void single_scattering(const SSSShader *sss,
    const TraceContext *cxt, const SurfaceInput *in,
    const LightSample *light_sample, Color *C_scatter);
static void diffusion_scattering(const SSSShader *sss,
    const TraceContext *cxt, const SurfaceInput *in,
    const LightSample *light_sample, Color *C_scatter);

static int set_diffuse(void *self, const PropertyValue *value);
static int set_specular(void *self, const PropertyValue *value);
static int set_ambient(void *self, const PropertyValue *value);
static int set_roughness(void *self, const PropertyValue *value);
static int set_reflect(void *self, const PropertyValue *value);
static int set_ior(void *self, const PropertyValue *value);
static int set_opacity(void *self, const PropertyValue *value);
static int set_diffuse_map(void *self, const PropertyValue *value);

static int set_enable_single_scattering(void *self, const PropertyValue *value);
static int set_enable_multiple_scattering(void *self, const PropertyValue *value);
static int set_single_scattering_samples(void *self, const PropertyValue *value);
static int set_multiple_scattering_samples(void *self, const PropertyValue *value);
static int set_scattering_coeff(void *self, const PropertyValue *value);
static int set_absorption_coeff(void *self, const PropertyValue *value);
static int set_scattering_phase(void *self, const PropertyValue *value);
static int set_single_scattering_intensity(void *self, const PropertyValue *value);
static int set_multiple_scattering_intensity(void *self, const PropertyValue *value);

static const Property MyProperties[] = {
  {PROP_VECTOR3, "diffuse",     {.8, .8, .8, 0}, set_diffuse},
  {PROP_VECTOR3, "specular",    {1, 1, 1, 0}, set_specular},
  {PROP_VECTOR3, "ambient",     {1, 1, 1, 0}, set_ambient},
  {PROP_SCALAR,  "roughness",   {.05, 0, 0, 0}, set_roughness},
  {PROP_VECTOR3, "reflect",     {1, 1, 1, 0}, set_reflect},
  {PROP_SCALAR,  "ior",         {1.3, 0, 0, 0}, set_ior},
  {PROP_SCALAR,  "opacity",     {1, 0, 0, 0}, set_opacity},
  {PROP_TEXTURE, "diffuse_map", {0, 0, 0, 0}, set_diffuse_map},
  {PROP_SCALAR,  "enable_single_scattering",    {0, 0, 0, 0}, set_enable_single_scattering},
  {PROP_SCALAR,  "enable_multiple_scattering",  {1, 0, 0, 0}, set_enable_multiple_scattering},
  {PROP_SCALAR,  "single_scattering_samples",   {1, 0, 0, 0}, set_single_scattering_samples},
  {PROP_SCALAR,  "multiple_scattering_samples", {1, 0, 0, 0}, set_multiple_scattering_samples},
  {PROP_VECTOR3, "scattering_coefficient", {.07, .122, .19, 0},          set_scattering_coeff},
  {PROP_VECTOR3, "absorption_coefficient", {.00014, .00025, .001420, 0}, set_absorption_coeff},
  {PROP_SCALAR,  "scattering_phase", {0, 0, 0, 0}, set_scattering_phase},
  {PROP_SCALAR,  "single_scattering_intensity", {1, 0, 0, 0}, set_single_scattering_intensity},
  {PROP_SCALAR,  "multiple_scattering_intensity", {.02, 0, 0, 0}, set_multiple_scattering_intensity},
  {PROP_NONE, NULL, {0, 0, 0, 0}, NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A sss shader."},
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
      MyNew,
      MyFree,
      &MyFunctionTable,
      MyProperties,
      MyMetainfo);
}
} // extern "C"

static void *MyNew(void)
{
  SSSShader *sss = new SSSShader();

  PropSetAllDefaultValues(sss, MyProperties);

  return sss;
}

static void MyFree(void *self)
{
  SSSShader *sss = (SSSShader *) self;
  if (sss == NULL)
    return;
  delete sss;
}

static void MyEvaluate(const void *self, const TraceContext *cxt,
    const SurfaceInput *in, SurfaceOutput *out)
{
  const SSSShader *sss = (SSSShader *) self;
  Color diff;
  Color spec;
  Color4 diff_map(1, 1, 1, 1);
  int i;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(in);

  // allocate samples
  samples = SlNewLightSamples(in);

  for (i = 0; i < nsamples; i++) {
    LightOutput Lout;
    float Ks = 0;

    Color single_scatter;
    Color diffusion_scatter;

    SlIlluminance(cxt, &samples[i], &in->P, &in->N, PI / 2., in, &Lout);
    // spec
    Ks = SlPhong(&in->I, &in->N, &Lout.Ln, sss->roughness);
    spec.r += Ks * sss->specular.r;
    spec.g += Ks * sss->specular.g;
    spec.b += Ks * sss->specular.b;

    if (sss->enable_single_scattering) {
      single_scattering(sss, cxt, in, &samples[i], &single_scatter);
      single_scatter.r *= sss->single_scattering_intensity;
      single_scatter.g *= sss->single_scattering_intensity;
      single_scatter.b *= sss->single_scattering_intensity;
      diff.r += single_scatter.r;
      diff.g += single_scatter.g;
      diff.b += single_scatter.b;
    }
    if (sss->enable_multiple_scattering) {
      diffusion_scattering(sss, cxt, in, &samples[i], &diffusion_scatter);
      diffusion_scatter.r *= sss->multiple_scattering_intensity;
      diffusion_scatter.g *= sss->multiple_scattering_intensity;
      diffusion_scatter.b *= sss->multiple_scattering_intensity;
      diff.r += diffusion_scatter.r;
      diff.g += diffusion_scatter.g;
      diff.b += diffusion_scatter.b;
    }
  }

  SlFreeLightSamples(samples);

  // diffuse map
  if (sss->diffuse_map != NULL) {
    diff_map = sss->diffuse_map->Lookup(in->uv.u, in->uv.v);
  }

  // Cs
  out->Cs.r = diff.r * sss->diffuse.r * diff_map.r + spec.r;
  out->Cs.g = diff.g * sss->diffuse.g * diff_map.g + spec.g;
  out->Cs.b = diff.b * sss->diffuse.b * diff_map.b + spec.b;

  // reflect
  if (sss->do_reflect) {
    Color4 C_refl;
    Vector R;
    double t_hit = FLT_MAX;
    double Kr = 0;

    const TraceContext refl_cxt = SlReflectContext(cxt, in->shaded_object);

    SlReflect(&in->I, &in->N, &R);
    Normalize(&R);
    // TODO fix hard-coded trace distance
    SlTrace(&refl_cxt, &in->P, &R, .001, 1000, &C_refl, &t_hit);

    Kr = SlFresnel(&in->I, &in->N, 1/sss->ior);
    out->Cs.r += Kr * C_refl.r * sss->reflect.r;
    out->Cs.g += Kr * C_refl.g * sss->reflect.g;
    out->Cs.b += Kr * C_refl.b * sss->reflect.b;
  }

  out->Os = 1;
  out->Os = sss->opacity;
}

static void update_sss_properties(SSSShader *sss)
{
  const float eta = sss->ior;

  sss->extinction_coeff[0] = sss->scattering_coeff[0] + sss->absorption_coeff[0];
  sss->extinction_coeff[1] = sss->scattering_coeff[1] + sss->absorption_coeff[1];
  sss->extinction_coeff[2] = sss->scattering_coeff[2] + sss->absorption_coeff[2];

  sss->reduced_scattering_coeff[0] = sss->scattering_coeff[0] * (1-sss->scattering_phase);
  sss->reduced_scattering_coeff[1] = sss->scattering_coeff[1] * (1-sss->scattering_phase);
  sss->reduced_scattering_coeff[2] = sss->scattering_coeff[2] * (1-sss->scattering_phase);

  sss->reduced_extinction_coeff[0] =
      sss->reduced_scattering_coeff[0] + sss->absorption_coeff[0];
  sss->reduced_extinction_coeff[1] =
      sss->reduced_scattering_coeff[1] + sss->absorption_coeff[1];
  sss->reduced_extinction_coeff[2] =
      sss->reduced_scattering_coeff[2] + sss->absorption_coeff[2];

  sss->effective_extinction_coeff[0] =
      sqrt(sss->absorption_coeff[0] * sss->reduced_extinction_coeff[0] * 3);
  sss->effective_extinction_coeff[1] =
      sqrt(sss->absorption_coeff[1] * sss->reduced_extinction_coeff[1] * 3);
  sss->effective_extinction_coeff[2] =
      sqrt(sss->absorption_coeff[2] * sss->reduced_extinction_coeff[2] * 3);

  sss->diffuse_fresnel_reflectance =
      -1.440 / (eta * eta) + 0.710 / eta + 0.668 + 0.0636 * eta;
}

static void single_scattering(const SSSShader *sss,
    const TraceContext *cxt, const SurfaceInput *in,
    const LightSample *light_sample, Color *C_scatter)
{
  const Vector *P = &in->P;
  Vector Ln;
  Vector To;
  Vector Li;

  const float *sigma_s = sss->scattering_coeff;
  const float *sigma_t = sss->extinction_coeff;

  const float eta = sss->ior;
  const float one_over_eta = 1/eta;

  const float g = sss->scattering_phase;
  const float g_sq = g * g;

  const int nsamples = sss->single_scattering_samples;
  int i, j;

  const TraceContext self_cxt = SlSelfHitContext(cxt, in->shaded_object);
  LightOutput Lout;

  Color scatter;

  Ln.x = light_sample->P.x - P->x;
  Ln.y = light_sample->P.y - P->y;
  Ln.z = light_sample->P.z - P->z;
  Normalize(&Ln);

  Li.x = -Ln.x;
  Li.y = -Ln.y;
  Li.z = -Ln.z;

  SlRefract(&in->I, &in->N, one_over_eta, &To);
  Normalize(&To);

  for (i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(sss->rng);
    const float sp_dist = -log(mutable_rng.NextFloat01());

    for (j = 0; j < 3; j++) {
      Vector P_sample;
      Vector Pi;
      Vector Ni;
      double si = FLT_MAX;
      double Ln_dot_Ni = 0;

      Vector Ti;
      float Kri = 0;
      float Kti = 0;

      double G = 0;
      float sigma_tc = 0;
      float sp_i = 0;
      float sp_o = 0;
      float phase = 0;

      double Ni_dot_To = 0;
      double Ni_dot_Ti = 0;
      double Ti_dot_To = 0;

      double t_hit = FLT_MAX;
      int hit = 0;

      sp_o = sp_dist / sigma_t[j];

      P_sample.x = P->x + sp_o * To.x;
      P_sample.y = P->y + sp_o * To.y;
      P_sample.z = P->z + sp_o * To.z;

      hit = SlSurfaceRayIntersect(&self_cxt, &P_sample, &Ln, 0., FLT_MAX,
          &Pi, &Ni, &t_hit);
      if (!hit) {
        continue;
      }

      si = t_hit;
      Ln_dot_Ni = Dot(Ln, Ni);

      sp_i = si * Ln_dot_Ni /
          sqrt(1 - one_over_eta * one_over_eta * (1 - Ln_dot_Ni * Ln_dot_Ni));

      SlRefract(&Li, &Ni, one_over_eta, &Ti);
      Normalize(&Ti);

      Kri = SlFresnel(&Li, &Ni, one_over_eta);
      Kti = 1 - Kri;

      Ti_dot_To = Dot(Ti, To);
      phase = (1 - g_sq) / pow(1 + 2 * g * Ti_dot_To + g_sq, 1.5);

      Ni_dot_To = Dot(Ni, To);
      Ni_dot_Ti = Dot(Ni, Ti);
      G = Abs(Ni_dot_To) / Abs(Ni_dot_Ti);

      SlIlluminance(cxt, light_sample, &Pi, &Ni, PI / 2., in, &Lout);

      sigma_tc = sigma_t[j] + G * sigma_t[j];

      switch (j) {
      case 0:
        scatter.r += Lout.Cl.r * exp(-sp_i * sigma_t[0]) / sigma_tc * phase * Kti;
        break;
      case 1:
        scatter.g += Lout.Cl.g * exp(-sp_i * sigma_t[1]) / sigma_tc * phase * Kti;
        break;
      case 2:
        scatter.b += Lout.Cl.b * exp(-sp_i * sigma_t[2]) / sigma_tc * phase * Kti;
        break;
      default:
        break;
      }
    }
  }
  scatter.r *= PI * sigma_s[0] / nsamples;
  scatter.g *= PI * sigma_s[1] / nsamples;
  scatter.b *= PI * sigma_s[2] / nsamples;

  C_scatter->r += scatter.r;
  C_scatter->g += scatter.g;
  C_scatter->b += scatter.b;
}

static void diffusion_scattering(const SSSShader *sss,
    const TraceContext *cxt, const SurfaceInput *in,
    const LightSample *light_sample, Color *C_scatter)
{
  const Vector *P = &in->P;
  const Vector *N = &in->N;
  Vector Ln;

  Vector N_neg;
  Vector up(0, 1, 0);
  Vector base1;
  Vector base2;
  double local_dot_up = 0;

  const float *sigma_s_prime = sss->reduced_scattering_coeff;
  const float *sigma_t_prime = sss->reduced_extinction_coeff;
  const float *sigma_tr = sss->effective_extinction_coeff;

  const float Fdr = sss->diffuse_fresnel_reflectance;
  const float A = (1 + Fdr) / (1 - Fdr);
  float alpha_prime[3] = {0, 0, 0};

  const int nsamples = sss->multiple_scattering_samples;
  int i, j;

  Color scatter;

  alpha_prime[0] = sigma_s_prime[0] / sigma_t_prime[0];
  alpha_prime[1] = sigma_s_prime[1] / sigma_t_prime[1];
  alpha_prime[2] = sigma_s_prime[2] / sigma_t_prime[2];

  Ln.x = light_sample->P.x - P->x;
  Ln.y = light_sample->P.y - P->y;
  Ln.z = light_sample->P.z - P->z;
  Normalize(&Ln);

  N_neg.x = -N->x;
  N_neg.y = -N->y;
  N_neg.z = -N->z;

  local_dot_up = Dot(*N, up);
  if (Abs(local_dot_up) > .9) {
    up.x = 1;
    up.y = 0;
    up.z = 0;
  }

  base1 = Cross(*N, up);
  Normalize(&base1);
  base2 = Cross(*N, base1);

  for (i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(sss->rng);
    const double dist_rand = -log(mutable_rng.NextFloat01());

    for (j = 0; j < 3; j++) {
      const TraceContext self_cxt = SlSelfHitContext(cxt, in->shaded_object);
      LightOutput Lout;

      Vector P_sample;
      Vector2 disk(0, 0);

      Vector P_Pi;
      Vector Pi;
      Vector Ni;
      double Ln_dot_Ni = 0;
      double t_hit = FLT_MAX;
      int hit = 0;

      double r = 0;
      double zr = 0;
      double zv = 0;
      double dr = 0;
      double dv = 0;
      double sigma_tr_dr = 0;
      double sigma_tr_dv = 0;
      double Rd = 0;
      double scat = 0;

      const double dist = dist_rand / sigma_tr[j];

      disk = mutable_rng.HollowDiskRand();
      disk.x *= dist;
      disk.y *= dist;
      P_sample.x = P->x + 1/sigma_tr[j] * (disk.x * base1.x + disk.y * base2.x);
      P_sample.y = P->y + 1/sigma_tr[j] * (disk.x * base1.y + disk.y * base2.y);
      P_sample.z = P->z + 1/sigma_tr[j] * (disk.x * base1.z + disk.y * base2.z);

      hit = SlSurfaceRayIntersect(&self_cxt, &P_sample, &N_neg, 0., FLT_MAX,
          &Pi, &Ni, &t_hit);
      if (!hit) {
        Pi.x = P->x;
        Pi.y = P->y;
        Pi.z = P->z;
        Ni.x = N->x;
        Ni.y = N->y;
        Ni.z = N->z;
      }

      P_Pi.x = Pi.x - P->x;
      P_Pi.y = Pi.y - P->y;
      P_Pi.z = Pi.z - P->z;

      SlIlluminance(cxt, light_sample, &Pi, &Ni, PI / 2., in, &Lout);

      r = Length(P_Pi);
      zr = sqrt(3 * (1 - alpha_prime[j])) / sigma_tr[j];
      zv = A * zr;
      dr = sqrt(r * r + zr * zr); // distance to positive light
      dv = sqrt(r * r + zv * zv); // distance to negative light
      sigma_tr_dr = sigma_tr[j] * dr;
      sigma_tr_dv = sigma_tr[j] * dv;
      Rd = (sigma_tr_dr + 1) * exp(-sigma_tr_dr) * zr / pow(dr, 3) +
         (sigma_tr_dv + 1) * exp(-sigma_tr_dv) * zr / pow(dv, 3);

      Ln_dot_Ni = Dot(Ln, Ni);
      Ln_dot_Ni = Max(0, Ln_dot_Ni);

      scat = sigma_tr[j] * sigma_tr[j] * exp(-sigma_tr[j] * r);
      if (scat != 0) {
        switch (j) {
        case 0:
          scat = Lout.Cl.r * Ln_dot_Ni * Rd / scat;
          scatter.r += scat;
          break;
        case 1:
          scat = Lout.Cl.g * Ln_dot_Ni * Rd / scat;
          scatter.g += scat;
          break;
        case 2:
          scat = Lout.Cl.b * Ln_dot_Ni * Rd / scat;
          scatter.b += scat;
          break;
        default:
          break;
        }
      }
    }
  }
  scatter.r *= (1 - Fdr) * alpha_prime[0] / nsamples;
  scatter.g *= (1 - Fdr) * alpha_prime[1] / nsamples;
  scatter.b *= (1 - Fdr) * alpha_prime[2] / nsamples;


  C_scatter->r += scatter.r;
  C_scatter->g += scatter.g;
  C_scatter->b += scatter.b;
}

static int set_diffuse(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value->vector[0]);
  diffuse.g = Max(0, value->vector[1]);
  diffuse.b = Max(0, value->vector[2]);
  sss->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  Color specular;

  specular.r = Max(0, value->vector[0]);
  specular.g = Max(0, value->vector[1]);
  specular.b = Max(0, value->vector[2]);
  sss->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  Color ambient;

  ambient.r = Max(0, value->vector[0]);
  ambient.g = Max(0, value->vector[1]);
  ambient.b = Max(0, value->vector[2]);
  sss->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float roughness = value->vector[0];

  roughness = Max(0, roughness);
  sss->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  Color reflect;

  reflect.r = Max(0, value->vector[0]);
  reflect.g = Max(0, value->vector[1]);
  reflect.b = Max(0, value->vector[2]);
  sss->reflect = reflect;

  if (sss->reflect.r > 0 ||
    sss->reflect.g > 0 ||
    sss->reflect.b > 0 ) {
    sss->do_reflect = 1;
  }
  else {
    sss->do_reflect = 0;
  }

  return 0;
}

static int set_ior(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float ior = value->vector[0];

  ior = Max(.001, ior);
  sss->ior = ior;

  return 0;
}

static int set_opacity(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float opacity = value->vector[0];

  opacity = Clamp(opacity, 0, 1);
  sss->opacity = opacity;

  return 0;
}

static int set_diffuse_map(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;

  sss->diffuse_map = value->texture;

  return 0;
}

static int set_enable_single_scattering(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  const int enable = (int) value->vector[0] == 0 ? 0 : 1;

  sss->enable_single_scattering = enable;

  return 0;
}

static int set_enable_multiple_scattering(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  const int enable = (int) value->vector[0] == 0 ? 0 : 1;

  sss->enable_multiple_scattering = enable;

  return 0;
}

static int set_single_scattering_samples(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  int nsamples = (int) value->vector[0];

  nsamples = Max(1, nsamples);

  sss->single_scattering_samples = nsamples;

  return 0;
}

static int set_multiple_scattering_samples(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  int nsamples = (int) value->vector[0];

  nsamples = Max(1, nsamples);

  sss->multiple_scattering_samples = nsamples;

  return 0;
}

static int set_scattering_coeff(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float scattering_coeff[3] = {0, 0, 0};

  scattering_coeff[0] = Max(0, value->vector[0]);
  scattering_coeff[1] = Max(0, value->vector[1]);
  scattering_coeff[2] = Max(0, value->vector[2]);
  scattering_coeff[0] *= 1000; // 1/mm
  scattering_coeff[1] *= 1000; // 1/mm
  scattering_coeff[2] *= 1000; // 1/mm
  COPY3(sss->scattering_coeff, scattering_coeff);

  update_sss_properties(sss);

  return 0;
}

static int set_absorption_coeff(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float absorption_coeff[3] = {0, 0, 0};

  absorption_coeff[0] = Max(0, value->vector[0]);
  absorption_coeff[1] = Max(0, value->vector[1]);
  absorption_coeff[2] = Max(0, value->vector[2]);
  absorption_coeff[0] *= 1000; // 1/mm
  absorption_coeff[1] *= 1000; // 1/mm
  absorption_coeff[2] *= 1000; // 1/mm
  COPY3(sss->absorption_coeff, absorption_coeff);

  update_sss_properties(sss);

  return 0;
}

static int set_scattering_phase(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float scattering_phase = value->vector[0];

  scattering_phase = Max(0, scattering_phase);

  sss->scattering_phase = scattering_phase;

  return 0;
}

static int set_single_scattering_intensity(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float intensity = value->vector[0];

  intensity = Max(0, intensity);

  sss->single_scattering_intensity = intensity;

  return 0;
}

static int set_multiple_scattering_intensity(void *self, const PropertyValue *value)
{
  SSSShader *sss = (SSSShader *) self;
  float intensity = value->vector[0];

  intensity = Max(0, intensity);

  sss->multiple_scattering_intensity = intensity;

  return 0;
}
