#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"

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
};

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "PlasticShader";
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
	{"help", "A plastic shader."},
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
	struct PlasticShader *plastic;

	plastic = (struct PlasticShader *) malloc(sizeof(struct PlasticShader));
	if (plastic == NULL)
		return NULL;

	VEC3_SET(plastic->diffuse, .7, .8, .8);
	VEC3_SET(plastic->specular, 1, 1, 1);
	VEC3_SET(plastic->ambient, 1, 1, 1);
	plastic->roughness = .1;

	VEC3_SET(plastic->reflect, 1, 1, 1);
	plastic->ior = 1.4;

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
	const struct PlasticShader *plastic;
	int nlights;
	int i;

	float diff[5] = {0};
	float spec[3] = {0};

	float C_refl[3];
	double refldir[3];
	double Kr;

	struct TraceContext relf_cxt;

	plastic = (struct PlasticShader *) self;
	nlights = SlGetLightCount(in);

	for (i = 0; i < nlights; i++) {
		struct LightOutput Lout;
		float Kd;

		SlIlluminace(cxt, i, in, &Lout);
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

	/* Cs */
	out->Cs[0] = diff[0] * plastic->diffuse[0] + spec[0];
	out->Cs[1] = diff[1] * plastic->diffuse[1] + spec[1];
	out->Cs[2] = diff[2] * plastic->diffuse[2] + spec[2];

	/* reflect */
	if (plastic->reflect[0] > 0 &&
	    plastic->reflect[1] > 0 &&
	    plastic->reflect[2] > 0) {

		relf_cxt = SlReflectContext(cxt, in->shaded_object);
		SlReflect(in->I, in->N, refldir);
		SlTrace(&relf_cxt, in->P, refldir, .001, 1000, C_refl);

		Kr = SlFresnel(in->I, in->N, 1/plastic->ior);
		out->Cs[0] += Kr * C_refl[0] * plastic->reflect[0];
		out->Cs[1] += Kr * C_refl[1] * plastic->reflect[1];
		out->Cs[2] += Kr * C_refl[2] * plastic->reflect[2];
	}
	out->Alpha = 1;
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

	return 0;
}

static int set_ior(void *self, const struct PropertyValue *value)
{
	struct PlasticShader *plastic = (struct PlasticShader *) self;
	float ior = value->vector[0];

	ior = MAX(0, ior);
	plastic->ior = ior;

	return 0;
}

