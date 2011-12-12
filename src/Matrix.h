/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MATRIX_H
#define MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

struct Matrix {
	double e[16]; /* row-major */
};

extern void MatIdentity(struct Matrix *dst);

extern void MatTranslate(struct Matrix *dst, double tx, double ty, double tz );
extern void MatScale(struct Matrix *dst, double sx, double sy, double sz );
extern void MatRotateX(struct Matrix *dst, double angle);
extern void MatRotateY(struct Matrix *dst, double angle);
extern void MatRotateZ(struct Matrix *dst, double angle);

extern void MatMultiply(struct Matrix *dst, const struct Matrix *a, const struct Matrix *b);
extern void MatInverse(struct Matrix *dst, const struct Matrix *a);

extern void MatPrint(const struct Matrix *m);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

