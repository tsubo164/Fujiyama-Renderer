/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "Progress.h"
#include "Numeric.h"
#include "Vector.h"
#include "Mesh.h"

#include <stdlib.h>
/*
#include <string.h>
#include <float.h>
*/
#include <stdio.h>

struct ReadObjProcedure {
	struct Mesh *mesh;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "ReadObjProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyRun
};

static int set_mesh(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_MESH, "mesh", set_mesh},
	{PROP_NONE, NULL,   NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A wavefront obj format reader procedure."},
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
	struct ReadObjProcedure *readobj;

	readobj = (struct ReadObjProcedure *) malloc(sizeof(struct ReadObjProcedure));
	if (readobj == NULL)
		return NULL;

	readobj->mesh = NULL;

	return readobj;
}

static void MyFree(void *self)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;
	if (readobj == NULL)
		return;
	free(readobj);
}

static int MyRun(void *self)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;
	struct Mesh *mesh = readobj->mesh;
	int err = 0;

	printf("****** ReadObjProcedure \n");
	if (mesh == NULL)
		return err;

	/*
	MshAllocateFace(mesh, "indices", 10000);
	*/

	MshClear(mesh);
	MshAllocateVertex(mesh, "P", 3);
	MshAllocateVertex(mesh, "N", 3);
	MshAllocateFace(mesh, "indices", 1);
	{
		double P[3];
		VEC3_SET(P, -1, 0, 0);
		MshSetVertexPosition(mesh, 0, P);
		VEC3_SET(P, 1, 0, 0);
		MshSetVertexPosition(mesh, 1, P);
		VEC3_SET(P, 0, 1, 0);
		MshSetVertexPosition(mesh, 2, P);
	}
	{
		double N[3] = {0, 0, 1};
		MshSetVertexNormal(mesh, 0, N);
		MshSetVertexNormal(mesh, 1, N);
		MshSetVertexNormal(mesh, 2, N);
	}
	{
		int indices[3] = {0, 1, 2};
		MshSetFaceVertexIndices(mesh, 0, indices);
	}
	/*
	*/
	MshComputeBounds(mesh);

	return err;
}

static int set_mesh(void *self, const struct PropertyValue *value)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;

	if (value->mesh == NULL)
		return -1;

	readobj->mesh = value->mesh;

	return 0;
}

