/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Shader.h"
#include "Vector.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int error_no = ERR_SHD_NOERR;
static const float NO_SHADER_COLOR[] = {.5, 1., 0.};

struct Shader {
	void *self;
	const struct ShaderFunctionTable *vptr;
	const struct Plugin *plg;
};

struct Shader *ShdNew(const struct Plugin *plg)
{
	struct Shader *shader = NULL;
	void *tmpobj = NULL;
	const void *tmpvtbl = NULL;

	tmpobj = PlgCreateInstance(plg);
	if (tmpobj == NULL) {
		error_no = ERR_SHD_NOOBJ;
		return NULL;
	}

	tmpvtbl = PlgGetVtable(plg);
	if (tmpvtbl == NULL) {
		error_no = ERR_SHD_NOVTBL;
		return NULL;
	}

	shader = (struct Shader *) malloc(sizeof(struct Shader));
	if (shader == NULL) {
		error_no = ERR_SHD_NOMEM;
		return NULL;
	}

	/* commit */
	shader->self = tmpobj;
	shader->vptr = tmpvtbl;
	shader->plg = plg;
	error_no = ERR_SHD_NOERR;

	return shader;
}

void ShdFree(struct Shader *shader)
{
	if (shader == NULL)
		return;

	PlgDeleteInstance(shader->plg, shader->self);
	free(shader);
}

void ShdEvaluate(const struct Shader *shader, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out)
{
	if (shader == NULL) {
		VEC3_COPY(out->Cs, NO_SHADER_COLOR);
		out->Os = 1;
		return;
	}
	shader->vptr->MyEvaluate(shader->self, cxt, in, out);
}

const struct Property *ShdGetPropertyList(const struct Shader *shader)
{
	return shader->vptr->MyPropertyList();
}

int ShdSetProperty(struct Shader *shader,
		const char *prop_name, const struct PropertyValue *src_data)
{
	const struct Property *shd_props;
	const struct Property *dst_prop;

	shd_props = ShdGetPropertyList(shader);
	dst_prop = PropFind(shd_props, prop_name);
	if (dst_prop == NULL)
		return -1;

	assert(dst_prop->SetProperty != NULL);
	return dst_prop->SetProperty(shader->self, src_data);
}

