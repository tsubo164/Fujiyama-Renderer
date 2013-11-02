/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_triangle.h"
#include "fj_tex_coord.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_box.h"
#include <float.h>

#define EPSILON 1e-6

#define VEC_SUB(dst,a,b) do { \
  (dst)->x = (a)->x - (b)->x; \
  (dst)->y = (a)->y - (b)->y; \
  (dst)->z = (a)->z - (b)->z; \
  } while(0)

double TriComputeArea(
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2)
{
  struct Vector a;
  struct Vector b;
  struct Vector cross;

  VEC_SUB(&a, vert1, vert0);
  VEC_SUB(&b, vert2, vert0);
  VEC3_CROSS(&cross, &a, &b);

  return .5 * VEC3_LEN(&cross);
}

void TriComputeBounds(struct Box *box,
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2)
{
  BOX3_SET(box, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

  BoxAddPoint(box, vert0);
  BoxAddPoint(box, vert1);
  BoxAddPoint(box, vert2);
}

void TriComputeFaceNormal(struct Vector *N,
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2)
{
  struct Vector edge1;
  struct Vector edge2;

  VEC_SUB(&edge1, vert1, vert0);
  VEC_SUB(&edge2, vert2, vert0);

  VEC3_CROSS(N, &edge1, &edge2);
  VEC3_NORMALIZE(N);
}

void TriComputeNormal(struct Vector *N,
    const struct Vector *N0, const struct Vector *N1, const struct Vector *N2,
    double u, double v)
{
  /* N = (1-u-v) * N0 + u * N1 + v * N2 */
  const double t = 1-u-v;

  N->x = t * N0->x + u * N1->x + v * N2->x;
  N->y = t * N0->y + u * N1->y + v * N2->y;
  N->z = t * N0->z + u * N1->z + v * N2->z;
}

/* Codes from
 * Fast, minimum storage ray-triangle intersection.
 * Tomas Möller and Ben Trumbore.
 * Journal of Graphics Tools, 2(1):21--28, 1997.
 */
int TriRayIntersect(
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2,
    const struct Vector *orig, const struct Vector *dir, int cull_backfaces,
    double *t, double *u, double *v)
{
  struct Vector edge1, edge2, tvec, pvec, qvec;
  double det, inv_det;

  /* find vectors for two edges sharing vert0 */
  VEC_SUB(&edge1, vert1, vert0);
  VEC_SUB(&edge2, vert2, vert0);

  /* begin calculating determinant - also used to calculate U parameter */
  VEC3_CROSS(&pvec, dir, &edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  det = VEC3_DOT(&edge1, &pvec);

/*#ifdef TEST_CULL*/           /* define TEST_CULL if culling is desired */
  if (cull_backfaces) {
    if (det < EPSILON)
      return 0;

    /* calculate distance from vert0 to ray origin */
    VEC_SUB(&tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = VEC3_DOT(&tvec, &pvec);
    if (*u < 0.0 || *u > det)
      return 0;

    /* prepare to test V parameter */
    VEC3_CROSS(&qvec, &tvec, &edge1);

    /* calculate V parameter and test bounds */
    *v = VEC3_DOT(dir, &qvec);
    if (*v < 0.0 || *u + *v > det)
      return 0;

    /* calculate t, scale parameters, ray intersects triangle */
    *t = VEC3_DOT(&edge2, &qvec);
    inv_det = 1.0 / det;
    *t *= inv_det;
    *u *= inv_det;
    *v *= inv_det;
/*#else*/                    /* the non-culling branch */
  } else {
    if (det > -EPSILON && det < EPSILON)
      return 0;
    inv_det = 1.0 / det;

    /* calculate distance from vert0 to ray origin */
    VEC_SUB(&tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = VEC3_DOT(&tvec, &pvec) * inv_det;
    if (*u < 0.0 || *u > 1.0)
      return 0;

    /* prepare to test V parameter */
    VEC3_CROSS(&qvec, &tvec, &edge1);

    /* calculate V parameter and test bounds */
    *v = VEC3_DOT(dir, &qvec) * inv_det;
    if (*v < 0.0 || *u + *v > 1.0)
      return 0;

    /* calculate t, ray intersects triangle */
    *t = VEC3_DOT(&edge2, &qvec) * inv_det;
/*#endif*/
  }
  return 1;
}

void TriComputeDerivatives(
    const struct Vector *vert0, const struct Vector *vert1, const struct Vector *vert2,
    const struct TexCoord *tex0, const struct TexCoord *tex1, const struct TexCoord *tex2,
    struct Vector *dPdu, struct Vector *dPdv)
{
  struct Vector dP1 = {0, 0, 0};
  struct Vector dP2 = {0, 0, 0};
  const float du1 = tex1->u - tex0->u;
  const float du2 = tex2->u - tex0->u;
  const float dv1 = tex1->v - tex0->v;
  const float dv2 = tex2->v - tex0->v;
  float determinant = 0;
  float invdet = 0;

  VEC_SUB(&dP1, vert1, vert0);
  VEC_SUB(&dP2, vert2, vert0);

  determinant = du1 * dv2 - dv1 * du2;
  if (determinant == 0) {
    /* TODO error handling */
    const struct Vector zero = {0, 0, 0};
    *dPdu = zero;
    *dPdv = zero;
    return;
  }

  invdet = 1 / determinant;

  dPdu->x =  (dv2 * dP1.x - dv1 * dP2.x) * invdet;
  dPdu->y =  (dv2 * dP1.y - dv1 * dP2.y) * invdet;
  dPdu->z =  (dv2 * dP1.z - dv1 * dP2.z) * invdet;

  dPdv->x = (-du2 * dP1.x + du1 * dP2.x) * invdet;
  dPdv->y = (-du2 * dP1.y + du1 * dP2.y) * invdet;
  dPdv->z = (-du2 * dP1.z + du1 * dP2.z) * invdet;
}

#if 0
static void plane_equation_coeff(
    const struct Vector *v0, const struct Vector *v1, const struct Vector *v2,
    double *A, double *B, double *C, double *D)
{
  struct Vector e1 = {0, 0, 0};
  struct Vector e2 = {0, 0, 0};
  struct Vector cross = {0, 0, 0};

  e1.x = v0->x - v1->x;
  e1.y = v0->y - v1->y;
  e1.z = v0->z - v1->z;

  e2.x = v0->x - v2->x;
  e2.y = v0->y - v2->y;
  e2.z = v0->z - v2->z;

  VEC3_CROSS(&cross, &e1, &e2);

  *A = cross.x;
  *B = cross.y;
  *C = cross.z;
  *D = -VEC3_DOT(&cross, v0);
}

/*
 * http://http.developer.nvidia.com/CgTutorial/cg_tutorial_chapter08.html
 */
void TriComputeDerivatives(
    const struct Vector *vtx0, const struct Vector *vtx1, const struct Vector *vtx2,
    const struct TexCoord *tex0, const struct TexCoord *tex1, const struct TexCoord *tex2,
    struct Vector *dPdu, struct Vector *dPdv)
{
  struct Vector v0, v1, v2;
  double A0, B0, C0, D0;
  double A1, B1, C1, D1;
  double A2, B2, C2, D2;

  v0.x = vtx0->x;
  v0.y = tex0->u;
  v0.z = tex0->v;

  v1.x = vtx1->x;
  v1.y = tex1->u;
  v1.z = tex1->v;

  v2.x = vtx2->x;
  v2.y = tex2->u;
  v2.z = tex2->v;

  plane_equation_coeff(&v0, &v1, &v2, &A0, &B0, &C0, &D0);

  v0.x = vtx0->y;
  v0.y = tex0->u;
  v0.z = tex0->v;

  v1.x = vtx1->y;
  v1.y = tex1->u;
  v1.z = tex1->v;

  v2.x = vtx2->y;
  v2.y = tex2->u;
  v2.z = tex2->v;

  plane_equation_coeff(&v0, &v1, &v2, &A1, &B1, &C1, &D1);

  v0.x = vtx0->z;
  v0.y = tex0->u;
  v0.z = tex0->v;

  v1.x = vtx1->z;
  v1.y = tex1->u;
  v1.z = tex1->v;

  v2.x = vtx2->z;
  v2.y = tex2->u;
  v2.z = tex2->v;

  plane_equation_coeff(&v0, &v1, &v2, &A2, &B2, &C2, &D2);

  dPdu->x = -B0 / A0;
  dPdu->y = -B1 / A1;
  dPdu->z = -B2 / A2;

  dPdv->x = -C0 / A0;
  dPdv->y = -C1 / A1;
  dPdv->z = -C2 / A2;
}
#endif
