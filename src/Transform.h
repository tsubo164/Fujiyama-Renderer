/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

struct Matrix;

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

extern void ComputeMatrix(
		int transform_order, int rotate_order,
		double tx, double ty, double tz,
		double rx, double ry, double rz,
		double sx, double sy, double sz,
		struct Matrix *m);

extern int IsValidTransformOrder(int order);
extern int IsValidRotatedOrder(int order);

extern void TransformPoint(double *point, const struct Matrix *m);
extern void TransformVector(double *vector, const struct Matrix *m);
extern void TransformBounds(double *bounds, const struct Matrix *m);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

