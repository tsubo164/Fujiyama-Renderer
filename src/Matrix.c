/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Matrix.h"
#include "Numeric.h"
#include "Vector.h"
#include "Box.h"
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <math.h>

#define MAT_SET(dst, \
		 e00, e01, e02, e03, \
		 e10, e11, e12, e13, \
		 e20, e21, e22, e23, \
		 e30, e31, e32, e33) \
do { \
	dst[0]=e00;  dst[1]=e01;  dst[2]=e02;  dst[3]=e03; \
	dst[4]=e10;  dst[5]=e11;  dst[6]=e12;  dst[7]=e13; \
	dst[8]=e20;  dst[9]=e21;  dst[10]=e22; dst[11]=e23; \
	dst[12]=e30; dst[13]=e31; dst[14]=e32; dst[15]=e33; \
} while(0)

void MatIdentity(struct Matrix *dst)
{
	MAT_SET(dst->e,
			1., 0., 0., 0.,
			0., 1., 0., 0.,
			0., 0., 1., 0.,
			0., 0., 0., 1.);
}

void MatCopy(double *dst, const double *src)
{
	memmove(dst, src, sizeof(double) * 16);
}

extern void MatSet(struct Matrix *dst,
		double e00, double e01, double e02, double e03,
		double e10, double e11, double e12, double e13,
		double e20, double e21, double e22, double e23,
		double e30, double e31, double e32, double e33)
{
	MAT_SET(dst->e,
		e00, e01, e02, e03,
		e10, e11, e12, e13,
		e20, e21, e22, e23,
		e30, e31, e32, e33);
}

void MatTranslate(struct Matrix *dst, double tx, double ty, double tz )
{
	MAT_SET(dst->e,
			1., 0., 0., tx,
			0., 1., 0., ty,
			0., 0., 1., tz,
			0., 0., 0., 1.);
}

void MatScale(struct Matrix *dst, double sx, double sy, double sz )
{
	MAT_SET(dst->e,
			sx, 0., 0., 0.,
			0., sy, 0., 0.,
			0., 0., sz, 0.,
			0., 0., 0., 1.);
}

void MatRotateX(struct Matrix *dst, double angle)
{
	double sint = sin(RADIAN(angle));
	double cost = cos(RADIAN(angle));

	MAT_SET(dst->e,
			1.,   0.,    0., 0.,
			0., cost, -sint, 0.,
			0., sint,  cost, 0.,
			0.,   0.,    0., 1.);
}

void MatRotateY(struct Matrix *dst, double angle)
{
	double sint = sin(RADIAN(angle));
	double cost = cos(RADIAN(angle));

	MAT_SET(dst->e,
			 cost, 0.,  sint,  0.,
			   0., 1.,    0.,  0.,
			-sint, 0.,  cost,  0.,
			   0., 0.,    0.,  1.);
}

void MatRotateZ(struct Matrix *dst, double angle)
{
	double sint = sin(RADIAN(angle));
	double cost = cos(RADIAN(angle));

	MAT_SET(dst->e,
			cost, -sint, 0., 0.,
			sint,  cost, 0., 0.,
			  0.,    0., 1., 0.,
			  0.,    0., 0., 1.);
}

void MatMultiply(struct Matrix *dst, const struct Matrix *a, const struct Matrix *b)
{
	const struct Matrix aa = *a;
	const struct Matrix bb = *b;
	int i, j, k;
	for (j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) {
			dst->e[4*j+i] = 0.;
			for (k = 0; k < 4; k++) {
				dst->e[4*j+i] += aa.e[4*j+k] * bb.e[4*k+i];
			}
		}
	}
}

void MatInverse(struct Matrix *dst, const struct Matrix *a)
{
	/* 4x4-matrix inversion with Cramer's Rule.
	   codes from intel web site's pdf */

	int i, j;
	double tmp[12]; /* temp array for pairs */
	double src[16]; /* array of transpose source matrix */
	double det; /* determinant */

	/* transpose matrix */
	for (i = 0; i < 4; i++) {
		src[i]      = a->e[i*4];
		src[i +  4] = a->e[i*4 + 1];
		src[i +  8] = a->e[i*4 + 2];
		src[i + 12] = a->e[i*4 + 3];
	}

	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9]  * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9]  * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8]  * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8]  * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8]  * src[13];
	tmp[11] = src[9]  * src[12];

	/* calculate first 8 elements (cofactors) */
	dst->e[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst->e[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst->e[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst->e[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst->e[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst->e[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst->e[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst->e[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst->e[4]  = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst->e[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst->e[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst->e[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst->e[6]  = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst->e[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst->e[7]  = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst->e[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0]  = src[2]*src[7];
	tmp[1]  = src[3]*src[6];
	tmp[2]  = src[1]*src[7];
	tmp[3]  = src[3]*src[5];
	tmp[4]  = src[1]*src[6];
	tmp[5]  = src[2]*src[5];
	tmp[6]  = src[0]*src[7];
	tmp[7]  = src[3]*src[4];
	tmp[8]  = src[0]*src[6];
	tmp[9]  = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	/* calculate second 8 elements (cofactors) */
	dst->e[8]   = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst->e[8]  -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst->e[9]   = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst->e[9]  -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst->e[10]  = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst->e[10] -= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst->e[11]  = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst->e[11] -= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst->e[12]  = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst->e[12] -= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst->e[13]  = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst->e[13] -= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst->e[14]  = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst->e[14] -= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst->e[15]  = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst->e[15] -= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

	/* calculate determinant */
	det = src[0]*dst->e[0]+src[1]*dst->e[1]+src[2]*dst->e[2]+src[3]*dst->e[3];

	/* calculate matrix inverse */
	det = 1./det;
	for (j = 0; j < 16; j++)
		dst->e[j] *= det;
}

void MatTransformPoint(const double *m, double *point)
{
	double tmp[3];
	tmp[0] = m[0]*point[0] + m[1]*point[1] + m[2]*point[2] + m[3];
	tmp[1] = m[4]*point[0] + m[5]*point[1] + m[6]*point[2] + m[7];
	tmp[2] = m[8]*point[0] + m[9]*point[1] + m[10]*point[2] + m[11];
	point[0] = tmp[0];
	point[1] = tmp[1];
	point[2] = tmp[2];
}

void MatTransformVector(const double *m, double *vector)
{
	double tmp[3];
	tmp[0] = m[0]*vector[0] + m[1]*vector[1] + m[2]*vector[2];
	tmp[1] = m[4]*vector[0] + m[5]*vector[1] + m[6]*vector[2];
	tmp[2] = m[8]*vector[0] + m[9]*vector[1] + m[10]*vector[2];
	vector[0] = tmp[0];
	vector[1] = tmp[1];
	vector[2] = tmp[2];
}

void MatTransformBounds(const double *m, double *bounds)
{
	int i, j, k;
	double box[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			for (k = 0; k < 2; k++) {
				double pt[3];
				VEC3_SET(pt, bounds[3*i], bounds[3*j+1], bounds[3*k+2]);
				MatTransformPoint(m, pt);
				BoxAddPoint(box, pt);
			}
		}
	}
	BOX3_COPY(bounds, box);
}

void MatPrint(const struct Matrix *m)
{
	int i;
	for (i = 0; i < 4; i++) {
		printf("%g, %g, %g, %g\n", m->e[4*i+0], m->e[4*i+1], m->e[4*i+2], m->e[4*i+3]);
	}
}

