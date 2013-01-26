/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjBuffer.h"
#include "ObjParser.h"
#include "Triangle.h"
#include "Numeric.h"
#include "Vector.h"
#include "Array.h"
#include "Mesh.h"

#include <stdlib.h>

struct ObjBuffer {
	struct Array *P;
	struct Array *N;
	struct Array *uv;
	struct Array *vertex_indices;
	struct Array *texture_indices;
	struct Array *normal_indices;

	long nverts;
	long nfaces;
};

struct ObjBuffer *ObjBufferNew(void)
{
	struct ObjBuffer *buffer = (struct ObjBuffer *) malloc(sizeof(struct ObjBuffer));
	if (buffer == NULL)
		return NULL;

	buffer->P = ArrNew(sizeof(double));
	buffer->N = ArrNew(sizeof(double));
	buffer->uv = ArrNew(sizeof(float));
	buffer->vertex_indices = ArrNew(sizeof(int));
	buffer->texture_indices = ArrNew(sizeof(int));
	buffer->normal_indices = ArrNew(sizeof(int));

	buffer->nverts = 0;
	buffer->nfaces = 0;

	return buffer;
}

void ObjBufferFree(struct ObjBuffer *buffer)
{
	if (buffer == NULL)
		return;

	ArrFree(buffer->P);
	ArrFree(buffer->N);
	ArrFree(buffer->uv);
	ArrFree(buffer->vertex_indices);
	ArrFree(buffer->texture_indices);
	ArrFree(buffer->normal_indices);

	free(buffer);
}

static int read_vertx(
		void *interpreter,
		int scanned_ncomponents,
		double x,
		double y,
		double z,
		double w)
{
	struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;

	ArrPush(buffer->P, &x);
	ArrPush(buffer->P, &y);
	ArrPush(buffer->P, &z);

	buffer->nverts++;
	return 0;
}

static int read_texture(
		void *interpreter,
		int scanned_ncomponents,
		double x,
		double y,
		double z,
		double w)
{
	struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;

	ArrPush(buffer->uv, &x);
	ArrPush(buffer->uv, &y);

	return 0;
}

static int read_normal(
		void *interpreter,
		int scanned_ncomponents,
		double x,
		double y,
		double z,
		double w)
{
	struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;

	ArrPush(buffer->N, &x);
	ArrPush(buffer->N, &y);
	ArrPush(buffer->N, &z);

	return 0;
}

static int read_face(
		void *interpreter,
		long index_count,
		const long *vertex_indices,
		const long *texture_indices,
		const long *normal_indices)
{
	struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;
	int i;

	const int ntriangles = index_count - 2;

	for (i = 0; i < ntriangles; i++) {
		if (vertex_indices != NULL) {
			int indices[3] = {0};
			indices[0] = vertex_indices[0] - 1;
			indices[1] = vertex_indices[i + 1] - 1;
			indices[2] = vertex_indices[i + 2] - 1;
			ArrPush(buffer->vertex_indices, &indices[0]);
			ArrPush(buffer->vertex_indices, &indices[1]);
			ArrPush(buffer->vertex_indices, &indices[2]);
		}

		if (texture_indices != NULL) {
			ArrPush(buffer->texture_indices, &texture_indices[0]);
			ArrPush(buffer->texture_indices, &texture_indices[i + 1]);
			ArrPush(buffer->texture_indices, &texture_indices[i + 2]);
		}

		if (normal_indices != NULL) {
			ArrPush(buffer->normal_indices, &normal_indices[0]);
			ArrPush(buffer->normal_indices, &normal_indices[i + 1]);
			ArrPush(buffer->normal_indices, &normal_indices[i + 2]);
		}
	}

	buffer->nfaces += ntriangles;
	return 0;
}

int ObjBufferFromFile(struct ObjBuffer *buffer, const char *filename)
{
	struct ObjParser *parser = NULL;
	int err = 0;

	if (buffer == NULL)
		return -1;

	if (filename == NULL)
		return -1;

	parser = ObjParserNew(
			buffer,
			read_vertx,
			read_texture,
			read_normal,
			read_face);

	err = ObjParse(parser, filename);

	if (err) {
		return -1;
	}

	ObjParserFree(parser);
	return 0;
}

int ObjBufferToMesh(const struct ObjBuffer *buffer, struct Mesh *mesh)
{
	double *P = NULL;
	int *vertex_indices = NULL;
	int i;

	if (mesh == NULL)
		return -1;

	if (buffer == NULL)
		return -1;

	/* clear and re-allocate */
	MshClear(mesh);
	MshAllocateVertex(mesh, "P", buffer->nverts);
	MshAllocateVertex(mesh, "N", buffer->nverts);
	if (buffer->uv->nelems > 0) {
		MshAllocateVertex(mesh, "uv", buffer->nverts);
	}
	MshAllocateFace(mesh, "indices", buffer->nfaces);

	/* copy data */
	P = (double *) buffer->P->data;
	vertex_indices = (int *) buffer->vertex_indices->data;

	for (i = 0; i < buffer->nverts; i++) {
		MshSetVertexPosition(mesh, i, &P[3 * i]);
	}
	for (i = 0; i < buffer->nfaces; i++) {
		MshSetFaceVertexIndices(mesh, i, &vertex_indices[3 * i]);
	}

	/* compute data */
	MshComputeNormals(mesh);
	MshComputeBounds(mesh);

	return 0;
}

