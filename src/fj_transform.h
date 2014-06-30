// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TRANSFORM_H
#define FJ_TRANSFORM_H

#include "fj_compatibility.h"
#include "fj_property.h"
#include "fj_matrix.h"
#include "fj_vector.h"
#include "fj_types.h"

namespace fj {

enum TransformOrder {
  ORDER_SRT = 0,
  ORDER_STR,
  ORDER_RST,
  ORDER_RTS,
  ORDER_TRS,
  ORDER_TSR,
  ORDER_XYZ,
  ORDER_XZY,
  ORDER_YXZ,
  ORDER_YZX,
  ORDER_ZXY,
  ORDER_ZYX
};

extern bool IsTransformOrder(int order);
extern bool IsRotateOrder(int order);

class FJ_API Transform {
public:
  Transform();
  ~Transform();

  void Init();

  void TransformPoint(Vector *point)   const;
  void TransformVector(Vector *vector) const;
  void TransformBounds(Box *bounds)    const;

  void TransformPointInverse(Vector *point)   const;
  void TransformVectorInverse(Vector *vector) const;
  void TransformBoundsInverse(Box *bounds)    const;

  void SetTranslate(Real tx, Real ty, Real tz);
  void SetRotate   (Real rx, Real ry, Real rz);
  void SetScale    (Real sx, Real sy, Real sz);

  void SetTransformOrder(int order);
  void SetRotateOrder   (int order);
  void SetTransform(
      int transform_order, int rotate_order,
      Real tx, Real ty, Real tz,
      Real rx, Real ry, Real rz,
      Real sx, Real sy, Real sz);

public:
  void update_matrix();

  Matrix matrix;
  Matrix inverse;

  int transform_order;
  int rotate_order;

  Vector translate;
  Vector rotate;
  Vector scale;
};

class Box;

extern void XfmReset(Transform *transform);

extern void XfmTransformPoint(const Transform *transform, Vector *point);
extern void XfmTransformVector(const Transform *transform, Vector *vector);
extern void XfmTransformBounds(const Transform *transform, Box *bounds);

extern void XfmTransformPointInverse(const Transform *transform, Vector *point);
extern void XfmTransformVectorInverse(const Transform *transform, Vector *vector);
extern void XfmTransformBoundsInverse(const Transform *transform, Box *bounds);

extern void XfmSetTranslate(Transform *transform, Real tx, Real ty, Real tz);
extern void XfmSetRotate(Transform *transform, Real rx, Real ry, Real rz);
extern void XfmSetScale(Transform *transform, Real sx, Real sy, Real sz);
extern void XfmSetTransformOrder(Transform *transform, int order);
extern void XfmSetRotateOrder(Transform *transform, int order);
extern void XfmSetTransform(Transform *transform,
    int transform_order, int rotate_order,
    Real tx, Real ty, Real tz,
    Real rx, Real ry, Real rz,
    Real sx, Real sy, Real sz);

extern int XfmIsTransformOrder(int order);
extern int XfmIsRotateOrder(int order);

// TransformSampleList
class TransformSampleList {
public:
  TransformSampleList() {}
  ~TransformSampleList() {}

public:
  PropertySampleList translate;
  PropertySampleList rotate;
  PropertySampleList scale;
  int transform_order;
  int rotate_order;
};

extern void XfmInitTransformSampleList(TransformSampleList *list);
extern void XfmLerpTransformSample(const TransformSampleList *list, Real time,
    Transform *transform_interp);

extern void XfmPushTranslateSample(TransformSampleList *list,
    Real tx, Real ty, Real tz, Real time);
extern void XfmPushRotateSample(TransformSampleList *list,
    Real rx, Real ry, Real rz, Real time);
extern void XfmPushScaleSample(TransformSampleList *list,
    Real sx, Real sy, Real sz, Real time);

extern void XfmSetSampleTransformOrder(TransformSampleList *list, int order);
extern void XfmSetSampleRotateOrder(TransformSampleList *list, int order);

} // namespace xxx

#endif // FJ_XXX_H
