/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Transform.h"
#include "Matrix.h"
#include "Vector.h"
#include "Box.h"
#include <assert.h>
#include <float.h>

static int is_transform_order(int order);
static int is_rotate_order(int order);
static void update_matrix(struct Transform *transform);
static void make_transform_matrix(
		int transform_order, int rotate_order,
		double tx, double ty, double tz,
		double rx, double ry, double rz,
		double sx, double sy, double sz,
		double *m);

void XfmReset(struct Transform *transform)
{
	MatIdentity(transform->matrix);
	MatIdentity(transform->inverse);

	transform->transform_order = ORDER_SRT;
	transform->rotate_order = ORDER_XYZ;

	VEC3_SET(transform->translate, 0, 0, 0);
	VEC3_SET(transform->rotate, 0, 0, 0);
	VEC3_SET(transform->scale, 1, 1, 1);
}

void XfmTransformPoint(const struct Transform *transform, double *point)
{
	MatTransformPoint(transform->matrix, point);
}

void XfmTransformVector(const struct Transform *transform, double *vector)
{
	MatTransformVector(transform->matrix, vector);
}

void XfmTransformBounds(const struct Transform *transform, double *bounds)
{
	MatTransformBounds(transform->matrix, bounds);
}

void XfmTransformPointInverse(const struct Transform *transform, double *point)
{
	MatTransformPoint(transform->inverse, point);
}

void XfmTransformVectorInverse(const struct Transform *transform, double *vector)
{
	MatTransformVector(transform->inverse, vector);
}

void XfmTransformBoundsInverse(const struct Transform *transform, double *bounds)
{
	MatTransformBounds(transform->inverse, bounds);
}

void XfmSetTranslate(struct Transform *transform, double tx, double ty, double tz)
{
	VEC3_SET(transform->translate, tx, ty, tz);
	update_matrix(transform);
}

void XfmSetRotate(struct Transform *transform, double rx, double ry, double rz)
{
	VEC3_SET(transform->rotate, rx, ry, rz);
	update_matrix(transform);
}

void XfmSetScale(struct Transform *transform, double sx, double sy, double sz)
{
	VEC3_SET(transform->scale, sx, sy, sz);
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
	VEC3_SET(transform->translate, tx, ty, tz);
	VEC3_SET(transform->rotate, rx, ry, rz);
	VEC3_SET(transform->scale, sx, sy, sz);
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

	list->last_sample_time = -FLT_MAX;
	list->transform_order = ORDER_SRT;
	list->rotate_order = ORDER_ZXY;

	VEC3_SET(list->scale.samples[0].vector, 1, 1, 1);
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

void XfmLerpTransformSample(struct TransformSampleList *list, double time)
{
	struct PropertySample T = INIT_PROPERTYSAMPLE;
	struct PropertySample R = INIT_PROPERTYSAMPLE;
	struct PropertySample S = INIT_PROPERTYSAMPLE;

	if (list->last_sample_time != time) {
		PropLerpSamples(&list->translate, time, &T);
		PropLerpSamples(&list->rotate, time, &R);
		PropLerpSamples(&list->scale, time, &S);

		XfmSetTransform(&list->transform_sample,
			list->transform_order, list->rotate_order,
			T.vector[0], T.vector[1], T.vector[2],
			R.vector[0], R.vector[1], R.vector[2],
			S.vector[0], S.vector[1], S.vector[2]);

		list->last_sample_time = time;
	}
}

void XfmLerpTransformSample2(const struct TransformSampleList *list, double time,
		struct Transform *transform_interp)
{
	struct PropertySample T = INIT_PROPERTYSAMPLE;
	struct PropertySample R = INIT_PROPERTYSAMPLE;
	struct PropertySample S = INIT_PROPERTYSAMPLE;

	if (list->last_sample_time != time) {
		struct TransformSampleList *mutable_list = (struct TransformSampleList *) list;
		PropLerpSamples(&list->translate, time, &T);
		PropLerpSamples(&list->rotate, time, &R);
		PropLerpSamples(&list->scale, time, &S);

		XfmSetTransform(&mutable_list->transform_sample,
			list->transform_order, list->rotate_order,
			T.vector[0], T.vector[1], T.vector[2],
			R.vector[0], R.vector[1], R.vector[2],
			S.vector[0], S.vector[1], S.vector[2]);

		mutable_list->last_sample_time = time;
	}

	*transform_interp = list->transform_sample;
}

static void update_matrix(struct Transform *transform)
{
	make_transform_matrix(transform->transform_order, transform->rotate_order,
			transform->translate[0], transform->translate[1], transform->translate[2],
			transform->rotate[0],    transform->rotate[1],    transform->rotate[2],
			transform->scale[0],     transform->scale[1],     transform->scale[2],
			transform->matrix);

	MatInverse(transform->inverse, transform->matrix);
}

static void make_transform_matrix(
		int transform_order, int rotate_order,
		double tx, double ty, double tz,
		double rx, double ry, double rz,
		double sx, double sy, double sz,
		double *transform)
{
	int i;
	double T[16], R[16], S[16], RX[16], RY[16], RZ[16];
	double *queue[3];

	MatTranslate(T, tx, ty, tz);
	MatRotateX(RX, rx);
	MatRotateY(RY, ry);
	MatRotateZ(RZ, rz);
	MatScale(S, sx, sy, sz);

	switch (rotate_order) {
	case ORDER_XYZ: VEC3_SET(queue, RX, RY, RZ); break;
	case ORDER_XZY: VEC3_SET(queue, RX, RZ, RY); break;
	case ORDER_YXZ: VEC3_SET(queue, RY, RX, RZ); break;
	case ORDER_YZX: VEC3_SET(queue, RY, RZ, RX); break;
	case ORDER_ZXY: VEC3_SET(queue, RZ, RX, RY); break;
	case ORDER_ZYX: VEC3_SET(queue, RZ, RY, RX); break;
	default:
		assert(!"invalid rotate order");
		break;
	}

	MatIdentity(R);
	for (i = 0; i < 3; i++)
		MatMultiply(R, queue[i], R);

	switch (transform_order) {
	case ORDER_SRT: VEC3_SET(queue, S, R, T); break;
	case ORDER_STR: VEC3_SET(queue, S, T, R); break;
	case ORDER_RST: VEC3_SET(queue, R, S, T); break;
	case ORDER_RTS: VEC3_SET(queue, R, T, S); break;
	case ORDER_TRS: VEC3_SET(queue, T, R, S); break;
	case ORDER_TSR: VEC3_SET(queue, T, S, R); break;
	default:
		assert(!"invalid transform order order");
		break;
	}

	MatIdentity(transform);
	for (i = 0; i < 3; i++)
		MatMultiply(transform, queue[i], transform);
}

static int is_transform_order(int order)
{
	return order >= 0 && order < 6;
}

static int is_rotate_order(int order)
{
	return order >= 6 && order < 12;
}

