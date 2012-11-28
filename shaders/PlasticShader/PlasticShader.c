/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"
#include "Noise.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

struct PlasticShader {
	float diffuse[3];
	float specular[3];
	float ambient[3];
	float roughness;

	float reflect[3];
	float ior;

	float opacity;

	int do_reflect;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "PlasticShader";
static const struct ShaderFunctionTable MyFunctionTable = {
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_reflect(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);
static int set_opacity(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "diffuse",   set_diffuse},
	{PROP_VECTOR3, "specular",  set_specular},
	{PROP_VECTOR3, "ambient",   set_ambient},
	{PROP_SCALAR,  "roughness", set_roughness},
	{PROP_VECTOR3, "reflect",   set_reflect},
	{PROP_SCALAR,  "ior",       set_ior},
	{PROP_SCALAR,  "opacity",   set_opacity},
	{PROP_NONE, NULL, NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A plastic shader."},
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
	struct PlasticShader *plastic =
			(struct PlasticShader *) malloc(sizeof(struct PlasticShader));

	if (plastic == NULL)
		return NULL;

	VEC3_SET(plastic->diffuse, .7, .8, .8);
	VEC3_SET(plastic->specular, 1, 1, 1);
	VEC3_SET(plastic->ambient, 1, 1, 1);
	plastic->roughness = .1;

	VEC3_SET(plastic->reflect, 1, 1, 1);
	plastic->ior = 1.4;

	plastic->opacity = 1;

	plastic->do_reflect = 1;

	return plastic;
}

static void MyFree(void *self)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	if (plastic == NULL)
		return;
	free(plastic);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	const struct PlasticShader *plastic = (struct PlasticShader *) self;
	float diff[3] = {0};
	float spec[3] = {0};
	int i = 0;

	struct LightSample *samples = NULL;
	const int nsamples = SlGetLightSampleCount(in);

	samples = SlNewLightSamples(in);

	for (i = 0; i < nsamples; i++) {
		struct LightOutput Lout;
		float Kd = 0;
		SlSampleIlluminance(cxt, &samples[i], in->P, in->N, N_PI_2, in, &Lout);
		/* spec */
		/*
		Ks = SlPhong(in->I, in->N, Ln, .05);
		*/

		/* diff */
		Kd = VEC3_DOT(in->N, Lout.Ln);
		Kd = MAX(0, Kd);
		diff[0] += Kd * Lout.Cl[0];
		diff[1] += Kd * Lout.Cl[1];
		diff[2] += Kd * Lout.Cl[2];
	}

	SlFreeLightSamples(samples);

	/* Cs */
	out->Cs[0] = diff[0] * plastic->diffuse[0] + spec[0];
	out->Cs[1] = diff[1] * plastic->diffuse[1] + spec[1];
	out->Cs[2] = diff[2] * plastic->diffuse[2] + spec[2];

	/* reflect */
	if (plastic->do_reflect) {
		float C_refl[4] = {0};
		double refldir[3] = {0};
		double t_hit = FLT_MAX;
		double Kr = 0;

		const struct TraceContext relf_cxt = SlReflectContext(cxt, in->shaded_object);

		SlReflect(in->I, in->N, refldir);
		SlTrace(&relf_cxt, in->P, refldir, .001, 1000, C_refl, &t_hit);

		Kr = SlFresnel(in->I, in->N, 1/plastic->ior);
		out->Cs[0] += Kr * C_refl[0] * plastic->reflect[0];
		out->Cs[1] += Kr * C_refl[1] * plastic->reflect[1];
		out->Cs[2] += Kr * C_refl[2] * plastic->reflect[2];
	}

	out->Os = 1;
	out->Os = plastic->opacity;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float diffuse[3];

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(plastic->diffuse, diffuse);

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float specular[3];

	specular[0] = MAX(0, value->vector[0]);
	specular[1] = MAX(0, value->vector[1]);
	specular[2] = MAX(0, value->vector[2]);
	VEC3_COPY(plastic->specular, specular);

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float ambient[3];

	ambient[0] = MAX(0, value->vector[0]);
	ambient[1] = MAX(0, value->vector[1]);
	ambient[2] = MAX(0, value->vector[2]);
	VEC3_COPY(plastic->ambient, ambient);

	return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float roughness = value->vector[0];

	roughness = MAX(0, roughness);
	plastic->roughness = roughness;

	return 0;
}

static int set_reflect(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float reflect[3];

	reflect[0] = MAX(0, value->vector[0]);
	reflect[1] = MAX(0, value->vector[1]);
	reflect[2] = MAX(0, value->vector[2]);
	VEC3_COPY(plastic->reflect, reflect);

	if (plastic->reflect[0] > 0 ||
		plastic->reflect[1] > 0 ||
		plastic->reflect[2] > 0 ) {
		plastic->do_reflect = 1;
	}
	else {
		plastic->do_reflect = 0;
	}

	return 0;
}

static int set_ior(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float ior = value->vector[0];

	ior = MAX(.001, ior);
	plastic->ior = ior;

	return 0;
}

static int set_opacity(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float opacity = value->vector[0];

	opacity = CLAMP(opacity, 0, 1);
	plastic->opacity = opacity;

	return 0;
}

