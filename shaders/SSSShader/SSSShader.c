/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Numeric.h"
#include "Memory.h"
#include "Random.h"
#include "Vector.h"
#include "Color.h"

#include <string.h>
#include <stdio.h>
#include <float.h>

#define SET3(dst,x,y,z) do { \
	(dst)[0] = (x); \
	(dst)[1] = (y); \
	(dst)[2] = (z); \
	} while(0)

#define COPY3(dst,src) do { \
	(dst)[0] = (src)[0]; \
	(dst)[1] = (src)[1]; \
	(dst)[2] = (src)[2]; \
	} while(0)

struct SSSShader {
	struct Color diffuse;
	struct Color specular;
	struct Color ambient;
	float roughness;

	struct Color reflect;
	float ior;

	float opacity;

	int do_reflect;

	struct Texture *diffuse_map;

	/* TODO TEST sss */
	int enable_single_scattering;
	int enable_multiple_scattering;

	struct XorShift xr;

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
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "SSSShader";
static const struct ShaderFunctionTable MyFunctionTable = {
	MyEvaluate
};

static void update_sss_properties(struct SSSShader *sss);
static void single_scattering(const struct SSSShader *sss,
		const struct TraceContext *cxt, const struct SurfaceInput *in,
		const struct LightSample *light_sample, struct Color *C_scatter);
static void diffusion_scattering(const struct SSSShader *sss,
		const struct TraceContext *cxt, const struct SurfaceInput *in,
		const struct LightSample *light_sample, struct Color *C_scatter);

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_reflect(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);
static int set_opacity(void *self, const struct PropertyValue *value);
static int set_diffuse_map(void *self, const struct PropertyValue *value);

static int set_enable_single_scattering(void *self, const struct PropertyValue *value);
static int set_enable_multiple_scattering(void *self, const struct PropertyValue *value);
static int set_single_scattering_samples(void *self, const struct PropertyValue *value);
static int set_multiple_scattering_samples(void *self, const struct PropertyValue *value);
static int set_scattering_coefficient(void *self, const struct PropertyValue *value);
static int set_absorption_coefficient(void *self, const struct PropertyValue *value);
static int set_single_scattering_intensity(void *self, const struct PropertyValue *value);
static int set_multiple_scattering_intensity(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "diffuse",     set_diffuse},
	{PROP_VECTOR3, "specular",    set_specular},
	{PROP_VECTOR3, "ambient",     set_ambient},
	{PROP_SCALAR,  "roughness",   set_roughness},
	{PROP_VECTOR3, "reflect",     set_reflect},
	{PROP_SCALAR,  "ior",         set_ior},
	{PROP_SCALAR,  "opacity",     set_opacity},
	{PROP_TEXTURE, "diffuse_map", set_diffuse_map},
	{PROP_SCALAR,  "enable_single_scattering",      set_enable_single_scattering},
	{PROP_SCALAR,  "enable_multiple_scattering",    set_enable_multiple_scattering},
	{PROP_SCALAR,  "single_scattering_samples",     set_single_scattering_samples},
	{PROP_SCALAR,  "multiple_scattering_samples",   set_multiple_scattering_samples},
	{PROP_VECTOR3, "scattering_coefficient",        set_scattering_coefficient},
	{PROP_VECTOR3, "absorption_coefficient",        set_absorption_coefficient},
	{PROP_SCALAR,  "single_scattering_intensity",   set_single_scattering_intensity},
	{PROP_SCALAR,  "multiple_scattering_intensity", set_multiple_scattering_intensity},
	{PROP_NONE, NULL, NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A sss shader."},
	{"plugin_type", "Shader"},
	{NULL, NULL}
};

int Initialize(struct PluginInfo *info)
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

static void *MyNew(void)
{
	struct SSSShader *sss = MEM_ALLOC(struct SSSShader);

	if (sss == NULL)
		return NULL;

	ColSet(&sss->diffuse, .7, .8, .8);
	ColSet(&sss->specular, 1, 1, 1);
	ColSet(&sss->ambient, 1, 1, 1);
	sss->roughness = .05;

	ColSet(&sss->reflect, 1, 1, 1);
	sss->ior = 1.3;

	sss->opacity = 1;

	sss->do_reflect = 1;

	sss->diffuse_map = NULL;

	sss->enable_single_scattering = 0;
	sss->enable_multiple_scattering = 1;

	XorInit(&sss->xr);

	/* Skimmilk */
	SET3(sss->scattering_coeff, .07 * 1000, .122 * 1000, .19 * 1000); /* 1/mm */
	SET3(sss->absorption_coeff, .00014 * 1000, .00025 * 1000, .00142 * 1000); /* 1/mm */

	sss->scattering_phase = 0;

	sss->single_scattering_intensity = 1;
	sss->multiple_scattering_intensity = .02;

	sss->single_scattering_samples = 1;
	sss->multiple_scattering_samples = 1;
	update_sss_properties(sss);

	return sss;
}

static void MyFree(void *self)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	if (sss == NULL)
		return;
	MEM_FREE(sss);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	const struct SSSShader *sss = (struct SSSShader *) self;
	struct Color diff = {0, 0, 0};
	struct Color spec = {0, 0, 0};
	struct Color4 diff_map = {1, 1, 1, 1};
	int i;

