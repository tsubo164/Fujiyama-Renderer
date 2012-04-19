/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PROCEDURE_H
#define PROCEDURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Property.h"
#include "Plugin.h"

struct Procedure;

struct ProcedureFunctionTable {
	const struct Property *(*MyPropertyList)(void);
	int (*MyProcess)(void *self);
};

enum PrcErrorNo {
	ERR_PRC_NOERR = 0,
	ERR_PRC_NOOBJ,
	ERR_PRC_NOVTBL,
	ERR_PRC_NOMEM
};

extern struct Procedure *PrcNew(const struct Plugin *plugin);
extern void PrcFree(struct Procedure *procedure);

extern int PrcProcess(struct Procedure *procedure);

extern const struct Property *PrcGetPropertyList(const struct Procedure *procedure);
extern int PrcSetProperty(struct Procedure *procedure,
		const char *prop_name, const struct PropertyValue *src_data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

