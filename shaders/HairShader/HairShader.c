/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>

struct HairShader {
	float diffuse[3];
	float specular[3];
	float ambient[3];
	float roughness;

	float reflect[3];
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "HairShader";
static const struct ShaderFunctionTable MyFunctionTable = {
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_reflect(void *self, const struct PropertyValue *value);

/* hair shading implementations */
static float kajiya_diffuse(const double *tangent, const double *Ln);
static float kajiya_specular(const double *tangent, const double *Ln, const double *I);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "diffuse",   set_diffuse},
	{PROP_VECTOR3, "specular",  set_specular},
	{PROP_VECTOR3, "ambient",   set_ambient},
	{PROP_SCALAR,  "roughness", set_roughness},
	{PROP_VECTOR3, "reflect",   set_reflect},
	{PROP_NONE,    NULL,        NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A hair shader."},
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
	struct HairShader *hair = NULL;

	hair = (struct HairShader *) malloc(sizeof(struct HairShader));
	if (hair == NULL)
		return NULL;

	VEC3_SET(hair->diffuse, .7, .8, .8);
	VEC3_SET(hair->specular, 1, 1, 1);
	VEC3_SET(hair->ambient, 1, 1, 1);
	hair->roughness = .1;

	VEC3_SET(hair->reflect, 1, 1, 1);

	return hair;
}

static void MyFree(void *self)
{
	struct HairShader *hair = (struct HairShader *) self;
	if (hair == NULL)
		return;
	free(hair);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	/* TODO use hair */
	/*
	const struct HairShader *hair = (struct HairShader *) self;
	*/
	int i = 0;

	struct LightSample *samples = NULL;
	const int nsamples = SlGetLightSampleCount(in);

	/* allocate samples */
	samples = SlNewLightSamples(in);

	VEC3_SET(out->Cs, 0, 0, 0);

	for (i = 0; i < nsamples; i++) {
		struct LightOutput Lout = {{0}};
		double tangent[3] = {0};
		float diff = 0;
		float spec = 0;

		SlIlluminance(cxt, &samples[i], in->P, in->N, N_PI, in, &Lout);

		VEC3_COPY(tangent, in->dPdt);
		VEC3_NORMALIZE(tangent);

		diff = kajiya_diffuse(tangent, Lout.Ln);
		spec = kajiya_specular(tangent, Lout.Ln, in->I);

		out->Cs[0] += (in->Cd[0] * diff + spec) * Lout.Cl[0];
		out->Cs[1] += (in->Cd[1] * diff + spec) * Lout.Cl[1];
		out->Cs[2] += (in->Cd[2] * diff + spec) * Lout.Cl[2];
	}

	/* free samples */
	SlFreeLightSamples(samples);

	out->Cs[0] += .0;
	out->Cs[1] += .025;
	out->Cs[2] += .05;

	out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float diffuse[3] = {0};

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->diffuse, diffuse);

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float specular[3] = {0};

	specular[0] = MAX(0, value->vector[0]);
	specular[1] = MAX(0, value->vector[1]);
	specular[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->specular, specular);

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float ambient[3] = {0};

	ambient[0] = MAX(0, value->vector[0]);
	ambient[1] = MAX(0, value->vector[1]);
	ambient[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->ambient, ambient);

	return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float roughness = value->vector[0];

	roughness = MAX(0, roughness);
	hair->roughness = roughness;

	return 0;
}

static int set_reflect(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float reflect[3] = {0};

	reflect[0] = MAX(0, value->vector[0]);
	reflect[1] = MAX(0, value->vector[1]);
	reflect[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->reflect, reflect);

	return 0;
}

static float kajiya_diffuse(const double *tangent, const double *Ln)
{
	const float TL = VEC3_DOT(tangent, Ln);
	const float diff = sqrt(1-TL*TL);

	return diff;
}

static float kajiya_specular(const double *tangent, const double *Ln, const double *I)
{
	float spec = 0;
	const float roughness = .05;
	const float TL = VEC3_DOT(tangent, Ln);
	const float TI = VEC3_DOT(tangent, I);

	assert(-1 <= TL && TL <= 1);
	assert(-1 <= TI && TI <= 1);

	spec = sqrt(1-TL*TL) * sqrt(1-TI*TI) + TL*TI;
	spec = pow(spec, 1/roughness);

	return spec;
}

