// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_triangle.h"
#include "fj_tex_coord.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_box.h"

namespace fj {

static const Real EPSILON = 1e-6;

Real TriComputeArea(
    const Vector &vert0, const Vector &vert1, const Vector &vert2)
{
  const Vector a = vert1 - vert0;
  const Vector b = vert2 - vert0;
  const Vector cross = Cross(a, b);

  return .5 * Length(cross);
}

void TriComputeBounds(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    Box *box)
{
  box->ReverseInfinite();

  box->AddPoint(vert0);
  box->AddPoint(vert1);
  box->AddPoint(vert2);
}

Vector TriComputeFaceNormal(
    const Vector &vert0, const Vector &vert1, const Vector &vert2)
{
  const Vector a = vert1 - vert0;
  const Vector b = vert2 - vert0;

  return GetNormalized(Cross(a, b));
}

Vector TriComputeNormal(
    const Vector &N0, const Vector &N1, const Vector &N2,
    Real u, Real v)
{
  return (1 - u - v) * N0 + u * N1 + v * N2;
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

/* Codes from
 * Fast, minimum storage ray-triangle intersection.
 * Tomas Möller and Ben Trumbore.
 * Journal of Graphics Tools, 2(1):21--28, 1997.
 */
bool TriRayIntersect(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const Vector &orig, const Vector &dir, int cull_backfaces,
    Real *t, Real *u, Real *v)
{
  Vector edge1, edge2, tvec, pvec, qvec;
  Real det, inv_det;

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
      return false;

    /* calculate distance from vert0 to ray origin */
    tvec = orig - vert0;

    /* calculate U parameter and test bounds */
    *u = Dot(tvec, pvec);
    if (*u < 0.0 || *u > det)
      return false;

    /* prepare to test V parameter */
    qvec = Cross(tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = Dot(dir, qvec);
    if (*v < 0.0 || *u + *v > det)
      return false;

    /* calculate t, scale parameters, ray intersects triangle */
    *t = Dot(edge2, qvec);
    inv_det = 1.0 / det;
    *t *= inv_det;
    *u *= inv_det;
    *v *= inv_det;
/*#else*/                    /* the non-culling branch */
  } else {
    if (det > -EPSILON && det < EPSILON)
      return false;
    inv_det = 1.0 / det;

    /* calculate distance from vert0 to ray origin */
    tvec = orig - vert0;

    /* calculate U parameter and test bounds */
    *u = Dot(tvec, pvec) * inv_det;
    if (*u < 0.0 || *u > 1.0)
      return false;

    /* prepare to test V parameter */
    qvec = Cross(tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = Dot(dir, qvec) * inv_det;
    if (*v < 0.0 || *u + *v > 1.0)
      return false;

    /* calculate t, ray intersects triangle */
    *t = Dot(edge2, qvec) * inv_det;
/*#endif*/
  }
  return true;
}

/* Codes from
 * Fast 3D Triangle-Box Overlap Testing
 * Tomas Akenine-Möller
 * SIGGRAPH '05 ACM SIGGRAPH 2005 Courses
 */
static bool plane_box_overlap(
    const Vector &normal, const Vector &vert, const Vector &maxbox)
{
  Vector vmin, vmax;

  for (int i = 0; i <= 2; i++) {
    const Real v = vert[i];

    if (normal[i] > 0.) {
      vmin[i] = -maxbox[i] - v;
      vmax[i] =  maxbox[i] - v;
    }
    else {
      vmin[i] =  maxbox[i] - v;
      vmax[i] = -maxbox[i] - v;
    }
  }

  if (Dot(normal, vmin) > 0.) {
    return false;
  }

  if (Dot(normal, vmax) >= 0.) {
    return true;
  }

  return false;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb) \
  p0 = a*v0[1] - b*v0[2]; \
  p2 = a*v2[1] - b*v2[2]; \
  if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
  rad = fa * boxhalfsize[1] + fb * boxhalfsize[2]; \
  if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb) \
  p0 = a*v0[1] - b*v0[2]; \
  p1 = a*v1[1] - b*v1[2]; \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
  rad = fa * boxhalfsize[1] + fb * boxhalfsize[2]; \
  if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb) \
  p0 = -a*v0[0] + b*v0[2]; \
  p2 = -a*v2[0] + b*v2[2]; \
  if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[2]; \
  if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb) \
  p0 = -a*v0[0] + b*v0[2]; \
  p1 = -a*v1[0] + b*v1[2]; \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[2]; \
  if(min>rad || max<-rad) return false;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb) \
  p1 = a*v1[0] - b*v1[1]; \
  p2 = a*v2[0] - b*v2[1]; \
  if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[1]; \
  if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb) \
  p0 = a*v0[0] - b*v0[1]; \
  p1 = a*v1[0] - b*v1[1]; \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[1]; \
  if(min>rad || max<-rad) return false;

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0; \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

bool TriBoxIntersect(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const Vector &boxcenter, const Vector &boxhalfsize)
{
  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   Real min, max, p0, p1, p2, rad, fex, fey, fez;

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   const Vector v0 = vert0 - boxcenter;
   const Vector v1 = vert1 - boxcenter;
   const Vector v2 = vert2 - boxcenter;

   /* compute triangle edges */
   const Vector e0 = v1 - v0;      /* tri edge 0 */
   const Vector e1 = v2 - v1;      /* tri edge 1 */
   const Vector e2 = v0 - v2;      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = Abs(e0[0]);
   fey = Abs(e0[1]);
   fez = Abs(e0[2]);

   AXISTEST_X01(e0[2], e0[1], fez, fey);
   AXISTEST_Y02(e0[2], e0[0], fez, fex);
   AXISTEST_Z12(e0[1], e0[0], fey, fex);

   fex = Abs(e1[0]);
   fey = Abs(e1[1]);
   fez = Abs(e1[2]);

   AXISTEST_X01(e1[2], e1[1], fez, fey);
   AXISTEST_Y02(e1[2], e1[0], fez, fex);
   AXISTEST_Z0(e1[1], e1[0], fey, fex);

   fex = Abs(e2[0]);
   fey = Abs(e2[1]);
   fez = Abs(e2[2]);

   AXISTEST_X2(e2[2], e2[1], fez, fey);
   AXISTEST_Y1(e2[2], e2[0], fez, fex);
   AXISTEST_Z12(e2[1], e2[0], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[0], v1[0], v2[0], min, max);
   if (min > boxhalfsize[0] || max < -boxhalfsize[0])
     return false;

   /* test in Y-direction */
   FINDMINMAX(v0[1], v1[1], v2[1], min, max);
   if (min > boxhalfsize[1] || max < -boxhalfsize[1])
     return false;

   /* test in Z-direction */
   FINDMINMAX(v0[2], v1[2], v2[2], min, max);
   if (min > boxhalfsize[2] || max < -boxhalfsize[2])
     return false;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   const Vector normal = Cross(e0, e1);

   if (!plane_box_overlap(normal, v0, boxhalfsize))
     return false;

   return true;   /* box and triangle overlaps */
}

} // namespace xxx
