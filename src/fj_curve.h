// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_CURVE_H
#define FJ_CURVE_H

#include "fj_compatibility.h"
#include "fj_primitive_set.h"
#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

class FJ_API Curve : public PrimitiveSet {
public:
  Curve();
  virtual ~Curve();

  int GetVertexCount() const;
  int GetCurveCount() const;
  void SetVertexCount(int count);
  void SetCurveCount(int count);
  const Box &GetBounds() const;

  void AddVertexPosition();
  void AddVertexColor();
  void AddVertexTexture();
  void AddVertexVelocity();
  void AddVertexWidth();
  void AddCurveIndices();

  Vector   GetVertexPosition(int idx) const;
  Color    GetVertexColor(int idx) const;
  TexCoord GetVertexTexture(int idx) const;
  Vector   GetVertexVelocity(int idx) const;
  Real     GetVertexWidth(int idx) const;
  int      GetCurveIndices(int idx) const;

  void SetVertexPosition(int idx, const Vector &value);
  void SetVertexColor(int idx, const Color &value);
  void SetVertexTexture(int idx, const TexCoord &value);
  void SetVertexVelocity(int idx, const Vector &value);
  void SetVertexWidth(int idx, const Real &value);
  void SetCurveIndices(int idx, const int &value);

  bool HasVertexPosition() const;
  bool HasVertexColor() const;
  bool HasVertexTexture() const;
  bool HasVertexVelocity() const;
  bool HasVertexWidth() const;
  bool HasCurveIndices() const;

  void ComputeBounds();

private:
  virtual bool ray_intersect(Index prim_id, Real time,
      const Ray &ray, Intersection *isect) const;
  virtual void get_primitive_bounds(Index prim_id, Box *bounds) const;
  virtual void get_bounds(Box *bounds) const;
  virtual Index get_primitive_count() const;

  int nverts_;
  int ncurves_;

  std::vector<Vector>   P_;
  std::vector<Color>    Cd_;
  std::vector<TexCoord> uv_;
  std::vector<Vector>   velocity_;
  std::vector<Real>     width_;
  std::vector<int>      indices_;

  Box bounds_;

  std::vector<int> split_depth_;

  void cache_split_depth();
};

FJ_API Curve *CrvNew(void);
FJ_API void CrvFree(Curve *curve);

} // namespace xxx

#endif /* FJ_XXX_H */
