/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SHADER_H
#define SHADER_H

#include "Property.h"
#include "Texture.h"
#include "Plugin.h"
#include "SL.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Shader;

struct ShaderFunctionTable {
	const struct Property *(*MyPropertyList)(void);
	void (*MyEvaluate)(const void *self, const struct TraceContext *cxt,
			const struct SurfaceInput *in, struct SurfaceOutput *out);
};

enum ShdErrorNo {
	SHD_ERR_NOERR = 0,
	SHD_ERR_NOOBJ,
	SHD_ERR_NOVTBL,
	SHD_ERR_NOMEM
};

extern struct Shader *ShdNew(const struct Plugin *plg);
extern void ShdFree(struct Shader *shader);

extern void ShdEvaluate(const struct Shader *shader, const struct TraceContext *cxt,
		const struct SurfaceInput *in, struct SurfaceOutput *out);

extern const struct Property *ShdGetPropertyList(const struct Shader *shader);
extern int ShdSetProperty(struct Shader *shader,
		const char *prop_name, const struct PropertyValue *src_data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

