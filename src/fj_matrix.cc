// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_matrix.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_box.h"
#include <cstdio>
#include <cfloat>
#include <cmath>

namespace fj {

static inline void set_matrix(Matrix *dst,
     Real e00, Real e01, Real e02, Real e03,
     Real e10, Real e11, Real e12, Real e13,
     Real e20, Real e21, Real e22, Real e23,
     Real e30, Real e31, Real e32, Real e33)
{
  dst->e[0]=e00;  dst->e[1]=e01;  dst->e[2]=e02;  dst->e[3]=e03;
  dst->e[4]=e10;  dst->e[5]=e11;  dst->e[6]=e12;  dst->e[7]=e13;
  dst->e[8]=e20;  dst->e[9]=e21;  dst->e[10]=e22; dst->e[11]=e23;
  dst->e[12]=e30; dst->e[13]=e31; dst->e[14]=e32; dst->e[15]=e33;
}

void MatIdentity(Matrix *dst)
{
  set_matrix(dst,
      1., 0., 0., 0.,
      0., 1., 0., 0.,
      0., 0., 1., 0.,
      0., 0., 0., 1.);
}

void MatSet(Matrix *dst,
    Real e00, Real e01, Real e02, Real e03,
    Real e10, Real e11, Real e12, Real e13,
    Real e20, Real e21, Real e22, Real e23,
    Real e30, Real e31, Real e32, Real e33)
{
  set_matrix(dst,
      e00, e01, e02, e03,
      e10, e11, e12, e13,
      e20, e21, e22, e23,
      e30, e31, e32, e33);
}

void MatTranslate(Matrix *dst, Real tx, Real ty, Real tz )
{
  set_matrix(dst,
      1., 0., 0., tx,
      0., 1., 0., ty,
      0., 0., 1., tz,
      0., 0., 0., 1.);
}

void MatScale(Matrix *dst, Real sx, Real sy, Real sz )
{
  set_matrix(dst,
      sx, 0., 0., 0.,
      0., sy, 0., 0.,
      0., 0., sz, 0.,
      0., 0., 0., 1.);
}

void MatRotateX(Matrix *dst, Real angle)
{
  const Real sint = sin(Radian(angle));
  const Real cost = cos(Radian(angle));

  set_matrix(dst,
      1.,   0.,    0., 0.,
      0., cost, -sint, 0.,
      0., sint,  cost, 0.,
      0.,   0.,    0., 1.);
}

void MatRotateY(Matrix *dst, Real angle)
{
  const Real sint = sin(Radian(angle));
  const Real cost = cos(Radian(angle));

  set_matrix(dst,
       cost, 0.,  sint,  0.,
         0., 1.,    0.,  0.,
      -sint, 0.,  cost,  0.,
         0., 0.,    0.,  1.);
}

void MatRotateZ(Matrix *dst, Real angle)
{
  const Real sint = sin(Radian(angle));
  const Real cost = cos(Radian(angle));

  set_matrix(dst,
      cost, -sint, 0., 0.,
      sint,  cost, 0., 0.,
        0.,    0., 1., 0.,
        0.,    0., 0., 1.);
}

void MatMultiply(Matrix *dst, const Matrix &a, const Matrix &b)
{
  Matrix c;

  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      c.e[4*j+i] = 0.;
      for (int k = 0; k < 4; k++) {
        c.e[4*j+i] += a.e[4*j+k] * b.e[4*k+i];
      }
    }
  }
  *dst = c;
}

void MatInverse(Matrix *dst, const Matrix &a)
{
  /* 4x4-matrix inversion with Cramer's Rule.
     codes from intel web site's pdf */

  int i, j;
  Real tmp[12]; /* temp array for pairs */
  Real src[16]; /* array of transpose source matrix */
  Real det; /* determinant */

  /* transpose matrix */
  for (i = 0; i < 4; i++) {
    src[i]      = a.e[i*4];
    src[i +  4] = a.e[i*4 + 1];
    src[i +  8] = a.e[i*4 + 2];
    src[i + 12] = a.e[i*4 + 3];
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

void MatTransformPoint(const Matrix &m, Vector *point)
{
  *point = Vector(
      m.e[0]*point->x + m.e[1]*point->y + m.e[2] *point->z + m.e[3],
      m.e[4]*point->x + m.e[5]*point->y + m.e[6] *point->z + m.e[7],
      m.e[8]*point->x + m.e[9]*point->y + m.e[10]*point->z + m.e[11]);
}

void MatTransformVector(const Matrix &m, Vector *vector)
{
  *vector = Vector(
      m.e[0]*vector->x + m.e[1]*vector->y + m.e[2] *vector->z,
      m.e[4]*vector->x + m.e[5]*vector->y + m.e[6] *vector->z,
      m.e[8]*vector->x + m.e[9]*vector->y + m.e[10]*vector->z);
}

void MatTransformBounds(const Matrix &m, Box *bounds)
{
  Vector pt;
  Box box;

  box.ReverseInfinite();

#define TRANSFORM_BOX_VERTEX(minmax0, minmax1, minmax2) do { \
  pt.x = bounds->minmax0.x; \
  pt.y = bounds->minmax1.y; \
  pt.z = bounds->minmax2.z; \
  MatTransformPoint(m, &pt); \
  box.AddPoint(pt); \
} while (0)
  TRANSFORM_BOX_VERTEX  (min, min, min);
  TRANSFORM_BOX_VERTEX  (max, min, min);
  TRANSFORM_BOX_VERTEX  (min, max, min);
  TRANSFORM_BOX_VERTEX  (min, min, max);
  TRANSFORM_BOX_VERTEX  (min, max, max);
  TRANSFORM_BOX_VERTEX  (max, min, max);
  TRANSFORM_BOX_VERTEX  (max, max, min);
  TRANSFORM_BOX_VERTEX  (max, max, max);
#undef TRANSFORM_BOX_VERTEX

  *bounds = box;
}

void MatPrint(const Matrix &m)
{
  for (int i = 0; i < 4; i++) {
    printf("%g, %g, %g, %g\n", m.e[4*i+0], m.e[4*i+1], m.e[4*i+2], m.e[4*i+3]);
  }
}

} // namespace xxx
