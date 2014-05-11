/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_transform.h"
#include "fj_matrix.h"
#include "fj_box.h"
#include <assert.h>
#include <float.h>

namespace fj {

static int is_transform_order(int order);
static int is_rotate_order(int order);
static void update_matrix(struct Transform *transform);
static void make_transform_matrix(
    int transform_order, int rotate_order,
    double tx, double ty, double tz,
    double rx, double ry, double rz,
    double sx, double sy, double sz,
    struct Matrix *transform);

void XfmReset(struct Transform *transform)
{
  MatIdentity(&transform->matrix);
  MatIdentity(&transform->inverse);

  transform->transform_order = ORDER_SRT;
  transform->rotate_order = ORDER_XYZ;

  VEC3_SET(&transform->translate, 0, 0, 0);
  VEC3_SET(&transform->rotate, 0, 0, 0);
  VEC3_SET(&transform->scale, 1, 1, 1);
}

void XfmTransformPoint(const struct Transform *transform, struct Vector *point)
{
  MatTransformPoint(&transform->matrix, point);
}

void XfmTransformVector(const struct Transform *transform, struct Vector *vector)
{
  MatTransformVector(&transform->matrix, vector);
}

void XfmTransformBounds(const struct Transform *transform, struct Box *bounds)
{
  MatTransformBounds(&transform->matrix, bounds);
}

void XfmTransformPointInverse(const struct Transform *transform, struct Vector *point)
{
  MatTransformPoint(&transform->inverse, point);
}

void XfmTransformVectorInverse(const struct Transform *transform, struct Vector *vector)
{
  MatTransformVector(&transform->inverse, vector);
}

void XfmTransformBoundsInverse(const struct Transform *transform, struct Box *bounds)
{
  MatTransformBounds(&transform->inverse, bounds);
}

void XfmSetTranslate(struct Transform *transform, double tx, double ty, double tz)
{
  VEC3_SET(&transform->translate, tx, ty, tz);
  update_matrix(transform);
}

void XfmSetRotate(struct Transform *transform, double rx, double ry, double rz)
{
  VEC3_SET(&transform->rotate, rx, ry, rz);
  update_matrix(transform);
}

void XfmSetScale(struct Transform *transform, double sx, double sy, double sz)
{
  VEC3_SET(&transform->scale, sx, sy, sz);
  update_matrix(transform);
}

void XfmSetTransformOrder(struct Transform *transform, int order)
{
  assert(is_transform_order(order));
  transform->transform_order = order;
  update_matrix(transform);
}

void XfmSetRotateOrder(struct Transform *transform, int order)
{
  assert(is_rotate_order(order));
  transform->rotate_order = order;
  update_matrix(transform);
}

void XfmSetTransform(struct Transform *transform,
    int transform_order, int rotate_order,
    double tx, double ty, double tz,
    double rx, double ry, double rz,
    double sx, double sy, double sz)
{
  transform->transform_order = transform_order;
  transform->rotate_order = rotate_order;
  VEC3_SET(&transform->translate, tx, ty, tz);
  VEC3_SET(&transform->rotate, rx, ry, rz);
  VEC3_SET(&transform->scale, sx, sy, sz);
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

void XfmInitTransformSampleList(struct TransformSampleList *list)
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

void XfmPushTranslateSample(struct TransformSampleList *list,
    double tx, double ty, double tz, double time)
{
  struct PropertySample sample = {{0, 0, 0}, 0};

  sample.vector[0] = tx;
  sample.vector[1] = ty;
  sample.vector[2] = tz;
  sample.time = time;

  PropPushSample(&list->translate, &sample);
}

void XfmPushRotateSample(struct TransformSampleList *list,
    double rx, double ry, double rz, double time)
{
  struct PropertySample sample = {{0, 0, 0}, 0};

  sample.vector[0] = rx;
  sample.vector[1] = ry;
  sample.vector[2] = rz;
  sample.time = time;

  PropPushSample(&list->rotate, &sample);
}

void XfmPushScaleSample(struct TransformSampleList *list,
    double sx, double sy, double sz, double time)
{
  struct PropertySample sample = {{0, 0, 0}, 0};

  sample.vector[0] = sx;
  sample.vector[1] = sy;
  sample.vector[2] = sz;
  sample.time = time;

  PropPushSample(&list->scale, &sample);
}

void XfmSetSampleTransformOrder(struct TransformSampleList *list, int order)
{
  assert(is_transform_order(order));
  list->transform_order = order;
}

void XfmSetSampleRotateOrder(struct TransformSampleList *list, int order)
{
  assert(is_rotate_order(order));
  list->rotate_order = order;
}

void XfmLerpTransformSample(const struct TransformSampleList *list, double time,
    struct Transform *transform_interp)
{
  struct PropertySample T = INIT_PROPERTYSAMPLE;
  struct PropertySample R = INIT_PROPERTYSAMPLE;
  struct PropertySample S = INIT_PROPERTYSAMPLE;

  PropLerpSamples(&list->translate, time, &T);
  PropLerpSamples(&list->rotate, time, &R);
  PropLerpSamples(&list->scale, time, &S);

  XfmSetTransform(transform_interp,
    list->transform_order, list->rotate_order,
    T.vector[0], T.vector[1], T.vector[2],
    R.vector[0], R.vector[1], R.vector[2],
    S.vector[0], S.vector[1], S.vector[2]);
}

static void update_matrix(struct Transform *transform)
{
  make_transform_matrix(transform->transform_order, transform->rotate_order,
      transform->translate.x, transform->translate.y, transform->translate.z,
      transform->rotate.x,    transform->rotate.y,    transform->rotate.z,
      transform->scale.x,     transform->scale.y,     transform->scale.z,
      &transform->matrix);

  MatInverse(&transform->inverse, &transform->matrix);
}

static void make_transform_matrix(
    int transform_order, int rotate_order,
    double tx, double ty, double tz,
    double rx, double ry, double rz,
    double sx, double sy, double sz,
    struct Matrix *transform)
{
#define QUEUE_SET(dst,q0,q1,q2) do { \
  (dst)[0] = (q0); \
  (dst)[1] = (q1); \
  (dst)[2] = (q2); \
  } while(0)

  int i;
  struct Matrix T, R, S, RX, RY, RZ;
  struct Matrix *queue[3];

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
    MatMultiply(&R, queue[i], &R);

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
    MatMultiply(transform, queue[i], transform);

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
