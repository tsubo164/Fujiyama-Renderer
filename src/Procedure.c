/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int error_no = PRC_ERR_NOERR;

struct Procedure {
	void *self;
	const struct ProcedureFunctionTable *vptr;
	const struct Plugin *plugin;
};

static void set_error(int err);

struct Procedure *PrcNew(const struct Plugin *plugin)
{
	struct Procedure *procedure;
	const void *tmpvtbl;
	void *tmpobj;

	tmpobj = PlgCreateInstance(plugin);
	if (tmpobj == NULL) {
		set_error(PRC_ERR_NOOBJ);
		return NULL;
	}

	tmpvtbl = PlgGetVtable(plugin);
	if (tmpvtbl == NULL) {
		set_error(PRC_ERR_NOVTBL);
		PlgDeleteInstance(plugin, tmpobj);
		return NULL;
	}

	procedure = (struct Procedure *) malloc(sizeof(struct Procedure));
	if (procedure == NULL) {
		set_error(PRC_ERR_NOMEM);
		PlgDeleteInstance(plugin, tmpobj);
		return NULL;
	}

	/* commit */
	procedure->self = tmpobj;
	procedure->vptr = (const struct ProcedureFunctionTable *) tmpvtbl;
	procedure->plugin = plugin;
	set_error(PRC_ERR_NOERR);

	return procedure;
}

void PrcFree(struct Procedure *procedure)
{
	if (procedure == NULL)
		return;

	PlgDeleteInstance(procedure->plugin, procedure->self);
	free(procedure);
}

int PrcRun(struct Procedure *procedure)
{
	return procedure->vptr->MyRun(procedure->self);
}

const struct Property *PrcGetPropertyList(const struct Procedure *procedure)
{
	return procedure->vptr->MyPropertyList();
}

int PrcSetProperty(struct Procedure *procedure,
		const char *prop_name, const struct PropertyValue *src_data)
{
	const struct Property *prc_props;
	const struct Property *dst_prop;

	prc_props = PrcGetPropertyList(procedure);
	dst_prop = PropFind(prc_props, src_data->type, prop_name);
	if (dst_prop == NULL)
		return -1;

	assert(dst_prop->SetProperty != NULL);
	return dst_prop->SetProperty(procedure->self, src_data);
}

static void set_error(int err)
{
	error_no = err;
}

