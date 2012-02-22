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
	float ior;
};

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "HairShader";
static const struct ShaderFunctionTable MyShaderFunctionTable = {
	MyPropertyList,
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_reflect(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);

/* hair shading implementations */
static float kajiya_diffuse(const double *tangent, const double *Ln);
static float kajiya_specular(const double *tangent, const double *Ln, const double *I);

static const struct Property PlasticShaderProperties[] = {
	{"diffuse",   set_diffuse},
	{"specular",  set_specular},
	{"ambient",   set_ambient},
	{"roughness", set_roughness},
	{"reflect",   set_reflect},
	{"ior",       set_ior},
	{NULL,        NULL}
};

static const struct MetaInfo MyShaderMetainfo[] = {
	{"help", "A hair shader."},
	{"plugin_type", "Shader"},
	{NULL, NULL}
};

static const struct Property *MyPropertyList(void)
{
	return PlasticShaderProperties;
}

int Initialize(struct PluginInfo *info)
{
	info->api_version = PLUGIN_API_VERSION;
	info->name = MyPluginName;
	info->create_instance = MyNew;
	info->delete_instance = MyFree;
	info->vtbl = &MyShaderFunctionTable;
	info->meta = MyShaderMetainfo;

	return 0;
}

static void *MyNew(void)
{
	struct HairShader *hair;

	hair = (struct HairShader *) malloc(sizeof(struct HairShader));
	if (hair == NULL)
		return NULL;

	VEC3_SET(hair->diffuse, .7, .8, .8);
	VEC3_SET(hair->specular, 1, 1, 1);
	VEC3_SET(hair->ambient, 1, 1, 1);
	hair->roughness = .1;

	VEC3_SET(hair->reflect, 1, 1, 1);
	hair->ior = 1.4;

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
	const struct HairShader *hair;
	int nlights;
	int i;

	hair = (struct HairShader *) self;
	nlights = SlGetLightCount(in);

	VEC3_SET(out->Cs, 0, 0, 0);
	for (i = 0; i < nlights; i++) {
		struct LightOutput Lout;
		double tangent[3];
		float diff;
		float spec;

		SlIlluminace(cxt, i, in->P, in->N, N_PI, in, &Lout);

		VEC3_COPY(tangent, in->dPdt);
		VEC3_NORMALIZE(tangent);

		diff = kajiya_diffuse(tangent, Lout.Ln);
		spec = kajiya_specular(tangent, Lout.Ln, in->I);

		out->Cs[0] += (in->Cd[0] * diff + spec) * Lout.Cl[0];
		out->Cs[1] += (in->Cd[1] * diff + spec) * Lout.Cl[1];
		out->Cs[2] += (in->Cd[2] * diff + spec) * Lout.Cl[2];
	}
	out->Cs[0] += .0;
	out->Cs[1] += .025;
	out->Cs[2] += .05;

	out->Alpha = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float diffuse[3];

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->diffuse, diffuse);

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float specular[3];

	specular[0] = MAX(0, value->vector[0]);
	specular[1] = MAX(0, value->vector[1]);
	specular[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->specular, specular);

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float ambient[3];

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
	float reflect[3];

	reflect[0] = MAX(0, value->vector[0]);
	reflect[1] = MAX(0, value->vector[1]);
	reflect[2] = MAX(0, value->vector[2]);
	VEC3_COPY(hair->reflect, reflect);

	return 0;
}

static int set_ior(void *self, const struct PropertyValue *value)
{
	struct HairShader *hair = (struct HairShader *) self;
	float ior = value->vector[0];

	ior = MAX(0, ior);
	hair->ior = ior;

	return 0;
}

static float kajiya_diffuse(const double *tangent, const double *Ln)
{
	float diff;
	const float TL = VEC3_DOT(tangent, Ln);

	diff = sqrt(1-TL*TL);

	return diff;
}

static float kajiya_specular(const double *tangent, const double *Ln, const double *I)
{
	float spec;
	const float roughness = .05;
	const float TL = VEC3_DOT(tangent, Ln);
	const float TI = VEC3_DOT(tangent, I);

	assert(-1 <= TL && TL <= 1);
	assert(-1 <= TI && TI <= 1);

	spec = sqrt(1-TL*TL) * sqrt(1-TI*TI) + TL*TI;
	spec = pow(spec, 1/roughness);

	return spec;
}