	struct LightSample *samples = NULL;
	const int nsamples = SlGetLightSampleCount(in);

	/* allocate samples */
	samples = SlNewLightSamples(in);

	for (i = 0; i < nsamples; i++) {
		struct LightOutput Lout;
		float Ks = 0;

		struct Color single_scatter = {0, 0, 0};
		struct Color diffusion_scatter = {0, 0, 0};

		SlIlluminance(cxt, &samples[i], &in->P, &in->N, N_PI_2, in, &Lout);
		/* spec */
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

	/* MEM_FREE samples */
	SlFreeLightSamples(samples);

	/* diffuse map */
	if (sss->diffuse_map != NULL) {
		TexLookup(sss->diffuse_map, in->uv.u, in->uv.v, &diff_map);
	}

	/* Cs */
	out->Cs.r = diff.r * sss->diffuse.r * diff_map.r + spec.r;
	out->Cs.g = diff.g * sss->diffuse.g * diff_map.g + spec.g;
	out->Cs.b = diff.b * sss->diffuse.b * diff_map.b + spec.b;

	/* reflect */
	if (sss->do_reflect) {
		struct Color4 C_refl = {0, 0, 0};
		struct Vector R = {0, 0, 0};
		double t_hit = FLT_MAX;
		double Kr = 0;

		const struct TraceContext refl_cxt = SlReflectContext(cxt, in->shaded_object);

		SlReflect(&in->I, &in->N, &R);
		VEC3_NORMALIZE(&R);
		/* TODO fix hard-coded trace distance */
		SlTrace(&refl_cxt, &in->P, &R, .001, 1000, &C_refl, &t_hit);

		Kr = SlFresnel(&in->I, &in->N, 1/sss->ior);
		out->Cs.r += Kr * C_refl.r * sss->reflect.r;
		out->Cs.g += Kr * C_refl.g * sss->reflect.g;
		out->Cs.b += Kr * C_refl.b * sss->reflect.b;
	}

	out->Os = 1;
	out->Os = sss->opacity;
}

static void update_sss_properties(struct SSSShader *sss)
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

static void single_scattering(const struct SSSShader *sss,
		const struct TraceContext *cxt, const struct SurfaceInput *in,
		const struct LightSample *light_sample, struct Color *C_scatter)
{
	const struct Vector *P = &in->P;
	struct Vector Ln = {0, 0, 0};
	struct Vector To = {0, 0, 0};
	struct Vector Li = {0, 0, 0};

	const float *sigma_s = sss->scattering_coeff;
	const float *sigma_t = sss->extinction_coeff;

	const float eta = sss->ior;
	const float one_over_eta = 1/eta;

	const float g = sss->scattering_phase;
	const float g_sq = g * g;

	const int nsamples = sss->single_scattering_samples;
	int i, j;

	const struct TraceContext self_cxt = SlSelfHitContext(cxt, in->shaded_object);
	struct LightOutput Lout;

	struct Color scatter = {0, 0, 0};

	Ln.x = light_sample->P.x - P->x;
	Ln.y = light_sample->P.y - P->y;
	Ln.z = light_sample->P.z - P->z;
	VEC3_NORMALIZE(&Ln);

	Li.x = -Ln.x;
	Li.y = -Ln.y;
	Li.z = -Ln.z;

	SlRefract(&in->I, &in->N, one_over_eta, &To);
	VEC3_NORMALIZE(&To);

	for (i = 0; i < nsamples; i++) {
		struct XorShift *mutable_xr = (struct XorShift *) &sss->xr;
		const float sp_dist = -log(XorNextFloat01(mutable_xr));

		for (j = 0; j < 3; j++) {
			struct Vector P_sample = {0, 0, 0};
			struct Vector Pi = {0, 0, 0};
			struct Vector Ni = {0, 0, 0};
			double si = FLT_MAX;
			double Ln_dot_Ni = 0;

			struct Vector Ti = {0, 0, 0};
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
			Ln_dot_Ni = VEC3_DOT(&Ln, &Ni);

			sp_i = si * Ln_dot_Ni /
					sqrt(1 - one_over_eta * one_over_eta * (1 - Ln_dot_Ni * Ln_dot_Ni));

			SlRefract(&Li, &Ni, one_over_eta, &Ti);
			VEC3_NORMALIZE(&Ti);

			Kri = SlFresnel(&Li, &Ni, one_over_eta);
			Kti = 1 - Kri;

			Ti_dot_To = VEC3_DOT(&Ti, &To);
			phase = (1 - g_sq) / pow(1 + 2 * g * Ti_dot_To + g_sq, 1.5);

			Ni_dot_To = VEC3_DOT(&Ni, &To);
			Ni_dot_Ti = VEC3_DOT(&Ni, &Ti);
			G = ABS(Ni_dot_To) / ABS(Ni_dot_Ti);

			SlIlluminance(cxt, light_sample, &Pi, &Ni, N_PI_2, in, &Lout);

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
	scatter.r *= N_PI * sigma_s[0] / nsamples;
	scatter.g *= N_PI * sigma_s[1] / nsamples;
	scatter.b *= N_PI * sigma_s[2] / nsamples;

	C_scatter->r += scatter.r;
	C_scatter->g += scatter.g;
	C_scatter->b += scatter.b;
}

static void diffusion_scattering(const struct SSSShader *sss,
		const struct TraceContext *cxt, const struct SurfaceInput *in,
		const struct LightSample *light_sample, struct Color *C_scatter)
{
	const struct Vector *P = &in->P;
	const struct Vector *N = &in->N;
	struct Vector Ln = {0, 0, 0};

	struct Vector N_neg = {0, 0, 0};
	struct Vector up = {0, 1, 0};
	struct Vector base1 = {0, 0, 0};
	struct Vector base2 = {0, 0, 0};
	double local_dot_up = 0;

	const float *sigma_s_prime = sss->reduced_scattering_coeff;
	const float *sigma_t_prime = sss->reduced_extinction_coeff;
	const float *sigma_tr = sss->effective_extinction_coeff;

	const float Fdr = sss->diffuse_fresnel_reflectance;
	const float A = (1 + Fdr) / (1 - Fdr);
	float alpha_prime[3] = {0, 0, 0};

	const int nsamples = sss->multiple_scattering_samples;
	int i, j;

	struct Color scatter = {0, 0, 0};

	alpha_prime[0] = sigma_s_prime[0] / sigma_t_prime[0];
	alpha_prime[1] = sigma_s_prime[1] / sigma_t_prime[1];
	alpha_prime[2] = sigma_s_prime[2] / sigma_t_prime[2];

	Ln.x = light_sample->P.x - P->x;
	Ln.y = light_sample->P.y - P->y;
	Ln.z = light_sample->P.z - P->z;
	VEC3_NORMALIZE(&Ln);

	N_neg.x = -N->x;
	N_neg.y = -N->y;
	N_neg.z = -N->z;

	local_dot_up = VEC3_DOT(N, &up);
	if (ABS(local_dot_up) > .9) {
		up.x = 1;
		up.y = 0;
		up.z = 0;
	}

	VEC3_CROSS(&base1, N, &up);
	VEC3_NORMALIZE(&base1);
	VEC3_CROSS(&base2, N, &base1);

	for (i = 0; i < nsamples; i++) {
		struct XorShift *mutable_xr = (struct XorShift *) &sss->xr;
		const double dist_rand = -log(XorNextFloat01(mutable_xr));

		for (j = 0; j < 3; j++) {
			const struct TraceContext self_cxt = SlSelfHitContext(cxt, in->shaded_object);
			struct LightOutput Lout;

			struct Vector P_sample = {0, 0, 0};
			struct Vector2 disk = {0, 0};

			struct Vector P_Pi = {0, 0, 0};
			struct Vector Pi = {0, 0, 0};
			struct Vector Ni = {0, 0, 0};
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

			XorHollowDiskRand(mutable_xr, &disk);
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

			SlIlluminance(cxt, light_sample, &Pi, &Ni, N_PI_2, in, &Lout);

			r = VEC3_LEN(&P_Pi);
			zr = sqrt(3 * (1 - alpha_prime[j])) / sigma_tr[j];
			zv = A * zr;
			dr = sqrt(r * r + zr * zr); /* distance to positive light */
			dv = sqrt(r * r + zv * zv); /* distance to negative light */
			sigma_tr_dr = sigma_tr[j] * dr;
			sigma_tr_dv = sigma_tr[j] * dv;
			Rd = (sigma_tr_dr + 1) * exp(-sigma_tr_dr) * zr / pow(dr, 3) +
				 (sigma_tr_dv + 1) * exp(-sigma_tr_dv) * zr / pow(dv, 3);

			Ln_dot_Ni = VEC3_DOT(&Ln, &Ni);
			Ln_dot_Ni = MAX(0, Ln_dot_Ni);

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

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	struct Color diffuse = {0, 0, 0};

	diffuse.r = MAX(0, value->vector[0]);
	diffuse.g = MAX(0, value->vector[1]);
	diffuse.b = MAX(0, value->vector[2]);
	sss->diffuse = diffuse;

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	struct Color specular = {0, 0, 0};

	specular.r = MAX(0, value->vector[0]);
	specular.g = MAX(0, value->vector[1]);
	specular.b = MAX(0, value->vector[2]);
	sss->specular = specular;

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	struct Color ambient = {0, 0, 0};

	ambient.r = MAX(0, value->vector[0]);
	ambient.g = MAX(0, value->vector[1]);
	ambient.b = MAX(0, value->vector[2]);
	sss->ambient = ambient;

	return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float roughness = value->vector[0];

	roughness = MAX(0, roughness);
	sss->roughness = roughness;

	return 0;
}

static int set_reflect(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	struct Color reflect = {0, 0, 0};

	reflect.r = MAX(0, value->vector[0]);
	reflect.g = MAX(0, value->vector[1]);
	reflect.b = MAX(0, value->vector[2]);
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

static int set_ior(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float ior = value->vector[0];

	ior = MAX(.001, ior);
	sss->ior = ior;

	return 0;
}

static int set_opacity(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float opacity = value->vector[0];

	opacity = CLAMP(opacity, 0, 1);
	sss->opacity = opacity;

	return 0;
}

static int set_diffuse_map(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;

	sss->diffuse_map = value->texture;

	return 0;
}

static int set_enable_single_scattering(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	const int enable = (int) value->vector[0] == 0 ? 0 : 1;

	sss->enable_single_scattering = enable;

	return 0;
}

static int set_enable_multiple_scattering(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	const int enable = (int) value->vector[0] == 0 ? 0 : 1;

	sss->enable_multiple_scattering = enable;

	return 0;
}

static int set_single_scattering_samples(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	int nsamples = (int) value->vector[0];

	nsamples = MAX(1, nsamples);

	sss->single_scattering_samples = nsamples;

	return 0;
}

static int set_multiple_scattering_samples(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	int nsamples = (int) value->vector[0];

	nsamples = MAX(1, nsamples);

	sss->multiple_scattering_samples = nsamples;

	return 0;
}

static int set_scattering_coefficient(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float scattering_coeff[3] = {0, 0, 0};

	scattering_coeff[0] = MAX(0, value->vector[0]);
	scattering_coeff[1] = MAX(0, value->vector[1]);
	scattering_coeff[2] = MAX(0, value->vector[2]);
	scattering_coeff[0] *= 1000; /* 1/mm */
	scattering_coeff[1] *= 1000; /* 1/mm */
	scattering_coeff[2] *= 1000; /* 1/mm */
	COPY3(sss->scattering_coeff, scattering_coeff);

	update_sss_properties(sss);

	return 0;
}

static int set_absorption_coefficient(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float absorption_coeff[3] = {0, 0, 0};

	absorption_coeff[0] = MAX(0, value->vector[0]);
	absorption_coeff[1] = MAX(0, value->vector[1]);
	absorption_coeff[2] = MAX(0, value->vector[2]);
	absorption_coeff[0] *= 1000; /* 1/mm */
	absorption_coeff[1] *= 1000; /* 1/mm */
	absorption_coeff[2] *= 1000; /* 1/mm */
	COPY3(sss->absorption_coeff, absorption_coeff);

	update_sss_properties(sss);

	return 0;
}

static int set_single_scattering_intensity(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float intensity = value->vector[0];

	intensity = MAX(0, intensity);

	sss->single_scattering_intensity = intensity;

	return 0;
}

static int set_multiple_scattering_intensity(void *self, const struct PropertyValue *value)
{
	struct SSSShader *sss = (struct SSSShader *) self;
	float intensity = value->vector[0];

	intensity = MAX(0, intensity);

	sss->multiple_scattering_intensity = intensity;

	return 0;
}

