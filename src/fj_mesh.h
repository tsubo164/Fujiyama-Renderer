/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MESH_H
#define FJ_MESH_H

#include "fj_compatibility.h"
#include "fj_primitive_set.h"
#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

class FJ_API Mesh : public PrimitiveSet {
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
  Index3 GetFaceIndices(int idx) const;

  void SetVertexPosition(int idx, const Vector &value);
  void SetVertexNormal(int idx, const Vector &value);
  void SetVertexColor(int idx, const Color &value);
  void SetVertexTexture(int idx, const TexCoord &value);
  void SetVertexVelocity(int idx, const Vector &value);
  void SetFaceIndices(int idx, const Index3 &value);

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
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  int nverts;
  int nfaces;

  std::vector<Vector>   P;
  std::vector<Vector>   N;
  std::vector<Color>    Cd;
  std::vector<TexCoord> uv;
  std::vector<Vector>   velocity;
  std::vector<Index3> indices;

  Box bounds;
};

extern Mesh *MshNew(void);
extern void MshFree(Mesh *mesh);

extern void MshGetFaceVertexPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2);
extern void MshGetFaceVertexNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2);

} // namespace xxx

#endif /* FJ_XXX_H */
