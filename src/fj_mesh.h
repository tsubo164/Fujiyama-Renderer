// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_MESH_H
#define FJ_MESH_H

#include "fj_compatibility.h"
#include "fj_vertex_attribute.h"
#include "fj_primitive_set.h"
#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"
#include "fj_box.h"

#include <vector>
#include <string>
#include <map>

namespace fj {

class FJ_API Mesh : public PrimitiveSet {
public:
  Mesh();
  virtual ~Mesh();

  int GetPointCount() const;
  int GetFaceCount() const;
  void SetPointCount(int count);
  void SetFaceCount(int count);
  const Box &GetBounds() const;

  //TODO TEST
  bool HasVertexNormal() const;
  Vector GetVertexNormal(Index vertex_id) const;

  template <typename T>
  class VertexAttributeAccessor {
  public:
    typedef T Value;

  public:
    VertexAttributeAccessor(VertexAttribute<T> &attribute) : attr_(attribute) {}
    ~VertexAttributeAccessor() {}

    // Resize
    void ResizeValue(Index size)
    {
      attr_.ResizeValue(size);
    }
    void ResizeIndex(Index size)
    {
      attr_.ResizeIndex(size);
    }

    // Push
    Index PushValue(const Value &value)
    {
      attr_.PushValue(value);
      return attr_.GetValueCount() - 1;
    }
    Index PushIndex(Index index)
    {
      attr_.PushIndex(index);
      return attr_.GetIndexCount() - 1;
    }

    // Set
    void SetValue(Index offset, const Value &value)
    {
      attr_.SetValue(offset, value);
    }
    void SetIndex(Index offset, const Index index)
    {
      attr_.SetIndex(offset, index);
    }

    // Get
    Value GetValue(Index offset) const
    {
      return attr_.GetValue(offset);
    }
    Index GetIndex(Index offset) const
    {
      return attr_.GetIndex(offset);
    }

    // Count
    Index GetValueCount() const
    {
      return attr_.GetValueCount();
    }
    Index GetIndexCount() const
    {
      return attr_.GetIndexCount();
    }

    Value Get(Index vertex_id) const { return attr_.Get(vertex_id); }

  private:
    VertexAttribute<T> &attr_;
  };

  VertexAttributeAccessor<Vector> GetVertexNormal()
  {
    return VertexAttributeAccessor<Vector>(vertex_normal_);
  }

  template <typename T>
  class PointAttributeAccessor {
  public:
    typedef T Value;

  public:
    PointAttributeAccessor(std::vector<Value> &value, std::vector<Index> &index)
        : value_(value), index_(index) {}
    ~PointAttributeAccessor() {}

/*
    Index Push(const Value &value);
    Value Get(Index point_id) const;

    Value GetValue(Index offset) const;
    Index GetIndex(Index offset) const;
    Index GetValueCount() const;
    Index GetIndexCount() const;
*/
  private:
    std::vector<Value> &value_;
    std::vector<Index> &index_;
  };
#if 0
#endif

#if n
  void CreateVertexNormal(Index value_count, Index index_count);
  void SetVertexNormalValue(Index offset, const Vector &value);
  void SetVertexNormalIndex(Index offset, Index index);
  Vector GetVertexNormalValue(Index offset) const;
  Index  GetVertexNormalIndex(Index offset) const;
  Index GetVertexNormalValueCount() const;
  Index GetVertexNormalIndexCount() const;
#else
  Index  PushVertexNormalValue(const Vector &value);
  Index  PushVertexNormalIndex(Index index);
  Vector GetVertexNormalValue(Index offset) const;
  Index  GetVertexNormalIndex(Index offset) const;
  Index  GetVertexNormalValueCount() const;
  Index  GetVertexNormalIndexCount() const;
  void ShrinkToFit();
#endif

  void AddPointPosition();
  void AddPointNormal();
  void AddPointColor();
  void AddPointTexture();
  void AddPointVelocity();
  void AddFaceIndices();
  void AddFaceGroupID();

  Vector   GetPointPosition(int idx) const;
  Vector   GetPointNormal(int idx) const;
  Color    GetPointColor(int idx) const;
  TexCoord GetPointTexture(int idx) const;
  Vector   GetPointVelocity(int idx) const;
  Index3   GetFaceIndices(int idx) const;
  int      GetFaceGroupID(int idx) const;

  void SetPointPosition(int idx, const Vector &value);
  void SetPointNormal(int idx, const Vector &value);
  void SetPointColor(int idx, const Color &value);
  void SetPointTexture(int idx, const TexCoord &value);
  void SetPointVelocity(int idx, const Vector &value);
  void SetFaceIndices(int idx, const Index3 &value);
  void SetFaceGroupID(int idx, const int &value);

  bool HasPointPosition() const;
  bool HasPointNormal() const;
  bool HasPointColor() const;
  bool HasPointTexture() const;
  bool HasPointVelocity() const;
  bool HasFaceIndices() const;
  bool HasFaceGroupID() const;

  int CreateFaceGroup(const std::string &group_name);
  int LookupFaceGroup(const std::string &group_name) const;

  void ComputeNormals();
  void ComputeBounds();
  void Clear();

private:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const;
  virtual bool box_intersect(Index prim_id, const Box &box) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  int point_count_;
  int face_count_;

  //TODO TEST
  VertexAttribute<Vector> vertex_normal_;

  std::vector<Vector>   P_;
  std::vector<Vector>   N_;
  std::vector<Color>    Cd_;
  std::vector<TexCoord> uv_;
  std::vector<Vector>   velocity_;
  std::vector<Index3>   indices_;
  std::vector<int>      face_group_id_;

  std::map<std::string, int> face_group_name_;

  Box bounds_;
};

FJ_API void MshGetFacePointPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2);
FJ_API void MshGetFacePointNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2);

} // namespace xxx

#endif // FJ_XXX_H
