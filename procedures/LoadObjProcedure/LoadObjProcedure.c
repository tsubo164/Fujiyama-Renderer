/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "ObjBuffer.h"
#include "String.h"
#include "Mesh.h"

#include <stdlib.h>

struct LoadObjProcedure {
	char *filename;
	struct Mesh *mesh;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "LoadObjProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyRun
};

static int set_filename(void *self, const struct PropertyValue *value);
static int set_mesh(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_STRING, "filename", set_filename},
	{PROP_MESH,   "mesh",     set_mesh},
	{PROP_NONE,   NULL,       NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A wavefront obj format loader procedure."},
	{"plugin_type", "Procedure"},
	{NULL, NULL}
};

int Initialize(struct PluginInfo *info)
{
	return PlgSetupInfo(info,
			PLUGIN_API_VERSION,
			PROCEDURE_PLUGIN_TYPE,
			MyPluginName,
			MyNew,
			MyFree,
			&MyFunctionTable,
			MyProperties,
			MyMetainfo);
}

static void *MyNew(void)
{
	struct LoadObjProcedure *loader;

	loader = (struct LoadObjProcedure *) malloc(sizeof(struct LoadObjProcedure));
	if (loader == NULL)
		return NULL;

	loader->filename = NULL;
	loader->mesh = NULL;

	return loader;
}

static void MyFree(void *self)
{
	struct LoadObjProcedure *loader = (struct LoadObjProcedure *) self;
	if (loader == NULL)
		return;

	StrFree(loader->filename);
	free(loader);
}

static int MyRun(void *self)
{
	struct LoadObjProcedure *loader = (struct LoadObjProcedure *) self;
	struct ObjBuffer *buffer = ObjBufferNew();
	int err = 0;

	if (buffer == NULL) {
		return -1;
	}

	err = ObjBufferFromFile(buffer, loader->filename);
	if (err) {
		/* TODO error handling */
		return -1;
	}

	err = ObjBufferToMesh(buffer, loader->mesh);
	if (err) {
		/* TODO error handling */
		return -1;
	}

	ObjBufferFree(buffer);
	return 0;
}

static int set_filename(void *self, const struct PropertyValue *value)
{
	struct LoadObjProcedure *loader = (struct LoadObjProcedure *) self;

	if (value->string == NULL)
		return -1;

	loader->filename = StrDup(value->string);

	return 0;
}

static int set_mesh(void *self, const struct PropertyValue *value)
{
	struct LoadObjProcedure *loader = (struct LoadObjProcedure *) self;

	if (value->mesh == NULL)
		return -1;

	loader->mesh = value->mesh;

	return 0;
}

