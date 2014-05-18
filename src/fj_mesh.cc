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
  ATTR(Face,   TriIndex, indices,  Indices)

namespace fj {

static int triangle_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Intersection *isect);
static void triangle_bounds(const void *prim_set, int prim_id, Box *bounds);
static void triangleset_bounds(const void *prim_set, Box *bounds);
static int triangle_count(const void *prim_set);

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
    const TriIndex &face = indices[i];

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
    triangle_bounds(this, i, &tri_bounds);
    BoxAddBox(&bounds, tri_bounds);
  }
}

void Mesh::Clear()
{
  nverts = 0;
  nfaces = 0;
  bounds = Box();

#define ATTR(Class, Type, Name, Label) std::vector<Type>().swap(Name);
  ATTRIBUTE_LIST(ATTR)
#undef ATTR
}

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

Mesh *MshNew(void)
{
  return new Mesh();
}

void MshFree(Mesh *mesh)
{
  delete mesh;
}

void MshClear(Mesh *mesh)
{
  mesh->Clear();
}

void MshComputeBounds(Mesh *mesh)
{
  mesh->ComputeBounds();
}

void MshComputeNormals(Mesh *mesh)
{
  mesh->ComputeNormals();
}

void MshAllocateVertex(Mesh *mesh, const char *attr_name, int nverts)
{
  mesh->SetVertexCount(nverts);

  if (strcmp(attr_name, "P") == 0) {
    mesh->AddVertexPosition();
  }
  else if (strcmp(attr_name, "N") == 0) {
    mesh->AddVertexNormal();
  }
  else if (strcmp(attr_name, "Cd") == 0) {
    mesh->AddVertexColor();
  }
  else if (strcmp(attr_name, "uv") == 0) {
    mesh->AddVertexTexture();
  }
  else if (strcmp(attr_name, "velocity") == 0) {
    mesh->AddVertexVelocity();
  }
}

void MshAllocateFace(Mesh *mesh, const char *attr_name, int nfaces)
{
  mesh->SetFaceCount(nfaces);

  if (strcmp(attr_name, "indices") == 0) {
    mesh->AddFaceIndices();
  }
}

void MshGetFaceVertexPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2)
{
  const TriIndex face = mesh->GetFaceIndices(face_index);

  *P0 = mesh->GetVertexPosition(face.i0);
  *P1 = mesh->GetVertexPosition(face.i1);
  *P2 = mesh->GetVertexPosition(face.i2);
}

void MshGetFaceVertexNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2)
{
  const TriIndex face = mesh->GetFaceIndices(face_index);

  *N0 = mesh->GetVertexNormal(face.i0);
  *N1 = mesh->GetVertexNormal(face.i1);
  *N2 = mesh->GetVertexNormal(face.i2);
}

void MshSetVertexPosition(Mesh *mesh, int index, const Vector *P)
{
  mesh->SetVertexPosition(index, *P);
}

void MshSetVertexNormal(Mesh *mesh, int index, const Vector *N)
{
  mesh->SetVertexNormal(index, *N);
}

void MshSetVertexColor(Mesh *mesh, int index, const Color *Cd)
{
  mesh->SetVertexColor(index, *Cd);
}

void MshSetVertexTexture(Mesh *mesh, int index, const TexCoord *uv)
{
  mesh->SetVertexTexture(index, *uv);
}

void MshSetVertexVelocity(Mesh *mesh, int index, const Vector *velocity)
{
  mesh->SetVertexVelocity(index, *velocity);
}

void MshSetFaceVertexIndices(Mesh *mesh, int face_index,
    const TriIndex *tri_index)
{
  mesh->SetFaceIndices(face_index, *tri_index);
}

void MshGetVertexPosition(const Mesh *mesh, int index, Vector *P)
{
  *P = mesh->GetVertexPosition(index);
}

void MshGetVertexNormal(const Mesh *mesh, int index, Vector *N)
{
  *N = mesh->GetVertexNormal(index);
}

void MshGetFaceVertexIndices(const Mesh *mesh, int face_index,
    TriIndex *tri_index)
{
  *tri_index = mesh->GetFaceIndices(face_index);
}

