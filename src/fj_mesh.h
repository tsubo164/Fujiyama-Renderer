/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MESH_H
#define FJ_MESH_H

#include "fj_vector.h"
#include "fj_color.h"
#include "fj_box.h"
#include <vector>

namespace fj {

typedef int Index;

struct TriIndex {
  Index i0, i1, i2;
};

struct PrimitiveSet;
struct TexCoord;
struct Vector;
struct Color;

struct Mesh;

extern Mesh *MshNew(void);
extern void MshFree(Mesh *mesh);

extern void MshClear(Mesh *mesh);

/* allocations */
extern void MshAllocateVertex(Mesh *mesh, const char *attr_name, int nverts);
extern void MshAllocateFace(Mesh *mesh, const char *attr_name, int nfaces);

/* properties */
extern int MshGetVertexCount(const Mesh *mesh);
extern int MshGetFaceCount(const Mesh *mesh);

extern void MshGetFaceVertexPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2);
extern void MshGetFaceVertexNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2);

/* property setting */
extern void MshSetVertexPosition(Mesh *mesh, int index, const Vector *P);
extern void MshSetVertexNormal(Mesh *mesh, int index, const Vector *N);
extern void MshSetVertexColor(Mesh *mesh, int index, const Color *Cd);
extern void MshSetVertexTexture(Mesh *mesh, int index, const TexCoord *uv);
extern void MshSetVertexVelocity(Mesh *mesh, int index, const Vector *velocity);
extern void MshSetFaceVertexIndices(Mesh *mesh, int face_index,
    const TriIndex *tri_index);

extern void MshGetVertexPosition(const Mesh *mesh, int index, Vector *P);
extern void MshGetVertexNormal(const Mesh *mesh, int index, Vector *N);
extern void MshGetFaceVertexIndices(const Mesh *mesh, int face_index,
    TriIndex *tri_index);

/* re-computation */
extern void MshComputeBounds(Mesh *mesh);
extern void MshComputeNormals(Mesh *mesh);

extern void MshGetPrimitiveSet(const Mesh *mesh, PrimitiveSet *primset);

struct Mesh {
public:
  Mesh();
  ~Mesh();

  int GetVertexCount() const;
  int GetFaceCount() const;
  void SetVertexCount(int count);
  void SetFaceCount(int count);
  const Box &GetBounds() const;

  void AddVertexPosition();
  void AddVertexNormal();
  void AddVertexColor();
  void AddVertexTexture();
  void AddVertexVelocity();
  void AddFaceIndices();

  Vector   GetVertexPosition(int idx) const;
  Vector   GetVertexNormal(int idx) const;
  Color    GetVertexColor(int idx) const;
  TexCoord GetVertexTexture(int idx) const;
  Vector   GetVertexVelocity(int idx) const;
  TriIndex GetFaceIndices(int idx) const;

  void SetVertexPosition(int idx, const Vector &value);
  void SetVertexNormal(int idx, const Vector &value);
  void SetVertexColor(int idx, const Color &value);
  void SetVertexTexture(int idx, const TexCoord &value);
  void SetVertexVelocity(int idx, const Vector &value);
  void SetFaceIndices(int idx, const TriIndex &value);

  bool HasVertexPosition() const;
  bool HasVertexNormal() const;
  bool HasVertexColor() const;
  bool HasVertexTexture() const;
  bool HasVertexVelocity() const;
  bool HasFaceIndices() const;

  void ComputeNormals();
  void ComputeBounds();
  void Clear();

private:
  int nverts;
  int nfaces;

  std::vector<Vector>   P;
  std::vector<Vector>   N;
  std::vector<Color>    Cd;
  std::vector<TexCoord> uv;
  std::vector<Vector>   velocity;
  std::vector<TriIndex> indices;

  Box bounds;
};

} // namespace xxx

#endif /* FJ_XXX_H */
