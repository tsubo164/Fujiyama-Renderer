/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

struct ConstantShader {
	float diffuse[3];
	struct Texture *texture;
};

static const char MyPluginName[] = "ConstantShader";

static const struct ShaderFunctionTable MyFunctionTable = {
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_texture(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "diffuse", set_diffuse},
	{PROP_TEXTURE, "texture", set_texture},
	{PROP_NONE,    NULL,      NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A constant shader."},
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
	struct ConstantShader *constant = NULL;

	constant = (struct ConstantShader *) malloc(sizeof(struct ConstantShader));
	if (constant == NULL)
		return NULL;

	VEC3_SET(constant->diffuse, 1, 1, 1);
	constant->texture = NULL;

	return constant;
}

static void MyFree(void *self)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;
	if (constant == NULL)
		return;
	free(constant);
}

static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	const struct ConstantShader *constant = NULL;
	float C_tex[3] = {0};

	constant = (struct ConstantShader *) self;

	/* C_tex */
	if (constant->texture != NULL) {
		TexLookup(constant->texture, in->uv[0], in->uv[1], C_tex);
		C_tex[0] *= constant->diffuse[0];
		C_tex[1] *= constant->diffuse[1];
		C_tex[2] *= constant->diffuse[2];
	}
	else {
		C_tex[0] = constant->diffuse[0];
		C_tex[1] = constant->diffuse[1];
		C_tex[2] = constant->diffuse[2];
	}

	/* Cs */
	VEC3_COPY(out->Cs, C_tex);
	out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;
	float diffuse[3] = {0};

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(constant->diffuse, diffuse);

	return 0;
}

static int set_texture(void *self, const struct PropertyValue *value)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;

	constant->texture = value->texture;

	return 0;
}

