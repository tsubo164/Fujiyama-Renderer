#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static void MyEvaluate(const void *self, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

struct ConstantShader {
	float diffuse[3];
	struct Texture *texture;
};

static const char MyPluginName[] = "ConstantShader";

static const struct ShaderFunctionTable MyShaderFunctionTable = {
	MyPropertyList,
	MyEvaluate
};

static int set_diffuse(void *self, const struct PropertyValue *value);
static int set_texture(void *self, const struct PropertyValue *value);

static const struct Property PlasticShaderProperties[] = {
	{"diffuse", set_diffuse},
	{"texture", set_texture},
	{NULL,      NULL}
};

static const struct MetaInfo MyShaderMetainfo[] = {
	{"help", "A constant shader."},
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
	struct ConstantShader *constant;

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
	const struct ConstantShader *constant;
	float Ctex[3];

	constant = (struct ConstantShader *) self;

	/* Ctex */
	TexLookup(constant->texture, in->uv[0], in->uv[1], Ctex);

	/* Cs */
	VEC3_COPY(out->Cs, Ctex);
	out->Alpha = 1;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;
	float diffuse[3];

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(constant->diffuse, diffuse);

	return 0;
}

static int set_texture(void *self, const struct PropertyValue *value)
{
	struct ConstantShader *constant = (struct ConstantShader *) self;
	struct Texture *texture = (struct Texture *) value->pointer;

	constant->texture = texture;

	return 0;
}

