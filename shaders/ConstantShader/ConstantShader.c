/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Numeric.h"
#include "Vector.h"
#include "Color.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *MyNew(void);
static void MyFree(void *self);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

struct ConstantShader {
	struct Color diffuse;
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

	ColSet(&constant->diffuse, 1, 1, 1);
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
	const struct ConstantShader *constant = (struct ConstantShader *) self;
	struct Color4 C_tex = {0, 0, 0, 0};

	/* C_tex */
	if (constant->texture != NULL) {
		TexLookup(constant->texture, in->uv.u, in->uv.v, &C_tex);
		C_tex.r *= constant->diffuse.r;
		C_tex.g *= constant->diffuse.g;
		C_tex.b *= constant->diffuse.b;
	}
	else {
		C_tex.r = constant->diffuse.r;
		C_tex.g = constant->diffuse.g;
		C_tex.b = constant->diffuse.b;
	}

	/* Cs */
	out->Cs.r = C_tex.r;
	out->Cs.g = C_tex.g;
	out->Cs.b = C_tex.b;
	out->Os = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;
	struct Color diffuse = {0, 0, 0};

	diffuse.r = MAX(0, value->vector[0]);
	diffuse.g = MAX(0, value->vector[1]);
	diffuse.b = MAX(0, value->vector[2]);
	constant->diffuse = diffuse;

	return 0;
}

static int set_texture(void *self, const struct PropertyValue *value)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;

	constant->texture = value->texture;

	return 0;
}