int MshGetVertexCount(const Mesh *mesh)
{
  return mesh->GetVertexCount();
}

int MshGetFaceCount(const Mesh *mesh)
{
  return mesh->GetFaceCount();
}

void MshGetPrimitiveSet(const Mesh *mesh, PrimitiveSet *primset)
{
  MakePrimitiveSet(primset,
      "Mesh",
      mesh,
      triangle_ray_intersect,
      triangle_bounds,
      triangleset_bounds,
      triangle_count);
}

static int triangle_ray_intersect(const void *prim_set, int prim_id, double time,
    const Ray *ray, Intersection *isect)
{
  const Mesh *mesh = (const Mesh *) prim_set;
  const TriIndex face = mesh->GetFaceIndices(prim_id);

  /* TODO make function */
  Vector P0 = mesh->GetVertexPosition(face.i0);
  Vector P1 = mesh->GetVertexPosition(face.i1);
  Vector P2 = mesh->GetVertexPosition(face.i2);

  if (mesh->HasVertexVelocity()) {
    const Vector velocity0 = mesh->GetVertexVelocity(face.i0);
    const Vector velocity1 = mesh->GetVertexVelocity(face.i1);
    const Vector velocity2 = mesh->GetVertexVelocity(face.i2);

    P0 += time * velocity0;
    P1 += time * velocity1;
    P2 += time * velocity2;
  }

  double u, v;
  double t_hit;
  const int hit = TriRayIntersect(
      P0, P1, P2,
      ray->orig, ray->dir, DO_NOT_CULL_BACKFACES,
      &t_hit, &u, &v);

  if (!hit)
    return 0;

  if (isect == NULL)
    return 1;

  /* we don't know N at time sampled point with velocity motion blur */
  /* just using N from mesh data */
  /* intersect info */
  const Vector N0 = mesh->GetVertexNormal(face.i0);
  const Vector N1 = mesh->GetVertexNormal(face.i1);
  const Vector N2 = mesh->GetVertexNormal(face.i2);
  isect->N = TriComputeNormal(N0, N1, N2, u, v);

  /* TODO TMP uv handling */
  /* UV = (1-u-v) * UV0 + u * UV1 + v * UV2 */
  if (mesh->HasVertexTexture()) {
    const float t = 1 - u - v;
    const TexCoord uv0 = mesh->GetVertexTexture(face.i0);
    const TexCoord uv1 = mesh->GetVertexTexture(face.i1);
    const TexCoord uv2 = mesh->GetVertexTexture(face.i2);
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

  isect->P = RayPointAt(*ray, t_hit);
  isect->object = NULL;
  isect->prim_id = prim_id;
  isect->t_hit = t_hit;

  return 1;
}

static void triangle_bounds(const void *prim_set, int prim_id, Box *bounds)
{
  const Mesh *mesh = (const Mesh *) prim_set;
  const TriIndex face = mesh->GetFaceIndices(prim_id);

  const Vector P0 = mesh->GetVertexPosition(face.i0);
  const Vector P1 = mesh->GetVertexPosition(face.i1);
  const Vector P2 = mesh->GetVertexPosition(face.i2);

  TriComputeBounds(P0, P1, P2, bounds);

  if (mesh->HasVertexVelocity()) {
    const Vector velocity0 = mesh->GetVertexVelocity(face.i0);
    const Vector velocity1 = mesh->GetVertexVelocity(face.i1);
    const Vector velocity2 = mesh->GetVertexVelocity(face.i2);

    const Vector P0_close = P0 + velocity0;
    const Vector P1_close = P1 + velocity1;
    const Vector P2_close = P2 + velocity2;

    BoxAddPoint(bounds, P0_close);
    BoxAddPoint(bounds, P1_close);
    BoxAddPoint(bounds, P2_close);
  }
}

static void triangleset_bounds(const void *prim_set, Box *bounds)
{
  const Mesh *mesh = (const Mesh *) prim_set;
  *bounds = mesh->GetBounds();
}

static int triangle_count(const void *prim_set)
{
  const Mesh *mesh = (const Mesh *) prim_set;
  return mesh->GetFaceCount();
}

} // namespace xxx
