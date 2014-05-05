/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MESH_H
#define FJ_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int Index;

struct TriIndex {
  Index i0, i1, i2;
};

struct PrimitiveSet;
struct TexCoord;
struct Vector;
struct Color;

extern struct Mesh *MshNew(void);
extern void MshFree(struct Mesh *mesh);

extern void MshClear(struct Mesh *mesh);

/* allocations */
extern void *MshAllocateVertex(struct Mesh *mesh, const char *attr_name, int nverts);
extern void *MshAllocateFace(struct Mesh *mesh, const char *attr_name, int nfaces);

/* properties */
extern int MshGetVertexCount(const struct Mesh *mesh);
extern int MshGetFaceCount(const struct Mesh *mesh);

extern void MshGetFaceVertexPosition(const struct Mesh *mesh, int face_index,
    struct Vector *P0, struct Vector *P1, struct Vector *P2);
extern void MshGetFaceVertexNormal(const struct Mesh *mesh, int face_index,
    struct Vector *N0, struct Vector *N1, struct Vector *N2);

/* property setting */
extern void MshSetVertexPosition(struct Mesh *mesh, int index, const struct Vector *P);
extern void MshSetVertexNormal(struct Mesh *mesh, int index, const struct Vector *N);
extern void MshSetVertexColor(struct Mesh *mesh, int index, const struct Color *Cd);
extern void MshSetVertexTexture(struct Mesh *mesh, int index, const struct TexCoord *uv);
extern void MshSetVertexVelocity(struct Mesh *mesh, int index, const struct Vector *velocity);
extern void MshSetFaceVertexIndices(struct Mesh *mesh, int face_index,
    const struct TriIndex *tri_index);

extern void MshGetVertexPosition(const struct Mesh *mesh, int index, struct Vector *P);
extern void MshGetVertexNormal(const struct Mesh *mesh, int index, struct Vector *N);
extern void MshGetFaceVertexIndices(const struct Mesh *mesh, int face_index,
    struct TriIndex *tri_index);

/* re-computation */
extern void MshComputeBounds(struct Mesh *mesh);
extern void MshComputeNormals(struct Mesh *mesh);

extern void MshGetPrimitiveSet(const struct Mesh *mesh, struct PrimitiveSet *primset);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
