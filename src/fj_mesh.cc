// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_mesh.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_triangle.h"
#include "fj_ray.h"

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Vertex, Vector,   P_,        Position) \
  ATTR(Vertex, Vector,   N_,        Normal) \
  ATTR(Vertex, Color,    Cd_,       Color) \
  ATTR(Vertex, TexCoord, uv_,       Texture) \
  ATTR(Vertex, Vector,   velocity_, Velocity) \
  ATTR(Face,   Index3,   indices_,  Indices) \
  ATTR(Face,   int,      face_group_id_,  GroupID)

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
  nverts_ = 0;
  nfaces_ = 0;
  bounds_ = Box();

#define ATTR(Class, Type, Name, Label) std::vector<Type>().swap(Name);
  ATTRIBUTE_LIST(ATTR)
#undef ATTR
}

Mesh::Mesh() : nverts_(0), nfaces_(0), bounds_()
{
  face_group_name_[""] = 0;
}

Mesh::~Mesh()
{
}

int Mesh::GetVertexCount() const
{
  return nverts_;
}

int Mesh::GetFaceCount() const
{
  return nfaces_;
}

void Mesh::SetVertexCount(int count)
{
  nverts_ = count;
}

void Mesh::SetFaceCount(int count)
{
  nfaces_ = count;
}

const Box &Mesh::GetBounds() const
{
  return bounds_;
}

int Mesh::CreateFaceGroup(const std::string &group_name)
{
  std::map<std::string, int>::const_iterator it = face_group_name_.find(group_name);
  if (it != face_group_name_.end()) {
    return it->second;
  }

  const int new_id = static_cast<int>(face_group_name_.size());
  face_group_name_[group_name] = new_id;

  return 0;
}

void Mesh::ComputeNormals()
{
  if (!HasVertexPosition() || !HasFaceIndices())
    return;

  const int nverts = GetVertexCount();
  const int nfaces = GetFaceCount();

  if (!HasVertexNormal()) {
    AddVertexNormal();
  }

  // initialize N
  for (int i = 0; i < nverts; i++) {
    SetVertexNormal(i, Vector(0, 0, 0));
  }

  // compute N
  for (int i = 0; i < nfaces; i++) {
    const Index3 face = GetFaceIndices(i);

    const Vector P0 = GetVertexPosition(face.i0);
    const Vector P1 = GetVertexPosition(face.i1);
    const Vector P2 = GetVertexPosition(face.i2);

    const Vector Ng = TriComputeFaceNormal(P0, P1, P2);
    const Vector N0 = GetVertexNormal(face.i0) + Ng;
    const Vector N1 = GetVertexNormal(face.i1) + Ng;
    const Vector N2 = GetVertexNormal(face.i2) + Ng;

    SetVertexNormal(face.i0, N0);
    SetVertexNormal(face.i1, N1);
    SetVertexNormal(face.i2, N2);
  }

  // normalize N
  for (int i = 0; i < nverts; i++) {
    Vector N = GetVertexNormal(i);
    SetVertexNormal(i, Normalize(&N));
  }
}

void Mesh::ComputeBounds()
{
  BoxReverseInfinite(&bounds_);

  for (int i = 0; i < GetFaceCount(); i++) {
    Box tri_bounds;
    GetPrimitiveBounds(i, &tri_bounds);
    BoxAddBox(&bounds_, tri_bounds);
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

  // we don't know N at time sampled point with velocity motion blur
  // just using N from mesh data
  // intersect info
  const Vector N0 = GetVertexNormal(face.i0);
  const Vector N1 = GetVertexNormal(face.i1);
  const Vector N2 = GetVertexNormal(face.i2);
  isect->N = TriComputeNormal(N0, N1, N2, u, v);

  // TODO TMP uv handling
  // UV = (1-u-v) * UV0 + u * UV1 + v * UV2
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
  isect->shading_group_id = GetFaceGroupID(prim_id);
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
