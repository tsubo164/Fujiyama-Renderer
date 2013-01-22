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

extern struct Mesh *MshNew(void);
extern void MshFree(struct Mesh *mesh);

extern void MshClear(struct Mesh *mesh);

extern void MshComputeBounds(struct Mesh *mesh);

extern void *MshAllocateVertex(struct Mesh *mesh, const char *attr_name, int nverts);
extern void *MshAllocateFace(struct Mesh *mesh, const char *attr_name, int nfaces);

extern void MshGetFaceVertexPosition(const struct Mesh *mesh, int face_index,
		double *P0, double *P1, double *P2);
extern void MshGetFaceVertexNormal(const struct Mesh *mesh, int face_index,
		double *N0, double *N1, double *N2);

extern void MshSetVertexPosition(struct Mesh *mesh, int index, const double *P);
extern void MshSetVertexNormal(struct Mesh *mesh, int index, const double *N);
extern void MshSetVertexTexture(struct Mesh *mesh, int index, const float *uv);
extern void MshSetFaceVertexIndices(struct Mesh *mesh, int index, const int *indices);

extern int MshGetVertexCount(const struct Mesh *mesh);
extern int MshGetFaceCount(const struct Mesh *mesh);

extern void MshGetPrimitiveSet(const struct Mesh *mesh, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

