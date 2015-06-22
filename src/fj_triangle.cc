// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_triangle.h"
#include "fj_tex_coord.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_box.h"

namespace fj {

static const double EPSILON = 1e-6;

double TriComputeArea(const Vector &vert0, const Vector &vert1, const Vector &vert2)
{
  const Vector a = vert1 - vert0;
  const Vector b = vert2 - vert0;
  const Vector cross = Cross(a, b);

  return .5 * Length(cross);
}

void TriComputeBounds(const Vector &vert0, const Vector &vert1, const Vector &vert2,
    Box *box)
{
  BoxReverseInfinite(box);

  BoxAddPoint(box, vert0);
  BoxAddPoint(box, vert1);
  BoxAddPoint(box, vert2);
}

Vector TriComputeFaceNormal(const Vector &vert0, const Vector &vert1, const Vector &vert2)
{
  const Vector a = vert1 - vert0;
  const Vector b = vert2 - vert0;
  Vector cross = Cross(a, b);

  return Normalize(&cross);
}

Vector TriComputeNormal( const Vector &N0, const Vector &N1, const Vector &N2,
    double u, double v)
{
  return (1 - u - v) * N0 + u * N1 + v * N2;
}

/* Codes from
 * Fast, minimum storage ray-triangle intersection.
 * Tomas Möller and Ben Trumbore.
 * Journal of Graphics Tools, 2(1):21--28, 1997.
 */
int TriRayIntersect(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const Vector &orig, const Vector &dir, int cull_backfaces,
    double *t, double *u, double *v)
{
  Vector edge1, edge2, tvec, pvec, qvec;
  double det, inv_det;

  /* find vectors for two edges sharing vert0 */
  edge1 = vert1 - vert0;
  edge2 = vert2 - vert0;

  /* begin calculating determinant - also used to calculate U parameter */
  pvec = Cross(dir, edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  det = Dot(edge1, pvec);

/*#ifdef TEST_CULL*/           /* define TEST_CULL if culling is desired */
  if (cull_backfaces) {
    if (det < EPSILON)
      return 0;

    /* calculate distance from vert0 to ray origin */
    tvec = orig - vert0;

    /* calculate U parameter and test bounds */
    *u = Dot(tvec, pvec);
    if (*u < 0.0 || *u > det)
      return 0;

    /* prepare to test V parameter */
    qvec = Cross(tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = Dot(dir, qvec);
    if (*v < 0.0 || *u + *v > det)
      return 0;

    /* calculate t, scale parameters, ray intersects triangle */
    *t = Dot(edge2, qvec);
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
    tvec = orig - vert0;

    /* calculate U parameter and test bounds */
    *u = Dot(tvec, pvec) * inv_det;
    if (*u < 0.0 || *u > 1.0)
      return 0;

    /* prepare to test V parameter */
    qvec = Cross(tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = Dot(dir, qvec) * inv_det;
    if (*v < 0.0 || *u + *v > 1.0)
      return 0;

    /* calculate t, ray intersects triangle */
    *t = Dot(edge2, qvec) * inv_det;
/*#endif*/
  }
  return 1;
}

void TriComputeDerivatives(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const TexCoord &tex0, const TexCoord &tex1, const TexCoord &tex2,
    Vector *dPdu, Vector *dPdv)
{
  const Vector dP1 = vert1 - vert0;
  const Vector dP2 = vert2 - vert0;

  const float du1 = tex1.u - tex0.u;
  const float du2 = tex2.u - tex0.u;
  const float dv1 = tex1.v - tex0.v;
  const float dv2 = tex2.v - tex0.v;

  const float determinant = du1 * dv2 - dv1 * du2;
  if (determinant == 0) {
    *dPdu = Vector(0, 0, 0);
    *dPdv = Vector(0, 0, 0);
    return;
  }

  const float invdet = 1. / determinant;
  *dPdu =  (dv2 * dP1 - dv1 * dP2) * invdet;
  *dPdv = (-du2 * dP1 + du1 * dP2) * invdet;
}

} // namespace xxx
