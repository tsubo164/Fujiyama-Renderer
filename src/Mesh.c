/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Mesh.h"
#include "Intersection.h"
#include "PrimitiveSet.h"
#include "Triangle.h"
#include "Vector.h"
#include "Ray.h"
#include "Box.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>

static int triangle_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect);
static void triangle_bounds(const void *prim_set, int prim_id, double *bounds);
static void triangleset_bounds(const void *prim_set, double *bounds);
static int triangle_count(const void *prim_set);

struct Mesh {
	int nverts;
	int nfaces;

	double *P;
	double *N;
	float *Cd;
	float *uv;
	int *indices;

	double bounds[6];
};

struct Mesh *MshNew(void)
{
	struct Mesh *mesh = (struct Mesh *) malloc(sizeof(struct Mesh));
	if (mesh == NULL)
		return NULL;

	mesh->nverts = 0;
	mesh->nfaces = 0;
	BOX3_SET(mesh->bounds, 0, 0, 0, 0, 0, 0);

	mesh->P = NULL;
	mesh->N = NULL;
	mesh->Cd = NULL;
	mesh->uv = NULL;
	mesh->indices = NULL;

	return mesh;
}

void MshFree(struct Mesh *mesh)
{
	if (mesh == NULL)
		return;

	free(mesh->P);
	free(mesh->N);
	free(mesh->Cd);
	free(mesh->uv);
	free(mesh->indices);

	free(mesh);
}

void MshClear(struct Mesh *mesh)
{
	if (mesh->P != NULL)
		free(mesh->P);
	if (mesh->N != NULL)
		free(mesh->N);
	if (mesh->Cd != NULL)
		free(mesh->Cd);
	if (mesh->uv != NULL)
		free(mesh->uv);
	if (mesh->indices != NULL)
		free(mesh->indices);

	mesh->nverts = 0;
	mesh->nfaces = 0;
	BOX3_SET(mesh->bounds, 0, 0, 0, 0, 0, 0);

	mesh->P = NULL;
	mesh->N = NULL;
	mesh->Cd = NULL;
	mesh->uv = NULL;
	mesh->indices = NULL;
}

void MshComputeBounds(struct Mesh *mesh)
{
	int i;
	BOX3_SET(mesh->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; i < mesh->nverts; i++) {
		BoxAddPoint(mesh->bounds, VEC3_NTH(mesh->P,i));
	}
}

void MshComputeNormals(struct Mesh *mesh)
{
	const int nverts = MshGetVertexCount(mesh);
	const int nfaces = MshGetFaceCount(mesh);
	double *P = (double *) mesh->P;
	double *N = (double *) mesh->N;
	int *indices = (int *) mesh->indices;
	int i;

	if (P == NULL || indices == NULL)
		return;

	if (N == NULL) {
		MshAllocateVertex(mesh, "N", nverts);
		return;
	}

	/* initialize N */
	for (i = 0; i < nverts; i++) {
		double *nml = &N[3*i];
		VEC3_SET(nml, 0, 0, 0);
	}

	/* compute N */
	for (i = 0; i < nfaces; i++) {
		double *P0, *P1, *P2;
		double *N0, *N1, *N2;
		double Ng[3] = {0, 0, 0};
		const int i0 = indices[3*i + 0];
		const int i1 = indices[3*i + 1];
		const int i2 = indices[3*i + 2];

		P0 = &P[3*i0];
		P1 = &P[3*i1];
		P2 = &P[3*i2];
		N0 = &N[3*i0];
		N1 = &N[3*i1];
		N2 = &N[3*i2];

		TriComputeFaceNormal(Ng, P0, P1, P2);
		VEC3_ADD_ASGN(N0, Ng);
		VEC3_ADD_ASGN(N1, Ng);
		VEC3_ADD_ASGN(N2, Ng);
	}

	/* normalize N */
	for (i = 0; i < nverts; i++) {
		double *nml = &N[3*i];
		VEC3_NORMALIZE(nml);
	}
}

void *MshAllocateVertex(struct Mesh *mesh, const char *attr_name, int nverts)
{
	void *ret = NULL;

	if (strcmp(attr_name, "P") == 0) {
		mesh->P = VEC3_REALLOC(mesh->P, double, nverts);
		ret = mesh->P;
	}
	else if (strcmp(attr_name, "N") == 0) {
		mesh->N = VEC3_REALLOC(mesh->N, double, nverts);
		ret = mesh->N;
	}
	else if (strcmp(attr_name, "uv") == 0) {
		mesh->uv = VEC2_REALLOC(mesh->uv, float, nverts);
		ret = mesh->uv;
	}

	mesh->nverts = nverts;
	return ret;
}

void *MshAllocateFace(struct Mesh *mesh, const char *attr_name, int nfaces)
{
	void *ret = NULL;

	if (strcmp(attr_name, "indices") == 0) {
		mesh->indices = VEC3_REALLOC(mesh->indices, int, nfaces);
		ret = mesh->indices;
	}

	mesh->nfaces = nfaces;
	return ret;
}

void MshGetFaceVertexPosition(const struct Mesh *mesh, int face_index,
		double *P0, double *P1, double *P2)
{
	int i0, i1, i2;
	double *p0, *p1, *p2;

	i0 = VEC3_NTH(mesh->indices, face_index)[0];
	i1 = VEC3_NTH(mesh->indices, face_index)[1];
	i2 = VEC3_NTH(mesh->indices, face_index)[2];
	p0 = VEC3_NTH(mesh->P, i0);
	p1 = VEC3_NTH(mesh->P, i1);
	p2 = VEC3_NTH(mesh->P, i2);

	VEC3_COPY(P0, p0);
	VEC3_COPY(P1, p1);
	VEC3_COPY(P2, p2);
}

