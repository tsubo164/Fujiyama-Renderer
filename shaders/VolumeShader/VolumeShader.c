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

struct VolumeShader {
	float diffuse[3];
};

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

static const char MyPluginName[] = "VolumeShader";
static const struct ShaderFunctionTable MyShaderFunctionTable = {
	MyPropertyList,
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);

static const struct Property PlasticShaderProperties[] = {
	{PROP_VECTOR3, "diffuse", set_diffuse},
	{PROP_NONE,    NULL,      NULL}
};

static const struct MetaInfo MyShaderMetainfo[] = {
	{"help", "A volume shader."},
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
	struct VolumeShader *volume = NULL;

	volume = (struct VolumeShader *) malloc(sizeof(struct VolumeShader));
	if (volume == NULL)
		return NULL;

	VEC3_SET(volume->diffuse, 1, 1, 1);

	return volume;
}

static void MyFree(void *self)
{
	struct VolumeShader *volume = (struct VolumeShader *) self;
	if (volume == NULL)
		return;
	free(volume);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	const struct VolumeShader *volume = NULL;
	int nlights = 0;
	int i = 0;

	float diff[3] = {0};

	volume = (struct VolumeShader *) self;
	nlights = SlGetLightCount(in);

	for (i = 0; i < nlights; i++) {
		struct LightOutput Lout;

		SlIlluminace(cxt, i, in->P, in->N, N_PI, in, &Lout);

		/* diff */
		diff[0] += Lout.Cl[0];
		diff[1] += Lout.Cl[1];
		diff[2] += Lout.Cl[2];
	}

	/* Cs */
	out->Cs[0] = diff[0] * volume->diffuse[0];
	out->Cs[1] = diff[1] * volume->diffuse[1];
	out->Cs[2] = diff[2] * volume->diffuse[2];

	/* Os */
	out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct VolumeShader *volume = (struct VolumeShader *) self;
	float diffuse[3] = {0};

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(volume->diffuse, diffuse);

	return 0;
}

