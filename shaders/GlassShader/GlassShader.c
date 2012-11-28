/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

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

	float filter_color[3];

	float roughness;
	float ior;

	int do_color_filter;
};

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "GlassShader";

static const struct ShaderFunctionTable MyFunctionTable = {
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_specular(void *self, const struct PropertyValue *value);
static int set_ambient(void *self, const struct PropertyValue *value);
static int set_filter_color(void *self, const struct PropertyValue *value);
static int set_roughness(void *self, const struct PropertyValue *value);
static int set_ior(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "diffuse",      set_diffuse},
	{PROP_VECTOR3, "specular",     set_specular},
	{PROP_VECTOR3, "ambient",      set_ambient},
	{PROP_VECTOR3, "filter_color", set_filter_color},
	{PROP_SCALAR,  "roughness",    set_roughness},
	{PROP_SCALAR,  "ior",          set_ior},
	{PROP_NONE,    NULL,           NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A glass shader."},
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
	struct GlassShader *glass = NULL;

	glass = (struct GlassShader *) malloc(sizeof(struct GlassShader));
	if (glass == NULL)
		return NULL;

	VEC3_SET(glass->diffuse, 0, 0, 0);
	VEC3_SET(glass->specular, 1, 1, 1);
	VEC3_SET(glass->ambient, 1, 1, 1);
	VEC3_SET(glass->filter_color, 1, 1, 1);
	glass->roughness = .1;
	glass->ior = 1.4;

	glass->do_color_filter = 0;

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
	const struct GlassShader *glass = (struct GlassShader *) self;
	struct TraceContext refl_cxt;
	struct TraceContext refr_cxt;
	double T[3] = {0};
	double R[3] = {0};
	float C_refl[4] = {0};
	float C_refr[4] = {0};
	double Kt = 0, Kr = 0;
	double t_hit = FLT_MAX;

	/* Cs */
	out->Cs[0] = 0;
	out->Cs[1] = 0;
	out->Cs[2] = 0;

	Kr = SlFresnel(in->I, in->N, 1/glass->ior);
	Kt = 1 - Kr;

	/* reflect */
	refl_cxt = SlReflectContext(cxt, in->shaded_object);
	SlReflect(in->I, in->N, R);
	SlTrace(&refl_cxt, in->P, R, .0001, 1000, C_refl, &t_hit);
	out->Cs[0] += Kr * C_refl[0];
	out->Cs[1] += Kr * C_refl[1];
	out->Cs[2] += Kr * C_refl[2];

	/* refract */
	refr_cxt = SlRefractContext(cxt, in->shaded_object);
	SlRefract(in->I, in->N, 1/glass->ior, T);
	SlTrace(&refr_cxt, in->P, T, .0001, 1000, C_refr, &t_hit);

	if (glass->do_color_filter && VEC3_DOT(in->I, in->N) < 0) {
		C_refr[0] *= pow(glass->filter_color[0], t_hit);
		C_refr[1] *= pow(glass->filter_color[1], t_hit);
		C_refr[2] *= pow(glass->filter_color[2], t_hit);
	}

	out->Cs[0] += Kt * C_refr[0];
	out->Cs[1] += Kt * C_refr[1];
	out->Cs[2] += Kt * C_refr[2];

	out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float diffuse[3] = {0};

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(glass->diffuse, diffuse);

	return 0;
}

static int set_specular(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float specular[3] = {0};

	specular[0] = MAX(0, value->vector[0]);
	specular[1] = MAX(0, value->vector[1]);
	specular[2] = MAX(0, value->vector[2]);
	VEC3_COPY(glass->specular, specular);

	return 0;
}

static int set_ambient(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float ambient[3] = {0};

	ambient[0] = MAX(0, value->vector[0]);
	ambient[1] = MAX(0, value->vector[1]);
	ambient[2] = MAX(0, value->vector[2]);
	VEC3_COPY(glass->ambient, ambient);

	return 0;
}

static int set_filter_color(void *self, const struct PropertyValue *value)
{
	struct GlassShader *glass = (struct GlassShader *) self;
	float filter_color[3] = {0};

	filter_color[0] = MAX(.001, value->vector[0]);
	filter_color[1] = MAX(.001, value->vector[1]);
	filter_color[2] = MAX(.001, value->vector[2]);
	VEC3_COPY(glass->filter_color, filter_color);

	if (glass->filter_color[0] == 1 &&
		glass->filter_color[1] == 1 &&
		glass->filter_color[2] == 1) {
		glass->do_color_filter = 0;
	}
	else {
		glass->do_color_filter = 1;
	}

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