void MshGetFaceVertexNormal(const struct Mesh *mesh, int face_index,
		double *N0, double *N1, double *N2)
{
	int i0, i1, i2;
	double *n0, *n1, *n2;

	i0 = VEC3_NTH(mesh->indices, face_index)[0];
	i1 = VEC3_NTH(mesh->indices, face_index)[1];
	i2 = VEC3_NTH(mesh->indices, face_index)[2];
	n0 = VEC3_NTH(mesh->N, i0);
	n1 = VEC3_NTH(mesh->N, i1);
	n2 = VEC3_NTH(mesh->N, i2);

	VEC3_COPY(N0, n0);
	VEC3_COPY(N1, n1);
	VEC3_COPY(N2, n2);
}

void MshSetVertexPosition(struct Mesh *mesh, int index, const double *P)
{
	if (mesh->P == NULL)
		return;
	if (index < 0 || index >= mesh->nverts)
		return;

	mesh->P[3*index + 0] = P[0];
	mesh->P[3*index + 1] = P[1];
	mesh->P[3*index + 2] = P[2];
}

void MshSetVertexNormal(struct Mesh *mesh, int index, const double *N)
{
	if (mesh->N == NULL)
		return;
	if (index < 0 || index >= mesh->nverts)
		return;

	mesh->N[3*index + 0] = N[0];
	mesh->N[3*index + 1] = N[1];
	mesh->N[3*index + 2] = N[2];
}

void MshSetVertexTexture(struct Mesh *mesh, int index, const float *uv)
{
	if (mesh->uv == NULL)
		return;
	if (index < 0 || index >= mesh->nverts)
		return;

	mesh->uv[2*index + 0] = uv[0];
	mesh->uv[2*index + 1] = uv[1];
}

void MshSetFaceVertexIndices(struct Mesh *mesh, int index, const int *indices)
{
	if (mesh->indices == NULL)
		return;
	if (index < 0 || index >= mesh->nfaces)
		return;

	mesh->indices[3*index + 0] = indices[0];
	mesh->indices[3*index + 1] = indices[1];
	mesh->indices[3*index + 2] = indices[2];
}

int MshGetVertexCount(const struct Mesh *mesh)
{
	return mesh->nverts;
}

int MshGetFaceCount(const struct Mesh *mesh)
{
	return mesh->nfaces;
}

void MshGetPrimitiveSet(const struct Mesh *mesh, struct PrimitiveSet *primset)
{
	MakePrimitiveSet(primset,
			"Mesh",
			mesh,
			triangle_ray_intersect,
			triangle_bounds,
			triangleset_bounds,
			triangle_count);
}

static int triangle_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	const double *P0, *P1, *P2;
	const double *N0, *N1, *N2;
	int i0, i1, i2;
	double u, v;
	double t_hit;
	int hit;

	/* TODO make function */
	i0 = VEC3_NTH(mesh->indices, prim_id)[0];
	i1 = VEC3_NTH(mesh->indices, prim_id)[1];
	i2 = VEC3_NTH(mesh->indices, prim_id)[2];
	P0 = VEC3_NTH(mesh->P, i0);
	P1 = VEC3_NTH(mesh->P, i1);
	P2 = VEC3_NTH(mesh->P, i2);

	hit = TriRayIntersect(
			P0, P1, P2,
			ray->orig, ray->dir, DO_NOT_CULL_BACKFACES,
			&t_hit, &u, &v);

	if (!hit)
		return 0;

	if (isect == NULL)
		return 1;

	/* intersect info */
	N0 = VEC3_NTH(mesh->N, i0);
	N1 = VEC3_NTH(mesh->N, i1);
	N2 = VEC3_NTH(mesh->N, i2);
	TriComputeNormal(isect->N, N0, N1, N2, u, v);

	/* TODO TMP uv handling */
	/* UV = (1-u-v) * UV0 + u * UV1 + v * UV2 */
	if (mesh->uv != NULL) {
		const float *uv0, *uv1, *uv2;
		const float t = 1-u-v;
		uv0 = VEC2_NTH(mesh->uv, i0);
		uv1 = VEC2_NTH(mesh->uv, i1);
		uv2 = VEC2_NTH(mesh->uv, i2);
		isect->uv[0] = t * uv0[0] + u * uv1[0] + v * uv2[0];
		isect->uv[1] = t * uv0[1] + u * uv1[1] + v * uv2[1];
	}
	else {
		isect->uv[0] = 0;
		isect->uv[1] = 0;
	}

	POINT_ON_RAY(isect->P, ray->orig, ray->dir, t_hit);
	isect->object = NULL;
	isect->prim_id = prim_id;
	isect->t_hit = t_hit;

	return 1;
}

static void triangle_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	const double *p0, *p1, *p2;
	int i0, i1, i2;

	i0 = VEC3_NTH(mesh->indices, prim_id)[0];
	i1 = VEC3_NTH(mesh->indices, prim_id)[1];
	i2 = VEC3_NTH(mesh->indices, prim_id)[2];
	p0 = VEC3_NTH(mesh->P, i0);
	p1 = VEC3_NTH(mesh->P, i1);
	p2 = VEC3_NTH(mesh->P, i2);

	TriComputeBounds(bounds, p0, p1, p2);
}

static void triangleset_bounds(const void *prim_set, double *bounds)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	BOX3_COPY(bounds, mesh->bounds);
}

static int triangle_count(const void *prim_set)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	return mesh->nfaces;
}

