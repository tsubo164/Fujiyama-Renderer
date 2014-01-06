/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MATRIX_H
#define FJ_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

struct Matrix {
  /* row-major */
  double e[16];
};

struct Vector;
struct Box;

extern void MatIdentity(struct Matrix *dst);
extern void MatSet(struct Matrix *dst,
    double e00, double e01, double e02, double e03,
    double e10, double e11, double e12, double e13,
    double e20, double e21, double e22, double e23,
    double e30, double e31, double e32, double e33);

extern void MatTranslate(struct Matrix *dst, double tx, double ty, double tz );
extern void MatScale(struct Matrix *dst, double sx, double sy, double sz );
extern void MatRotateX(struct Matrix *dst, double angle);
extern void MatRotateY(struct Matrix *dst, double angle);
extern void MatRotateZ(struct Matrix *dst, double angle);

extern void MatMultiply(struct Matrix *dst, const struct Matrix *a, const struct Matrix *b);
extern void MatInverse(struct Matrix *dst, const struct Matrix *a);

extern void MatTransformPoint(const struct Matrix *m, struct Vector *point);
extern void MatTransformVector(const struct Matrix *m, struct Vector *vector);
extern void MatTransformBounds(const struct Matrix *m, struct Box *bounds);

extern void MatPrint(const struct Matrix *m);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
