/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_mesh.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_triangle.h"
#include "fj_memory.h"
#include "fj_ray.h"

// TODO REMOVE THIS
#include <string.h>

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Vertex, Vector,   P,        Position) \
  ATTR(Vertex, Vector,   N,        Normal) \
  ATTR(Vertex, Color,    Cd,       Color) \
  ATTR(Vertex, TexCoord, uv,       Texture) \
  ATTR(Vertex, Vector,   velocity, Velocity) \
  ATTR(Face,   Index3,   indices,  Indices)

namespace fj {

#define ATTR(Class, Type, Name, Label) \
void Mesh::Add##Class##Label() \
{ \
  Name.resize(Get##Class##Count()); \
} \
Type Mesh::Get##Class##Label(int idx) const \
{ \
  if (idx < 0 || idx >= static_cast<int>(Name.size())) { \
    return Type(); \
  } \
  return Name[idx]; \
} \
void Mesh::Set##Class##Label(int idx, const Type &value) \
{ \
  if (idx < 0 || idx >= static_cast<int>(Name.size())) \
    return; \
  Name[idx] = value; \
} \
bool Mesh::Has##Class##Label() const \
{ \
  return !Name.empty(); \
}
  ATTRIBUTE_LIST(ATTR)
#undef ATTR

void Mesh::Clear()
{
  nverts = 0;
  nfaces = 0;
  bounds = Box();

#define ATTR(Class, Type, Name, Label) std::vector<Type>().swap(Name);
  ATTRIBUTE_LIST(ATTR)
#undef ATTR
}

Mesh::Mesh() : nverts(0), nfaces(0), bounds()
{
}

Mesh::~Mesh()
{
}

int Mesh::GetVertexCount() const
{
  return nverts;
}

int Mesh::GetFaceCount() const
{
  return nfaces;
}

void Mesh::SetVertexCount(int count)
{
  nverts = count;
}

void Mesh::SetFaceCount(int count)
{
  nfaces = count;
}

const Box &Mesh::GetBounds() const
{
  return bounds;
}

void Mesh::ComputeNormals()
{
  if (P.empty() || indices.empty())
    return;

  const int nverts = GetVertexCount();
  const int nfaces = GetFaceCount();

  if (!HasVertexNormal()) {
    AddVertexNormal();
  }

  /* initialize N */
  for (int i = 0; i < nverts; i++) {
    N[i] = Vector(0, 0, 0);
  }

  /* compute N */
  for (int i = 0; i < nfaces; i++) {
    const Index3 &face = indices[i];

    const Vector &P0 = P[face.i0];
    const Vector &P1 = P[face.i1];
    const Vector &P2 = P[face.i2];

    const Vector Ng = TriComputeFaceNormal(P0, P1, P2);
    N[face.i0] += Ng;
    N[face.i1] += Ng;
    N[face.i2] += Ng;
  }

  /* normalize N */
  for (int i = 0; i < nverts; i++) {
    N[i] = Normalize(N[i]);
  }
}

void Mesh::ComputeBounds()
{
  BoxReverseInfinite(&bounds);

  for (int i = 0; i < GetFaceCount(); i++) {
    Box tri_bounds;
    GetPrimitiveBounds(i, &tri_bounds);
    BoxAddBox(&bounds, tri_bounds);
  }
}

bool Mesh::ray_intersect(Index prim_id, Real time,
    const Ray &ray, Intersection *isect) const
{
  const Index3 face = GetFaceIndices(prim_id);

  /* TODO make function */
  Vector P0 = GetVertexPosition(face.i0);
  Vector P1 = GetVertexPosition(face.i1);
  Vector P2 = GetVertexPosition(face.i2);

  if (HasVertexVelocity()) {
    const Vector velocity0 = GetVertexVelocity(face.i0);
    const Vector velocity1 = GetVertexVelocity(face.i1);
    const Vector velocity2 = GetVertexVelocity(face.i2);

    P0 += time * velocity0;
    P1 += time * velocity1;
    P2 += time * velocity2;
  }

  double u, v;
  double t_hit;
  const int hit = TriRayIntersect(
      P0, P1, P2,
      ray.orig, ray.dir, DO_NOT_CULL_BACKFACES,
      &t_hit, &u, &v);

  if (!hit)
    return false;

  if (isect == NULL)
    return true;

  /* we don't know N at time sampled point with velocity motion blur */
  /* just using N from mesh data */
  /* intersect info */
  const Vector N0 = GetVertexNormal(face.i0);
  const Vector N1 = GetVertexNormal(face.i1);
  const Vector N2 = GetVertexNormal(face.i2);
  isect->N = TriComputeNormal(N0, N1, N2, u, v);

  /* TODO TMP uv handling */
  /* UV = (1-u-v) * UV0 + u * UV1 + v * UV2 */
  if (HasVertexTexture()) {
    const float t = 1 - u - v;
    const TexCoord uv0 = GetVertexTexture(face.i0);
    const TexCoord uv1 = GetVertexTexture(face.i1);
    const TexCoord uv2 = GetVertexTexture(face.i2);
    isect->uv.u = t * uv0.u + u * uv1.u + v * uv2.u;
    isect->uv.v = t * uv0.v + u * uv1.v + v * uv2.v;

    TriComputeDerivatives(
        P0, P1, P2,
        uv0, uv1, uv2,
        &isect->dPdu, &isect->dPdv);
  }
  else {
    isect->uv.u = 0;
    isect->uv.v = 0;
    isect->dPdu = Vector(0, 0, 0);
    isect->dPdv = Vector(0, 0, 0);
  }

  isect->P = RayPointAt(ray, t_hit);
  isect->object = NULL;
  isect->prim_id = prim_id;
  isect->t_hit = t_hit;

  return true;
}

void Mesh::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  const Index3 face = GetFaceIndices(prim_id);

  const Vector P0 = GetVertexPosition(face.i0);
  const Vector P1 = GetVertexPosition(face.i1);
  const Vector P2 = GetVertexPosition(face.i2);

  TriComputeBounds(P0, P1, P2, bounds);

  if (HasVertexVelocity()) {
    const Vector velocity0 = GetVertexVelocity(face.i0);
    const Vector velocity1 = GetVertexVelocity(face.i1);
    const Vector velocity2 = GetVertexVelocity(face.i2);

    const Vector P0_close = P0 + velocity0;
    const Vector P1_close = P1 + velocity1;
    const Vector P2_close = P2 + velocity2;

    BoxAddPoint(bounds, P0_close);
    BoxAddPoint(bounds, P1_close);
    BoxAddPoint(bounds, P2_close);
  }
}

void Mesh::get_bounds(Box *bounds) const
{
  *bounds = GetBounds();
}

Index Mesh::get_primitive_count() const
{
  return GetFaceCount();
}

Mesh *MshNew(void)
{
  return new Mesh();
}

void MshFree(Mesh *mesh)
{
  delete mesh;
}

void MshGetFaceVertexPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2)
{
  const Index3 face = mesh->GetFaceIndices(face_index);

  *P0 = mesh->GetVertexPosition(face.i0);
  *P1 = mesh->GetVertexPosition(face.i1);
  *P2 = mesh->GetVertexPosition(face.i2);
}

void MshGetFaceVertexNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2)
{
  const Index3 face = mesh->GetFaceIndices(face_index);

  *N0 = mesh->GetVertexNormal(face.i0);
  *N1 = mesh->GetVertexNormal(face.i1);
  *N2 = mesh->GetVertexNormal(face.i2);
}

} // namespace xxx
