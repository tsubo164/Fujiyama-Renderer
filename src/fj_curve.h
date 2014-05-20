/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_CURVE_H
#define FJ_CURVE_H

#include "fj_compatibility.h"
#include "fj_tex_coord.h"
#include "fj_color.h"
#include "fj_types.h"
#include "fj_box.h"
#include <vector>

namespace fj {

struct PrimitiveSet;

class FJ_API Curve {
public:
  Curve();
  ~Curve();

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
  double   GetVertexWidth(int idx) const;
  int      GetCurveIndices(int idx) const;

  void SetVertexPosition(int idx, const Vector &value);
  void SetVertexColor(int idx, const Color &value);
  void SetVertexTexture(int idx, const TexCoord &value);
  void SetVertexVelocity(int idx, const Vector &value);
  void SetVertexWidth(int idx, const double &value);
  void SetCurveIndices(int idx, const int &value);

  bool HasVertexPosition() const;
  bool HasVertexColor() const;
  bool HasVertexTexture() const;
  bool HasVertexVelocity() const;
  bool HasVertexWidth() const;
  bool HasCurveIndices() const;

  void ComputeBounds();

public:
  int nverts;
  int ncurves;

  std::vector<Vector>   P;
  std::vector<Color>    Cd;
  std::vector<TexCoord> uv;
  std::vector<Vector>   velocity;
  std::vector<double>   width;
  std::vector<int>      indices;

  std::vector<int> split_depth_;

  Box bounds;

  void cache_split_depth();
};

FJ_API Curve *CrvNew(void);
FJ_API void CrvFree(Curve *curve);

FJ_API void CrvAllocateVertex(Curve *curve, const char *attr_name, int nverts);
FJ_API void CrvAllocateCurve(Curve *curve, const char *attr_name, int ncurves);

FJ_API void CrvAddVelocity(Curve *curve);

FJ_API void CrvComputeBounds(Curve *curve);
FJ_API void CrvGetPrimitiveSet(const Curve *curve, PrimitiveSet *primset);

} // namespace xxx

#endif /* FJ_XXX_H */
