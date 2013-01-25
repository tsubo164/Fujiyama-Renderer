/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "ObjParser.h"
#include "Progress.h"
#include "Numeric.h"
#include "String.h"
#include "Vector.h"
#include "Mesh.h"

#include <stdlib.h>
/*
#include <string.h>
#include <float.h>
*/
#include <stdio.h>

struct ReadObjProcedure {
	char *filename;
	struct Mesh *mesh;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "ReadObjProcedure";
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
	{"help", "A wavefront obj format reader procedure."},
	{"plugin_type", "Procedure"},
	{NULL, NULL}
};

static int build_mesh(struct ReadObjProcedure *readobj);

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

	readobj->filename = NULL;
	readobj->mesh = NULL;

	return readobj;
}

static void MyFree(void *self)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;
	if (readobj == NULL)
		return;

	StrFree(readobj->filename);
	free(readobj);
}

static int MyRun(void *self)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;

	return build_mesh(readobj);
#if 0
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
	MshComputeBounds(mesh);
#endif
}

static int set_filename(void *self, const struct PropertyValue *value)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;

	if (value->string == NULL)
		return -1;

	readobj->filename = StrDup(value->string);
	printf("++++++++++ [%s]\n", readobj->filename);

	return 0;
}

static int set_mesh(void *self, const struct PropertyValue *value)
{
	struct ReadObjProcedure *readobj = (struct ReadObjProcedure *) self;

	if (value->mesh == NULL)
		return -1;

	readobj->mesh = value->mesh;

	return 0;
}

struct ObjPrinter {
	long nverts;
	long nfaces;
};

static int print_vertx(
		void *interpreter,
		int scanned_ncomponents,
		double x,
		double y,
		double z,
		double w)
{
	struct ObjPrinter *obj = (struct ObjPrinter *) interpreter;

	printf("VERTEX ");
	printf("%ld: (%g", obj->nverts, x);
	if (scanned_ncomponents > 1)
		printf(", %g", y);
	if (scanned_ncomponents > 2)
		printf(", %g", z);
	if (scanned_ncomponents > 3)
		printf(", %g", w);
	printf(")\n");

	obj->nverts++;
	return 0;
}

static int print_texture(
		void *interpreter,
		int scanned_ncomponents,
		double x,
		double y,
		double z,
		double w)
{
	struct ObjPrinter *obj = (struct ObjPrinter *) interpreter;

	printf("TEXTURE ");
	printf("%ld: (%g", obj->nverts, x);
	if (scanned_ncomponents > 1)
		printf(", %g", y);
	if (scanned_ncomponents > 2)
		printf(", %g", z);
	if (scanned_ncomponents > 3)
		printf(", %g", w);
	printf(")\n");

	obj->nverts++;
	return 0;
}

static int print_normal(
		void *interpreter,
		int scanned_ncomponents,
		double x,
		double y,
		double z,
		double w)
{
	struct ObjPrinter *obj = (struct ObjPrinter *) interpreter;

	printf("NORMAL ");
	printf("%ld: (%g", obj->nverts, x);
	if (scanned_ncomponents > 1)
		printf(", %g", y);
	if (scanned_ncomponents > 2)
		printf(", %g", z);
	if (scanned_ncomponents > 3)
		printf(", %g", w);
	printf(")\n");

	obj->nverts++;
	return 0;
}

static int print_face(
		void *interpreter,
		long index_count,
		const long *vertex_indices,
		const long *texture_indices,
		const long *normal_indices)
{
	struct ObjPrinter *obj = (struct ObjPrinter *) interpreter;
	int i;

	printf("FACE ");
	printf("%ld:%ld: ", obj->nfaces, index_count);

	for (i = 0; i < index_count; i++) {
		printf("(");
		if (vertex_indices != NULL)
			printf("%ld", vertex_indices[i]);

		printf(", ");

		if (texture_indices != NULL)
			printf("%ld", texture_indices[i]);

		printf(", ");

		if (normal_indices != NULL)
			printf("%ld", normal_indices[i]);

		printf(") ");
	}
	printf("\n");

	obj->nfaces++;
	return 0;
}

static int build_mesh(struct ReadObjProcedure *readobj)
{
	struct Mesh *mesh = readobj->mesh;
	struct ObjParser *parser = NULL;
	struct ObjPrinter obj;
	int err = 0;

	if (mesh == NULL)
		return -1;

	if (readobj->filename == NULL)
		return -1;

	obj.nverts = 0;
	obj.nfaces = 0;

	parser = ObjParserNew(
			&obj,
			print_vertx,
			print_texture,
			print_normal,
			print_face);

	err = ObjParse(parser, readobj->filename);

	if (err) {
		fprintf(stderr, "parse error: worng format\n");
		exit(EXIT_FAILURE);
	}

	ObjParserFree(parser);

	return 0;
}

