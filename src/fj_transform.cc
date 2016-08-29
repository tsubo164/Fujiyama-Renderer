// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_transform.h"
#include "fj_matrix.h"
#include "fj_box.h"
#include <assert.h>
#include <float.h>

namespace fj {

static int is_transform_order(int order);
static int is_rotate_order(int order);
static void update_matrix(Transform *transform);
static void make_transform_matrix(
    int transform_order, int rotate_order,
    Real tx, Real ty, Real tz,
    Real rx, Real ry, Real rz,
    Real sx, Real sy, Real sz,
    Matrix *transform);

bool IsTransformOrder(int order)
{
  return order >= 0 && order < 6;
}

bool IsRotateOrder(int order)
{
  return order >= 6 && order < 12;
}

Transform::Transform()
{
  Init();
}

Transform::~Transform()
{
}

void Transform::Init()
{
  MatIdentity(&matrix);
  MatIdentity(&inverse);

  transform_order = ORDER_SRT;
  rotate_order = ORDER_XYZ;

  translate = Vector(0, 0, 0);
  rotate    = Vector(0, 0, 0);
  scale     = Vector(1, 1, 1);
}

void Transform::TransformPoint(Vector *point) const
{
  MatTransformPoint(matrix, point);
}

void Transform::TransformVector(Vector *vector) const
{
  MatTransformVector(matrix, vector);
}

void Transform::TransformBounds(Box *bounds) const
{
  MatTransformBounds(matrix, bounds);
}

void Transform::TransformPointInverse(Vector *point) const
{
  MatTransformPoint(inverse, point);
}

void Transform::TransformVectorInverse(Vector *vector) const
{
  MatTransformVector(inverse, vector);
}

void Transform::TransformBoundsInverse(Box *bounds) const
{
  MatTransformBounds(inverse, bounds);
}

void Transform::SetTranslate(Real tx, Real ty, Real tz)
{
  translate = Vector(tx, ty, tz);
  update_matrix();
}

void Transform::SetRotate(Real rx, Real ry, Real rz)
{
  rotate = Vector(rx, ry, rz);
  update_matrix();
}

void Transform::SetScale(Real sx, Real sy, Real sz)
{
  scale = Vector(sx, sy, sz);
  update_matrix();
}

void Transform::SetTransformOrder(int order)
{
  assert(is_transform_order(order));
  transform_order = order;
  update_matrix();
}

void Transform::SetRotateOrder(int order)
{
  assert(is_rotate_order(order));
  rotate_order = order;
  update_matrix();
}

void Transform::SetTransform(
    int transform_order, int rotate_order,
    Real tx, Real ty, Real tz,
    Real rx, Real ry, Real rz,
    Real sx, Real sy, Real sz)
{
  transform_order = transform_order;
  rotate_order    = rotate_order;
  translate       = Vector(tx, ty, tz);
  rotate          = Vector(rx, ry, rz);
  scale           = Vector(sx, sy, sz);
  update_matrix();
}

void Transform::update_matrix()
{
  make_transform_matrix(transform_order, rotate_order,
      translate.x, translate.y, translate.z,
      rotate.x,    rotate.y,    rotate.z,
      scale.x,     scale.y,     scale.z,
      &matrix);

  MatInverse(&inverse, matrix);
}

void XfmReset(Transform *transform)
{
  MatIdentity(&transform->matrix);
  MatIdentity(&transform->inverse);

  transform->transform_order = ORDER_SRT;
  transform->rotate_order = ORDER_XYZ;

  transform->translate = Vector(0, 0, 0);
  transform->rotate    = Vector(0, 0, 0);
  transform->scale     = Vector(1, 1, 1);
}

void XfmTransformPoint(const Transform *transform, Vector *point)
{
  MatTransformPoint(transform->matrix, point);
}

void XfmTransformVector(const Transform *transform, Vector *vector)
{
  MatTransformVector(transform->matrix, vector);
}

void XfmTransformBounds(const Transform *transform, Box *bounds)
{
  MatTransformBounds(transform->matrix, bounds);
}

void XfmTransformPointInverse(const Transform *transform, Vector *point)
{
  MatTransformPoint(transform->inverse, point);
}

void XfmTransformVectorInverse(const Transform *transform, Vector *vector)
{
  MatTransformVector(transform->inverse, vector);
}

void XfmTransformBoundsInverse(const Transform *transform, Box *bounds)
{
  MatTransformBounds(transform->inverse, bounds);
}

void XfmSetTranslate(Transform *transform, Real tx, Real ty, Real tz)
{
  transform->translate = Vector(tx, ty, tz);
  update_matrix(transform);
}

void XfmSetRotate(Transform *transform, Real rx, Real ry, Real rz)
{
  transform->rotate = Vector(rx, ry, rz);
  update_matrix(transform);
}

void XfmSetScale(Transform *transform, Real sx, Real sy, Real sz)
{
  transform->scale = Vector(sx, sy, sz);
  update_matrix(transform);
}

void XfmSetTransformOrder(Transform *transform, int order)
{
  assert(is_transform_order(order));
  transform->transform_order = order;
  update_matrix(transform);
}

void XfmSetRotateOrder(Transform *transform, int order)
{
  assert(is_rotate_order(order));
  transform->rotate_order = order;
  update_matrix(transform);
}

void XfmSetTransform(Transform *transform,
    int transform_order, int rotate_order,
    Real tx, Real ty, Real tz,
    Real rx, Real ry, Real rz,
    Real sx, Real sy, Real sz)
{
  transform->transform_order = transform_order;
  transform->rotate_order = rotate_order;
  transform->translate = Vector(tx, ty, tz);
  transform->rotate = Vector(rx, ry, rz);
  transform->scale = Vector(sx, sy, sz);
  update_matrix(transform);
}

int XfmIsTransformOrder(int order)
{
  return is_transform_order(order);
}

int XfmIsRotateOrder(int order)
{
  return is_rotate_order(order);
}

void XfmInitTransformSampleList(TransformSampleList *list)
{
  PropInitSampleList(&list->translate);
  PropInitSampleList(&list->rotate);
  PropInitSampleList(&list->scale);

  list->transform_order = ORDER_SRT;
  list->rotate_order = ORDER_ZXY;

  list->scale.samples[0].vector[0] = 1;
  list->scale.samples[0].vector[1] = 1;
  list->scale.samples[0].vector[2] = 1;
  list->scale.samples[0].vector[3] = 1;
}

void XfmPushTranslateSample(TransformSampleList *list,
    Real tx, Real ty, Real tz, Real time)
{
  PropertySample sample;

  sample.vector[0] = tx;
  sample.vector[1] = ty;
  sample.vector[2] = tz;
  sample.time = time;

  PropPushSample(&list->translate, &sample);
}

void XfmPushRotateSample(TransformSampleList *list,
    Real rx, Real ry, Real rz, Real time)
{
  PropertySample sample;

  sample.vector[0] = rx;
  sample.vector[1] = ry;
  sample.vector[2] = rz;
  sample.time = time;

  PropPushSample(&list->rotate, &sample);
}

void XfmPushScaleSample(TransformSampleList *list,
    Real sx, Real sy, Real sz, Real time)
{
  PropertySample sample;

  sample.vector[0] = sx;
  sample.vector[1] = sy;
  sample.vector[2] = sz;
  sample.time = time;

  PropPushSample(&list->scale, &sample);
}

void XfmSetSampleTransformOrder(TransformSampleList *list, int order)
{
  assert(is_transform_order(order));
  list->transform_order = order;
}

void XfmSetSampleRotateOrder(TransformSampleList *list, int order)
{
  assert(is_rotate_order(order));
  list->rotate_order = order;
}

void XfmLerpTransformSample(const TransformSampleList *list, Real time,
    Transform *transform_interp)
{
  PropertySample T;
  PropertySample R;
  PropertySample S;

  PropLerpSamples(&list->translate, time, &T);
  PropLerpSamples(&list->rotate, time, &R);
  PropLerpSamples(&list->scale, time, &S);

  XfmSetTransform(transform_interp,
    list->transform_order, list->rotate_order,
    T.vector[0], T.vector[1], T.vector[2],
    R.vector[0], R.vector[1], R.vector[2],
    S.vector[0], S.vector[1], S.vector[2]);
}

static void update_matrix(Transform *transform)
{
  make_transform_matrix(transform->transform_order, transform->rotate_order,
      transform->translate.x, transform->translate.y, transform->translate.z,
      transform->rotate.x,    transform->rotate.y,    transform->rotate.z,
      transform->scale.x,     transform->scale.y,     transform->scale.z,
      &transform->matrix);

  MatInverse(&transform->inverse, transform->matrix);
}

static void make_transform_matrix(
    int transform_order, int rotate_order,
    Real tx, Real ty, Real tz,
    Real rx, Real ry, Real rz,
    Real sx, Real sy, Real sz,
    Matrix *transform)
{
#define QUEUE_SET(dst,q0,q1,q2) do { \
  (dst)[0] = (q0); \
  (dst)[1] = (q1); \
  (dst)[2] = (q2); \
  } while(0)

