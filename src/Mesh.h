/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MESH_H
#define MESH_H

#ifdef __cplusplus
extern "C" {
#endif

struct PrimitiveSet;

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

extern struct Mesh *MshNew(void);
extern void MshFree(struct Mesh *mesh);

extern void MshComputeBounds(struct Mesh *mesh);

extern void *MshAllocateVertex(struct Mesh *mesh, const char *attr_name, int nverts);
extern void *MshAllocateFace(struct Mesh *mesh, const char *attr_name, int nfaces);

extern void MshGetFaceVertex(const struct Mesh *mesh, int face_index,
		const double **v0, const double **v1, const double **v2);

extern void MshGetPrimitiveSet(const struct Mesh *mesh, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

