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
		struct Matrix *m);

void XfmReset(struct Transform *transform)
{
	MatIdentity(&transform->matrix);
	MatIdentity(&transform->inverse);

	transform->transform_order = ORDER_SRT;
	transform->rotate_order = ORDER_XYZ;

	VEC3_SET(transform->translate, 0, 0, 0);
	VEC3_SET(transform->rotate, 0, 0, 0);
	VEC3_SET(transform->scale, 1, 1, 1);
}

void XfmTransformPoint(const struct Transform *transform, double *point)
{
	TransformPoint(&transform->matrix, point);
}

void XfmTransformVector(const struct Transform *transform, double *vector)
{
	TransformVector(&transform->matrix, vector);
}

void XfmTransformBounds(const struct Transform *transform, double *bounds)
{
	TransformBounds(&transform->matrix, bounds);
}

void XfmTransformPointInverse(const struct Transform *transform, double *point)
{
	TransformPoint(&transform->inverse, point);
}

void XfmTransformVectorInverse(const struct Transform *transform, double *vector)
{
	TransformVector(&transform->inverse, vector);
}

void XfmTransformBoundsInverse(const struct Transform *transform, double *bounds)
{
	TransformBounds(&transform->inverse, bounds);
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

static void update_matrix(struct Transform *transform)
{
	make_transform_matrix(transform->transform_order, transform->rotate_order,
			transform->translate[0], transform->translate[1], transform->translate[2],
			transform->rotate[0],    transform->rotate[1],    transform->rotate[2],
			transform->scale[0],     transform->scale[1],     transform->scale[2],
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
	int i;
	struct Matrix T, R, S, RX, RY, RZ;
	struct Matrix *queue[3];

	MatTranslate(&T, tx, ty, tz);
	MatRotateX(&RX, rx);
	MatRotateY(&RY, ry);
	MatRotateZ(&RZ, rz);
	MatScale(&S, sx, sy, sz);

	switch (rotate_order) {
	case ORDER_XYZ: VEC3_SET(queue, &RX, &RY, &RZ); break;
	case ORDER_XZY: VEC3_SET(queue, &RX, &RZ, &RY); break;
	case ORDER_YXZ: VEC3_SET(queue, &RY, &RX, &RZ); break;
	case ORDER_YZX: VEC3_SET(queue, &RY, &RZ, &RX); break;
	case ORDER_ZXY: VEC3_SET(queue, &RZ, &RX, &RY); break;
	case ORDER_ZYX: VEC3_SET(queue, &RZ, &RY, &RX); break;
	default:
		assert(!"invalid rotate order");
		break;
	}

	MatIdentity(&R);
	for (i = 0; i < 3; i++)
		MatMultiply(&R, queue[i], &R);

	switch (transform_order) {
	case ORDER_SRT: VEC3_SET(queue, &S, &R, &T); break;
	case ORDER_STR: VEC3_SET(queue, &S, &T, &R); break;
	case ORDER_RST: VEC3_SET(queue, &R, &S, &T); break;
	case ORDER_RTS: VEC3_SET(queue, &R, &T, &S); break;
	case ORDER_TRS: VEC3_SET(queue, &T, &R, &S); break;
	case ORDER_TSR: VEC3_SET(queue, &T, &S, &R); break;
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

void TransformPoint(const struct Matrix *m, double *point)
{
	double tmp[3];
	tmp[0] = m->e[0]*point[0] + m->e[1]*point[1] + m->e[2]*point[2] + m->e[3];
	tmp[1] = m->e[4]*point[0] + m->e[5]*point[1] + m->e[6]*point[2] + m->e[7];
	tmp[2] = m->e[8]*point[0] + m->e[9]*point[1] + m->e[10]*point[2] + m->e[11];
	point[0] = tmp[0];
	point[1] = tmp[1];
	point[2] = tmp[2];
}

void TransformVector(const struct Matrix *m, double *vector)
{
	double tmp[3];
	tmp[0] = m->e[0]*vector[0] + m->e[1]*vector[1] + m->e[2]*vector[2];
	tmp[1] = m->e[4]*vector[0] + m->e[5]*vector[1] + m->e[6]*vector[2];
	tmp[2] = m->e[8]*vector[0] + m->e[9]*vector[1] + m->e[10]*vector[2];
	vector[0] = tmp[0];
	vector[1] = tmp[1];
	vector[2] = tmp[2];
}

void TransformBounds(const struct Matrix *m, double *bounds)
{
	int i, j, k;
	double box[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			for (k = 0; k < 2; k++) {
				double pt[3];
				VEC3_SET(pt, bounds[3*i], bounds[3*j+1], bounds[3*k+2]);
				TransformPoint(m, pt);
				BoxAddPoint(box, pt);
			}
		}
	}
	BOX3_COPY(bounds, box);
}

