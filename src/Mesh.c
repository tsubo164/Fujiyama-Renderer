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

struct Mesh *MshNew(void)
{
	struct Mesh *mesh;

	mesh = (struct Mesh *) malloc(sizeof(struct Mesh));
	if (mesh == NULL)
		return NULL;

	mesh->nverts = 0;
	mesh->nfaces = 0;
	BOX3_SET(mesh->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

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

void MshComputeBounds(struct Mesh *mesh)
{
	int i;
	BOX3_SET(mesh->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; i < mesh->nverts; i++) {
		BoxAddPoint(mesh->bounds, VEC3_NTH(mesh->P,i));
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

void MshGetFaceVertex(const struct Mesh *mesh, int face_index,
		const double **v0, const double **v1, const double **v2)
{
	int i0, i1, i2;

	i0 = VEC3_NTH(mesh->indices, face_index)[0];
	i1 = VEC3_NTH(mesh->indices, face_index)[1];
	i2 = VEC3_NTH(mesh->indices, face_index)[2];
	*v0 = VEC3_NTH(mesh->P, i0);
	*v1 = VEC3_NTH(mesh->P, i1);
	*v2 = VEC3_NTH(mesh->P, i2);
}

void MshGetPrimitiveSet(const struct Mesh *mesh, struct PrimitiveSet *primset)
{
	MakePrimitiveSet(primset,
			"Mesh",
			mesh,
			mesh->nfaces,
			mesh->bounds,
			triangle_ray_intersect,
			triangle_bounds);
}

static int triangle_ray_intersect(const void *prim_set, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const struct Mesh *mesh = (const struct Mesh *) prim_set;
	const double *v0, *v1, *v2;
	const double *N0, *N1, *N2;
	int i0, i1, i2;
	double u, v;
	double t_hit;
	int hit;

	/* TODO make function */
	i0 = VEC3_NTH(mesh->indices, prim_id)[0];
	i1 = VEC3_NTH(mesh->indices, prim_id)[1];
	i2 = VEC3_NTH(mesh->indices, prim_id)[2];
	v0 = VEC3_NTH(mesh->P, i0);
	v1 = VEC3_NTH(mesh->P, i1);
	v2 = VEC3_NTH(mesh->P, i2);

	hit = TriRayIntersect(
			v0, v1, v2,
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
		const double t = 1-u-v;
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
	const double *v0, *v1, *v2;

	MshGetFaceVertex(mesh, prim_id, &v0, &v1, &v2);
	TriComputeBounds(bounds, v0, v1, v2);
}

