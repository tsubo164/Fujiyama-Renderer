/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_TRIANGLE_H
#define FJ_TRIANGLE_H

namespace fj {

enum {
  DO_NOT_CULL_BACKFACES = 0,
  CULL_BACKFACES = 1
};

struct TexCoord;
struct Vector;
struct Box;

extern double TriComputeArea(const Vector &vert0, const Vector &vert1, const Vector &vert2);

extern void TriComputeBounds(const Vector &vert0, const Vector &vert1, const Vector &vert2,
    Box *box);

extern Vector TriComputeFaceNormal(
    const Vector &vert0, const Vector &vert1, const Vector &vert2);

extern Vector TriComputeNormal( const Vector &N0, const Vector &N1, const Vector &N2,
    double u, double v);

extern int TriRayIntersect(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const Vector &orig, const Vector &dir, int cull_backfaces,
    double *t, double *u, double *v);

extern void TriComputeDerivatives(
    const Vector &vert0, const Vector &vert1, const Vector &vert2,
    const TexCoord &tex0, const TexCoord &tex1, const TexCoord &tex2,
    Vector *dPdu, Vector *dPdv);

} // namespace xxx

#endif /* FJ_XXX_H */
