/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Mesh.h"
#include "LocalGeometry.h"
#include "Accelerator.h"
#include "Triangle.h"
#include "Ray.h"
#include "Box.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>

static void *allocate_P(struct Mesh *mesh, int nverts);
static void *allocate_N(struct Mesh *mesh, int nverts);
static void *allocate_uv(struct Mesh *mesh, int nverts);
static void *allocate_indices(struct Mesh *mesh, int nfaces);

static int triangle_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void triangle_bounds(const void *prim_set, int prim_id, double *bounds);

struct Mesh *MshNew(void)
{
	struct Mesh *mesh;

	mesh = (struct Mesh *) malloc(sizeof(struct Mesh));
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

	if (mesh->P != NULL)
		free(mesh->P);
	if (mesh->N != NULL)
		free(mesh->N);
	if (mesh->Cd != NULL)
		free(mesh->Cd);
	if (mesh->uv != NULL)
		free(mesh->uv);

	free(mesh);
}

void MshComputeBounds(struct Mesh *mesh)
{
	int i;
	BOX3_SET(mesh->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; i < mesh->nverts; i++) {
		BoxAddPoint(mesh->bounds, &mesh->P[3*i]);
	}
}

void *MshAllocateVertex(struct Mesh *mesh, const char *attr_name, int nverts)
{
	void *ret = NULL;

	if (strcmp(attr_name, "P") == 0) {
		ret = allocate_P(mesh, nverts);
	}
	else if (strcmp(attr_name, "N") == 0) {
		ret = allocate_N(mesh, nverts);
	}
	else if (strcmp(attr_name, "uv") == 0) {
		ret = allocate_uv(mesh, nverts);
	}

	return ret;
}

void *MshAllocateFace(struct Mesh *mesh, const char *attr_name, int nfaces)
{
	void *ret = NULL;

	if (strcmp(attr_name, "indices") == 0) {
		ret = allocate_indices(mesh, nfaces);
	}

	return ret;
}

void MshGetFaceVertex(const struct Mesh *mesh, int face_index,
		const double **v0, const double **v1, const double **v2)
{
	int i0, i1, i2;

	i0 = mesh->indices[3*face_index + 0];
	i1 = mesh->indices[3*face_index + 1];
	i2 = mesh->indices[3*face_index + 2];
	*v0 = &mesh->P[3*i0];
	*v1 = &mesh->P[3*i1];
	*v2 = &mesh->P[3*i2];
}

void MshSetupAccelerator(const struct Mesh *mesh, struct Accelerator *acc)
{
	AccSetTargetGeometry(acc,
			mesh,
			mesh->nfaces,
			mesh->bounds,
			triangle_ray_intersect,
			triangle_bounds);
}

static void *allocate_P(struct Mesh *mesh, int nverts)
{
	mesh->P = (double *) realloc(mesh->P, 3 * sizeof(double) * nverts);
	if (mesh->P == NULL) {
		mesh->nverts = 0;
		return NULL;
	}

	mesh->nverts = nverts;
	return mesh->P;
}

static void *allocate_N(struct Mesh *mesh, int nverts)
{
	mesh->N = (double *) realloc(mesh->N, 3 * sizeof(double) * nverts);
	if (mesh->N == NULL) {
		mesh->nverts = 0;
		return NULL;
	}

	mesh->nverts = nverts;
	return mesh->N;
}

static void *allocate_uv(struct Mesh *mesh, int nverts)
{
	mesh->uv = (float *) realloc(mesh->uv, 2 * sizeof(float) * nverts);
	if (mesh->uv == NULL) {
		mesh->nverts = 0;
		return NULL;
	}

	mesh->nverts = nverts;
	return mesh->uv;
}

static void *allocate_indices(struct Mesh *mesh, int nfaces)
{
	mesh->indices = (int *) realloc(mesh->indices, 3 * sizeof(int) * nfaces);
	if (mesh->indices == NULL) {
		mesh->nfaces = 0;
		return NULL;
	}

	mesh->nfaces = nfaces;
	return mesh->indices;
}

static int triangle_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	const double *v0, *v1, *v2;
	const double *N0, *N1, *N2;
	int i0, i1, i2;
	double u, v;
	int hit;

	/* TODO make function */
	i0 = mesh->indices[3*prim_id + 0];
	i1 = mesh->indices[3*prim_id + 1];
	i2 = mesh->indices[3*prim_id + 2];
	v0 = &mesh->P[3*i0];
	v1 = &mesh->P[3*i1];
	v2 = &mesh->P[3*i2];

	hit = TriRayIntersect(
			v0, v1, v2,
			ray->orig, ray->dir, DO_NOT_CULL_BACKFACES,
			t_hit, &u, &v);

	if (!hit)
		return 0;

	if (isect == NULL)
		return 1;

	/* intersect info */
	/*
	TriComputeFaceNormal(isect->N, v0, v1, v2);
	*/
	N0 = &mesh->N[3*i0];
	N1 = &mesh->N[3*i1];
	N2 = &mesh->N[3*i2];
	TriComputeNormal(isect->N, N0, N1, N2, u, v);

	/* TODO TMP uv handling */
	/* N = (1-u-v) * N0 + u * N1 + v * N2 */
	if (mesh->uv != NULL) {
		const float *uv0, *uv1, *uv2;
		double t = 1-u-v;
		uv0 = &mesh->uv[2*i0];
		uv1 = &mesh->uv[2*i1];
		uv2 = &mesh->uv[2*i2];
		isect->uv[0] = t * uv0[0] + u * uv1[0] + v * uv2[0];
		isect->uv[1] = t * uv0[1] + u * uv1[1] + v * uv2[1];
	}
	else {
		isect->uv[0] = 0;
		isect->uv[1] = 0;
	}

	POINT_ON_RAY(isect->P, ray->orig, ray->dir, *t_hit);
	isect->object = NULL;
	isect->geometry = mesh;
	isect->prim_id = prim_id;

	return 1;
}

static void triangle_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	const double *v0, *v1, *v2;

	MshGetFaceVertex(mesh, prim_id, &v0, &v1, &v2);
	TriComputeBounds(bounds, v0, v1, v2);
}