  int i;
  Matrix T, R, S, RX, RY, RZ;
  Matrix *queue[3];

  MatTranslate(&T, tx, ty, tz);
  MatRotateX(&RX, rx);
  MatRotateY(&RY, ry);
  MatRotateZ(&RZ, rz);
  MatScale(&S, sx, sy, sz);

  switch (rotate_order) {
  case ORDER_XYZ: QUEUE_SET(queue, &RX, &RY, &RZ); break;
  case ORDER_XZY: QUEUE_SET(queue, &RX, &RZ, &RY); break;
  case ORDER_YXZ: QUEUE_SET(queue, &RY, &RX, &RZ); break;
  case ORDER_YZX: QUEUE_SET(queue, &RY, &RZ, &RX); break;
  case ORDER_ZXY: QUEUE_SET(queue, &RZ, &RX, &RY); break;
  case ORDER_ZYX: QUEUE_SET(queue, &RZ, &RY, &RX); break;
  default:
    assert(!"invalid rotate order");
    break;
  }

  MatIdentity(&R);
  for (i = 0; i < 3; i++)
    MatMultiply(&R, *queue[i], R);

  switch (transform_order) {
  case ORDER_SRT: QUEUE_SET(queue, &S, &R, &T); break;
  case ORDER_STR: QUEUE_SET(queue, &S, &T, &R); break;
  case ORDER_RST: QUEUE_SET(queue, &R, &S, &T); break;
  case ORDER_RTS: QUEUE_SET(queue, &R, &T, &S); break;
  case ORDER_TRS: QUEUE_SET(queue, &T, &R, &S); break;
  case ORDER_TSR: QUEUE_SET(queue, &T, &S, &R); break;
  default:
    assert(!"invalid transform order order");
    break;
  }

  MatIdentity(transform);
  for (i = 0; i < 3; i++)
    MatMultiply(transform, *queue[i], *transform);

#undef QUEUE_SET
}

static int is_transform_order(int order)
{
  return order >= 0 && order < 6;
}

static int is_rotate_order(int order)
{
  return order >= 6 && order < 12;
}

} // namespace xxx
