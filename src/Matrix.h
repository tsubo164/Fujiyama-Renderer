/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MATRIX_H
#define MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

/* double matrix[16] row-matrix */

extern void MatIdentity(double *dst);
extern void MatSet(double *dst,
		double e00, double e01, double e02, double e03,
		double e10, double e11, double e12, double e13,
		double e20, double e21, double e22, double e23,
		double e30, double e31, double e32, double e33);

extern void MatTranslate(double *dst, double tx, double ty, double tz );
extern void MatScale(double *dst, double sx, double sy, double sz );
extern void MatRotateX(double *dst, double angle);
extern void MatRotateY(double *dst, double angle);
extern void MatRotateZ(double *dst, double angle);

extern void MatMultiply(double *dst, const double *a, const double *b);
extern void MatInverse(double *dst, const double *a);

extern void MatTransformPoint(const double *m, double *point);
extern void MatTransformVector(const double *m, double *vector);
extern void MatTransformBounds(const double *m, double *bounds);

extern void MatPrint(const double *m);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

