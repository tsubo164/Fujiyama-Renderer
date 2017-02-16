// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_shader.h"

#define COPY3(dst,src) do { \
  (dst)[0] = (src)[0]; \
  (dst)[1] = (src)[1]; \
  (dst)[2] = (src)[2]; \
  } while(0)

using namespace fj;

class SSSShader : public Shader {
public:
  SSSShader() {}
  virtual ~SSSShader() {}

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

  void UpdateProperties();
private:
  virtual void evaluate(const TraceContext &cxt,
      const SurfaceInput &in, SurfaceOutput *out) const;
  const Property *get_property_list() const;

  Color single_scattering(const TraceContext &cxt, const SurfaceInput &in,
      const LightSample &light_sample) const;
  Color diffusion_scattering(const TraceContext &cxt, const SurfaceInput &in,
      const LightSample &light_sample) const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "SSSShader";

static int set_diffuse(void *self, const PropertyValue &value);
static int set_specular(void *self, const PropertyValue &value);
static int set_ambient(void *self, const PropertyValue &value);
static int set_roughness(void *self, const PropertyValue &value);
static int set_reflect(void *self, const PropertyValue &value);
static int set_ior(void *self, const PropertyValue &value);
static int set_opacity(void *self, const PropertyValue &value);
static int set_diffuse_map(void *self, const PropertyValue &value);

static int set_enable_single_scattering(void *self, const PropertyValue &value);
static int set_enable_multiple_scattering(void *self, const PropertyValue &value);
static int set_single_scattering_samples(void *self, const PropertyValue &value);
static int set_multiple_scattering_samples(void *self, const PropertyValue &value);
static int set_scattering_coeff(void *self, const PropertyValue &value);
static int set_absorption_coeff(void *self, const PropertyValue &value);
static int set_scattering_phase(void *self, const PropertyValue &value);
static int set_single_scattering_intensity(void *self, const PropertyValue &value);
static int set_multiple_scattering_intensity(void *self, const PropertyValue &value);

static const Property MyPropertyList[] = {
  Property("diffuse",     PropVector3(.8, .8, .8), set_diffuse),
  Property("specular",    PropVector3(1, 1, 1),    set_specular),
  Property("ambient",     PropVector3(1, 1, 1),    set_ambient),
  Property("roughness",   PropScalar(.05),         set_roughness),
  Property("reflect",     PropVector3(1, 1, 1),    set_reflect),
  Property("ior",         PropScalar(1.3),         set_ior),
  Property("opacity",     PropScalar(1),           set_opacity),
  Property("diffuse_map", PropTexture(NULL),       set_diffuse_map),
  Property("enable_single_scattering",    PropScalar(0), set_enable_single_scattering),
  Property("enable_multiple_scattering",  PropScalar(1), set_enable_multiple_scattering),
  Property("single_scattering_samples",   PropScalar(1), set_single_scattering_samples),
  Property("multiple_scattering_samples", PropScalar(1), set_multiple_scattering_samples),
  Property("scattering_coefficient", PropVector3(.07, .122, .19), set_scattering_coeff),
  Property("absorption_coefficient", PropVector3(.00014,.00025,.001420), set_absorption_coeff),
  Property("scattering_phase",       PropScalar(0), set_scattering_phase),
  Property("single_scattering_intensity",   PropScalar(1), set_single_scattering_intensity),
  Property("multiple_scattering_intensity", PropScalar(.02), set_multiple_scattering_intensity),
  Property()
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
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  SSSShader *sss = new SSSShader();

  PropSetAllDefaultValues(sss, MyPropertyList);

  return sss;
}

static void MyDeleteFunction(void *self)
{
  SSSShader *sss = (SSSShader *) self;
  if (sss == NULL)
    return;
  delete sss;
}

void SSSShader::evaluate(const TraceContext &cxt,
    const SurfaceInput &in, SurfaceOutput *out) const
{
  Color diff;
  Color spec;

  LightSample *samples = NULL;
  const int nsamples = SlGetLightSampleCount(&in);

  // allocate samples
  samples = SlNewLightSamples(&in);

  for (int i = 0; i < nsamples; i++) {
    LightOutput Lout;

    Color single_scatter;
    Color diffusion_scatter;

    SlIlluminance(&cxt, &samples[i], &in.P, &in.N, PI / 2.0, &in, &Lout);
    // spec
    const float Ks = SlPhong(&in.I, &in.N, &Lout.Ln, roughness);
    spec += Ks * specular;

    if (enable_single_scattering) {
      single_scatter += single_scattering(cxt, in, samples[i]);
      single_scatter *= single_scattering_intensity;
      diff += single_scatter;
    }
    if (enable_multiple_scattering) {
      diffusion_scatter += diffusion_scattering(cxt, in, samples[i]);
      diffusion_scatter *= multiple_scattering_intensity;
      diff += diffusion_scatter;
    }
  }

  SlFreeLightSamples(samples);

  // diffuse map
  Color4 diff_map4(1, 1, 1, 1);
  if (diffuse_map != NULL) {
    diff_map4 = diffuse_map->Lookup(in.uv.u, in.uv.v);
  }
  const Color diff_map = ToColor3(diff_map4);

  // Cs
  out->Cs = diff * diffuse * diff_map + spec;

  // reflect
  if (do_reflect) {
    Color4 C_refl;
    Vector R;
    double t_hit = REAL_MAX;

    const TraceContext refl_cxt = SlReflectContext(&cxt, in.shaded_object);

    SlReflect(&in.I, &in.N, &R);
    Normalize(&R);
    // TODO fix hard-coded trace distance
    SlTrace(&refl_cxt, &in.P, &R, .001, 1000, &C_refl, &t_hit);

    const double Kr = SlFresnel(&in.I, &in.N, 1.0/ior);
    out->Cs += Kr * C_refl.r * reflect;
  }

  out->Os = 1.0;
  out->Os = opacity;
}

const Property *SSSShader::get_property_list() const
{
  return MyPropertyList;
}

Color SSSShader::single_scattering(const TraceContext &cxt, const SurfaceInput &in,
    const LightSample &light_sample) const
{
  const Vector &P = in.P;
  Vector Ln;
  Vector To;
  Vector Li;

  const float *sigma_s = scattering_coeff;
  const float *sigma_t = extinction_coeff;

  const float eta = ior;
  const float one_over_eta = 1/eta;

  const float g = scattering_phase;
  const float g_sq = g * g;

  const int nsamples = single_scattering_samples;
  int i, j;

  const TraceContext self_cxt = SlSelfHitContext(&cxt, in.shaded_object);
  LightOutput Lout;

  Color scatter;

  Ln.x = light_sample.P.x - P.x;
  Ln.y = light_sample.P.y - P.y;
  Ln.z = light_sample.P.z - P.z;
  Normalize(&Ln);

  Li.x = -Ln.x;
  Li.y = -Ln.y;
  Li.z = -Ln.z;

  SlRefract(&in.I, &in.N, one_over_eta, &To);
  Normalize(&To);

  for (i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(rng);
    const float sp_dist = -log(mutable_rng.NextFloat01());

    for (j = 0; j < 3; j++) {
      Vector P_sample;
      Vector Pi;
      Vector Ni;
      double si = REAL_MAX;
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

      double t_hit = REAL_MAX;
      int hit = 0;

      sp_o = sp_dist / sigma_t[j];

      P_sample.x = P.x + sp_o * To.x;
      P_sample.y = P.y + sp_o * To.y;
      P_sample.z = P.z + sp_o * To.z;

      hit = SlSurfaceRayIntersect(&self_cxt, &P_sample, &Ln, 0., REAL_MAX,
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

      SlIlluminance(&cxt, &light_sample, &Pi, &Ni, PI / 2., &in, &Lout);

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

  return scatter;
}

Color SSSShader::diffusion_scattering(const TraceContext &cxt, const SurfaceInput &in,
    const LightSample &light_sample) const
{
  const Vector &P = in.P;
  const Vector &N = in.N;
  Vector Ln;

  Vector N_neg;
  Vector up(0, 1, 0);
  Vector base1;
  Vector base2;
  double local_dot_up = 0;

  const float *sigma_s_prime = reduced_scattering_coeff;
  const float *sigma_t_prime = reduced_extinction_coeff;
  const float *sigma_tr = effective_extinction_coeff;

  const float Fdr = diffuse_fresnel_reflectance;
  const float A = (1 + Fdr) / (1 - Fdr);
  float alpha_prime[3] = {0, 0, 0};

  const int nsamples = multiple_scattering_samples;
  int i, j;

  Color scatter;

  alpha_prime[0] = sigma_s_prime[0] / sigma_t_prime[0];
  alpha_prime[1] = sigma_s_prime[1] / sigma_t_prime[1];
  alpha_prime[2] = sigma_s_prime[2] / sigma_t_prime[2];

  Ln.x = light_sample.P.x - P.x;
  Ln.y = light_sample.P.y - P.y;
  Ln.z = light_sample.P.z - P.z;
  Normalize(&Ln);

  N_neg.x = -N.x;
  N_neg.y = -N.y;
  N_neg.z = -N.z;

  local_dot_up = Dot(N, up);
  if (Abs(local_dot_up) > .9) {
    up.x = 1;
    up.y = 0;
    up.z = 0;
  }

  base1 = Cross(N, up);
  Normalize(&base1);
  base2 = Cross(N, base1);

  for (i = 0; i < nsamples; i++) {
    XorShift &mutable_rng = const_cast<XorShift &>(rng);
    const double dist_rand = -log(mutable_rng.NextFloat01());

    for (j = 0; j < 3; j++) {
      const TraceContext self_cxt = SlSelfHitContext(&cxt, in.shaded_object);
      LightOutput Lout;

      Vector P_sample;
      Vector2 disk(0, 0);

      Vector P_Pi;
      Vector Pi;
      Vector Ni;
      double Ln_dot_Ni = 0;
      double t_hit = REAL_MAX;
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
      P_sample.x = P.x + 1/sigma_tr[j] * (disk.x * base1.x + disk.y * base2.x);
      P_sample.y = P.y + 1/sigma_tr[j] * (disk.x * base1.y + disk.y * base2.y);
      P_sample.z = P.z + 1/sigma_tr[j] * (disk.x * base1.z + disk.y * base2.z);

      hit = SlSurfaceRayIntersect(&self_cxt, &P_sample, &N_neg, 0., REAL_MAX,
          &Pi, &Ni, &t_hit);
      if (!hit) {
        Pi.x = P.x;
        Pi.y = P.y;
        Pi.z = P.z;
        Ni.x = N.x;
        Ni.y = N.y;
        Ni.z = N.z;
      }

      P_Pi.x = Pi.x - P.x;
      P_Pi.y = Pi.y - P.y;
      P_Pi.z = Pi.z - P.z;

      SlIlluminance(&cxt, &light_sample, &Pi, &Ni, PI / 2., &in, &Lout);

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

  return scatter;
}

void SSSShader::UpdateProperties()
{
  const float eta = ior;

  extinction_coeff[0] = scattering_coeff[0] + absorption_coeff[0];
  extinction_coeff[1] = scattering_coeff[1] + absorption_coeff[1];
  extinction_coeff[2] = scattering_coeff[2] + absorption_coeff[2];

  reduced_scattering_coeff[0] = scattering_coeff[0] * (1-scattering_phase);
  reduced_scattering_coeff[1] = scattering_coeff[1] * (1-scattering_phase);
  reduced_scattering_coeff[2] = scattering_coeff[2] * (1-scattering_phase);

  reduced_extinction_coeff[0] =
      reduced_scattering_coeff[0] + absorption_coeff[0];
  reduced_extinction_coeff[1] =
      reduced_scattering_coeff[1] + absorption_coeff[1];
  reduced_extinction_coeff[2] =
      reduced_scattering_coeff[2] + absorption_coeff[2];

  effective_extinction_coeff[0] =
      sqrt(absorption_coeff[0] * reduced_extinction_coeff[0] * 3);
  effective_extinction_coeff[1] =
      sqrt(absorption_coeff[1] * reduced_extinction_coeff[1] * 3);
  effective_extinction_coeff[2] =
      sqrt(absorption_coeff[2] * reduced_extinction_coeff[2] * 3);

  diffuse_fresnel_reflectance =
      -1.440 / (eta * eta) + 0.710 / eta + 0.668 + 0.0636 * eta;
}

static int set_diffuse(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  Color diffuse;

  diffuse.r = Max(0, value.vector[0]);
  diffuse.g = Max(0, value.vector[1]);
  diffuse.b = Max(0, value.vector[2]);
  sss->diffuse = diffuse;

  return 0;
}

static int set_specular(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  Color specular;

  specular.r = Max(0, value.vector[0]);
  specular.g = Max(0, value.vector[1]);
  specular.b = Max(0, value.vector[2]);
  sss->specular = specular;

  return 0;
}

static int set_ambient(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  Color ambient;

  ambient.r = Max(0, value.vector[0]);
  ambient.g = Max(0, value.vector[1]);
  ambient.b = Max(0, value.vector[2]);
  sss->ambient = ambient;

  return 0;
}

static int set_roughness(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float roughness = value.vector[0];

  roughness = Max(0, roughness);
  sss->roughness = roughness;

  return 0;
}

static int set_reflect(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  Color reflect;

  reflect.r = Max(0, value.vector[0]);
  reflect.g = Max(0, value.vector[1]);
  reflect.b = Max(0, value.vector[2]);
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

static int set_ior(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float ior = value.vector[0];

  ior = Max(.001, ior);
  sss->ior = ior;

  return 0;
}

static int set_opacity(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float opacity = value.vector[0];

  opacity = Clamp(opacity, 0, 1);
  sss->opacity = opacity;

  return 0;
}

static int set_diffuse_map(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;

  sss->diffuse_map = value.texture;

  return 0;
}

static int set_enable_single_scattering(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  const int enable = (int) value.vector[0] == 0 ? 0 : 1;

  sss->enable_single_scattering = enable;

  return 0;
}

static int set_enable_multiple_scattering(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  const int enable = (int) value.vector[0] == 0 ? 0 : 1;

  sss->enable_multiple_scattering = enable;

  return 0;
}

static int set_single_scattering_samples(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  int nsamples = (int) value.vector[0];

  nsamples = Max(1, nsamples);

  sss->single_scattering_samples = nsamples;

  return 0;
}

static int set_multiple_scattering_samples(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  int nsamples = (int) value.vector[0];

  nsamples = Max(1, nsamples);

  sss->multiple_scattering_samples = nsamples;

  return 0;
}

static int set_scattering_coeff(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float scattering_coeff[3] = {0, 0, 0};

  scattering_coeff[0] = Max(0, value.vector[0]);
  scattering_coeff[1] = Max(0, value.vector[1]);
  scattering_coeff[2] = Max(0, value.vector[2]);
  scattering_coeff[0] *= 1000; // 1/mm
  scattering_coeff[1] *= 1000; // 1/mm
  scattering_coeff[2] *= 1000; // 1/mm
  COPY3(sss->scattering_coeff, scattering_coeff);

  sss->UpdateProperties();

  return 0;
}

static int set_absorption_coeff(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float absorption_coeff[3] = {0, 0, 0};

  absorption_coeff[0] = Max(0, value.vector[0]);
  absorption_coeff[1] = Max(0, value.vector[1]);
  absorption_coeff[2] = Max(0, value.vector[2]);
  absorption_coeff[0] *= 1000; // 1/mm
  absorption_coeff[1] *= 1000; // 1/mm
  absorption_coeff[2] *= 1000; // 1/mm
  COPY3(sss->absorption_coeff, absorption_coeff);

  sss->UpdateProperties();

  return 0;
}

static int set_scattering_phase(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float scattering_phase = value.vector[0];

  scattering_phase = Max(0, scattering_phase);

  sss->scattering_phase = scattering_phase;

  return 0;
}

static int set_single_scattering_intensity(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float intensity = value.vector[0];

  intensity = Max(0, intensity);

  sss->single_scattering_intensity = intensity;

  return 0;
}

static int set_multiple_scattering_intensity(void *self, const PropertyValue &value)
{
  SSSShader *sss = (SSSShader *) self;
  float intensity = value.vector[0];

  intensity = Max(0, intensity);

  sss->multiple_scattering_intensity = intensity;

  return 0;
}
