// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_TRIANGLE_H
#define FJ_TRIANGLE_H

#include "fj_compatibility.h"

namespace fj {

enum {
  DO_NOT_CULL_BACKFACES = 0,
  CULL_BACKFACES = 1
};

class TexCoord;
class Vector;
class Box;

FJ_API double TriComputeArea(const Vector &vert0, const Vector &vert1, const Vector &vert2);

FJ_API void TriComputeBounds(const Vector &vert0, const Vector &vert1, const Vector &vert2,
    Box *box);

FJ_API Vector TriComputeFaceNormal(
    const Vector &vert0, const Vector &vert1, const Vector &vert2);

FJ_API Vector TriComputeNormal( const Vector &N0, const Vector &N1, const Vector &N2,
    double u, double v);

FJ_API int TriRayIntersect(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const Vector &orig, const Vector &dir, int cull_backfaces,
    double *t, double *u, double *v);

FJ_API void TriComputeDerivatives(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const TexCoord &tex0, const TexCoord &tex1, const TexCoord &tex2,
    Vector *dPdu, Vector *dPdv);

} // namespace xxx

#endif // FJ_XXX_H
