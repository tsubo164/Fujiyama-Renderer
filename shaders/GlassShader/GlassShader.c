#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

struct GlassShader {
	float diffuse[3];
	float specular[3];
	float ambient[3];
	float roughness;
	float ior;
};

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "GlassShader";

static const struct ShaderFunctionTable MyShaderFunctionTable = {
	MyPropertyList,
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);

static const struct Property PlasticShaderProperties[] = {
	{"diffuse",   set_diffuse},
	{"specular",  set_specular},
	{"ambient",   set_ambient},
	{"roughness", set_roughness},
	{"ior",       set_ior},
	{NULL,        NULL}
};

static const struct MetaInfo MyShaderMetainfo[] = {
	{"help", "A glass shader."},
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
	struct GlassShader *glass;

	glass = (struct GlassShader *) malloc(sizeof(struct GlassShader));
	if (glass == NULL)
		return NULL;

	VEC3_SET(glass->diffuse, 0, 0, 0);
	VEC3_SET(glass->specular, 1, 1, 1);
	VEC3_SET(glass->ambient, 1, 1, 1);
	glass->roughness = .1;
	glass->ior = 1.4;

	return glass;
}

static void MyFree(void *self)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	if (glass == NULL)
		return;
	free(glass);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	const struct GlassShader *glass;
	struct TraceContext refl_cxt;
	struct TraceContext refr_cxt;
	double T[3], R[3];
	float C_refl[4];
	float C_refr[4];
	double Kt, Kr;

	glass = (struct GlassShader *) self;

	/* Cs */
	out->Cs[0] = 0;
	out->Cs[1] = 0;
	out->Cs[2] = 0;

	Kr = SlFresnel(in->I, in->N, 1/glass->ior);
	Kt = 1 - Kr;

	/* reflect */
	refl_cxt = SlReflectContext(cxt, in->shaded_object);
	SlReflect(in->I, in->N, R);
	SlTrace(&refl_cxt, in->P, R, .0001, 1000, C_refl);
	out->Cs[0] += Kr * C_refl[0];
	out->Cs[1] += Kr * C_refl[1];
	out->Cs[2] += Kr * C_refl[2];

	/* refract_depth */
	refr_cxt = SlRefractContext(cxt, in->shaded_object);
	SlRefract(in->I, in->N, 1/glass->ior, T);
	SlTrace(&refr_cxt, in->P, T, .0001, 1000, C_refr);
	out->Cs[0] += Kt * C_refr[0];
	out->Cs[1] += Kt * C_refr[1];
	out->Cs[2] += Kt * C_refr[2];

	out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float diffuse[3];

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(glass->diffuse, diffuse);

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float specular[3];

	specular[0] = MAX(0, value->vector[0]);
	specular[1] = MAX(0, value->vector[1]);
	specular[2] = MAX(0, value->vector[2]);
	VEC3_COPY(glass->specular, specular);

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float ambient[3];

	ambient[0] = MAX(0, value->vector[0]);
	ambient[1] = MAX(0, value->vector[1]);
	ambient[2] = MAX(0, value->vector[2]);
	VEC3_COPY(glass->ambient, ambient);

	return 0;
}

static int set_roughness(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float roughness = value->vector[0];

	roughness = MAX(0, roughness);
	glass->roughness = roughness;

	return 0;
}

static int set_ior(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float ior = value->vector[0];

	ior = MAX(0, ior);
	glass->ior = ior;

	return 0;
}

