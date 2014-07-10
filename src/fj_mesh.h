// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

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
  virtual ~Mesh();

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
  Index3   GetFaceIndices(int idx) const;

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

#if 0
  int AddFaceGroup(const std::string &group_name);
  int SetFaceGroup(int idx, int group_id) const;
#endif

  void ComputeNormals();
  void ComputeBounds();
  void Clear();

private:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  int nverts_;
  int nfaces_;

  std::vector<Vector>   P_;
  std::vector<Vector>   N_;
  std::vector<Color>    Cd_;
  std::vector<TexCoord> uv_;
  std::vector<Vector>   velocity_;
  std::vector<Index3>   indices_;

  Box bounds_;
};

FJ_API void MshGetFaceVertexPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2);
FJ_API void MshGetFaceVertexNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2);

} // namespace xxx

#endif // FJ_XXX_H
