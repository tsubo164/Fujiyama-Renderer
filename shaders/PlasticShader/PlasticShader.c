/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Numeric.h"
#include "Memory.h"
#include "Vector.h"
#include "Color.h"

#include <string.h>
#include <stdio.h>
#include <float.h>

struct PlasticShader {
	struct Color diffuse;
	struct Color specular;
	struct Color ambient;
	float roughness;

	struct Color reflect;
	float ior;

	float opacity;

	int do_reflect;

	struct Texture *diffuse_map;
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
static int set_diffuse_map(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "diffuse",     set_diffuse},
	{PROP_VECTOR3, "specular",    set_specular},
	{PROP_VECTOR3, "ambient",     set_ambient},
	{PROP_SCALAR,  "roughness",   set_roughness},
	{PROP_VECTOR3, "reflect",     set_reflect},
	{PROP_SCALAR,  "ior",         set_ior},
	{PROP_SCALAR,  "opacity",     set_opacity},
	{PROP_TEXTURE, "diffuse_map", set_diffuse_map},
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
	struct PlasticShader *plastic = MEM_ALLOC(struct PlasticShader);

	if (plastic == NULL)
		return NULL;

	ColSet(&plastic->diffuse, .7, .8, .8);
	ColSet(&plastic->specular, 1, 1, 1);
	ColSet(&plastic->ambient, 1, 1, 1);
	plastic->roughness = .1;

	ColSet(&plastic->reflect, 1, 1, 1);
	plastic->ior = 1.4;

	plastic->opacity = 1;

	plastic->do_reflect = 1;

	plastic->diffuse_map = NULL;

	return plastic;
}

static void MyFree(void *self)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	if (plastic == NULL)
		return;
	MEM_FREE(plastic);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	const struct PlasticShader *plastic = (struct PlasticShader *) self;
	struct Color diff = {0, 0, 0};
	struct Color spec = {0, 0, 0};
	struct Color4 diff_map = {1, 1, 1, 1};
	int i = 0;

	struct LightSample *samples = NULL;
	const int nsamples = SlGetLightSampleCount(in);

	/* allocate samples */
	samples = SlNewLightSamples(in);

	for (i = 0; i < nsamples; i++) {
		struct LightOutput Lout;
		float Kd = 0;
		SlIlluminance(cxt, &samples[i], &in->P, &in->N, N_PI_2, in, &Lout);
		/* spec */
		/*
		Ks = SlPhong(in->I, in->N, Ln, .05);
		*/

		/* diff */
		Kd = VEC3_DOT(&in->N, &Lout.Ln);
		Kd = MAX(0, Kd);
		diff.r += Kd * Lout.Cl.r;
		diff.g += Kd * Lout.Cl.g;
		diff.b += Kd * Lout.Cl.b;
	}

	/* free samples */
	SlFreeLightSamples(samples);

	/* diffuse map */
	if (plastic->diffuse_map != NULL) {
		TexLookup(plastic->diffuse_map, in->uv.u, in->uv.v, &diff_map);
	}

	/* Cs */
	out->Cs.r = diff.r * plastic->diffuse.r * diff_map.r + spec.r;
	out->Cs.g = diff.g * plastic->diffuse.g * diff_map.g + spec.g;
	out->Cs.b = diff.b * plastic->diffuse.b * diff_map.b + spec.b;

	/* reflect */
	if (plastic->do_reflect) {
		struct Color4 C_refl = {0, 0, 0, 0};
		struct Vector R = {0, 0, 0};
		double t_hit = FLT_MAX;
		double Kr = 0;

		const struct TraceContext refl_cxt = SlReflectContext(cxt, in->shaded_object);

		SlReflect(&in->I, &in->N, &R);
		VEC3_NORMALIZE(&R);
		SlTrace(&refl_cxt, &in->P, &R, .001, 1000, &C_refl, &t_hit);

		Kr = SlFresnel(&in->I, &in->N, 1/plastic->ior);
		out->Cs.r += Kr * C_refl.r * plastic->reflect.r;
		out->Cs.g += Kr * C_refl.g * plastic->reflect.g;
		out->Cs.b += Kr * C_refl.b * plastic->reflect.b;
	}

	out->Os = 1;
	out->Os = plastic->opacity;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	struct Color diffuse;

	diffuse.r = MAX(0, value->vector[0]);
	diffuse.g = MAX(0, value->vector[1]);
	diffuse.b = MAX(0, value->vector[2]);
	plastic->diffuse = diffuse;

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	struct Color specular;

	specular.r = MAX(0, value->vector[0]);
	specular.g = MAX(0, value->vector[1]);
	specular.b = MAX(0, value->vector[2]);
	plastic->specular = specular;

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	struct Color ambient;

	ambient.r = MAX(0, value->vector[0]);
	ambient.g = MAX(0, value->vector[1]);
	ambient.b = MAX(0, value->vector[2]);
	plastic->ambient = ambient;

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
	struct Color reflect;

	reflect.r = MAX(0, value->vector[0]);
	reflect.g = MAX(0, value->vector[1]);
	reflect.b = MAX(0, value->vector[2]);
	plastic->reflect = reflect;

	if (plastic->reflect.r > 0 ||
		plastic->reflect.g > 0 ||
		plastic->reflect.b > 0 ) {
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

static int set_diffuse_map(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;

	plastic->diffuse_map = value->texture;

	return 0;
}

