/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

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

struct Transform {
	struct Matrix matrix;
	struct Matrix inverse;

	int transform_order;
	int rotate_order;

	double translate[3];
	double rotate[3];
	double scale[3];
};

extern void XfmReset(struct Transform *transform);

extern void XfmTransformPoint(const struct Transform *transform, double *point);
extern void XfmTransformVector(const struct Transform *transform, double *vector);
extern void XfmTransformBounds(const struct Transform *transform, double *bounds);

extern void XfmTransformPointInverse(const struct Transform *transform, double *point);
extern void XfmTransformVectorInverse(const struct Transform *transform, double *vector);
extern void XfmTransformBoundsInverse(const struct Transform *transform, double *bounds);

extern void XfmSetTranslate(struct Transform *transform, double tx, double ty, double tz);
extern void XfmSetRotate(struct Transform *transform, double rx, double ry, double rz);
extern void XfmSetScale(struct Transform *transform, double sx, double sy, double sz);
extern void XfmSetTransformOrder(struct Transform *transform, int order);
extern void XfmSetRotateOrder(struct Transform *transform, int order);

/* interfaces for matrix */
extern void TransformPoint(const struct Matrix *m, double *point);
extern void TransformVector(const struct Matrix *m, double *vector);
extern void TransformBounds(const struct Matrix *m, double *bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

