// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_mesh.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_triangle.h"
#include "fj_ray.h"

#define ATTRIBUTE_LIST(ATTR) \
  ATTR(Point, Vector,   P_,        Position) \
  ATTR(Point, Vector,   N_,        Normal) \
  ATTR(Point, Color,    Cd_,       Color) \
  ATTR(Point, TexCoord, uv_,       Texture) \
  ATTR(Point, Vector,   velocity_, Velocity) \
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
  point_count_ = 0;
  face_count_ = 0;
  bounds_ = Box();

#define ATTR(Class, Type, Name, Label) std::vector<Type>().swap(Name);
  ATTRIBUTE_LIST(ATTR)
#undef ATTR
}

static void get_point_positions(const Mesh &mesh, Index face_index,
    Vector &P0, Vector &P1, Vector &P2)
{
  const Index3 face = mesh.GetFaceIndices(face_index);

  P0 = mesh.GetPointPosition(face.i0);
  P1 = mesh.GetPointPosition(face.i1);
  P2 = mesh.GetPointPosition(face.i2);
}

static void get_point_normals(const Mesh &mesh, Index face_index,
    Vector &N0, Vector &N1, Vector &N2)
{
  const Index3 face = mesh.GetFaceIndices(face_index);

  N0 = mesh.GetPointNormal(face.i0);
  N1 = mesh.GetPointNormal(face.i1);
  N2 = mesh.GetPointNormal(face.i2);
}

static void get_point_texture(const Mesh &mesh, Index face_index,
    TexCoord &T0, TexCoord &T1, TexCoord &T2)
{
  const Index3 face = mesh.GetFaceIndices(face_index);

  T0 = mesh.GetPointTexture(face.i0);
  T1 = mesh.GetPointTexture(face.i1);
  T2 = mesh.GetPointTexture(face.i2);
}

static void get_point_velocity(const Mesh &mesh, Index face_index,
    Vector &V0, Vector &V1, Vector &V2)
{
  const Index3 face = mesh.GetFaceIndices(face_index);

  V0 = mesh.GetPointVelocity(face.i0);
  V1 = mesh.GetPointVelocity(face.i1);
  V2 = mesh.GetPointVelocity(face.i2);
}

static void get_vertex_normals(const Mesh &mesh, Index face_index,
    Vector &N0, Vector &N1, Vector &N2)
{
  N0 = mesh.GetVertexNormal(3 * face_index + 0);
  N1 = mesh.GetVertexNormal(3 * face_index + 1);
  N2 = mesh.GetVertexNormal(3 * face_index + 2);
}

static Vector compute_shading_normal(const Mesh &mesh, Index face_index, double u, double v)
{
  Vector N0, N1, N2;

  if (mesh.HasVertexNormal()) {
    get_vertex_normals(mesh, face_index, N0, N1, N2);
  }
  else {
    get_point_normals(mesh, face_index, N0, N1, N2);
  }

  return TriComputeNormal(N0, N1, N2, u, v);
}

Mesh::Mesh() : point_count_(0), face_count_(0), bounds_()
{
  face_group_name_[""] = 0;
}

Mesh::~Mesh()
{
}

int Mesh::GetPointCount() const
{
  return point_count_;
}

int Mesh::GetFaceCount() const
{
  return face_count_;
}

void Mesh::SetPointCount(int count)
{
  point_count_ = count;
}

void Mesh::SetFaceCount(int count)
{
  face_count_ = count;
}

const Box &Mesh::GetBounds() const
{
  return bounds_;
}

//TODO TEST
bool Mesh::HasVertexNormal() const
{
  return !vertex_normal_.IsEmpty();
}

Vector Mesh::GetVertexNormal(Index vertex_id) const
{
  if (HasVertexNormal()) {
    return vertex_normal_.Get(vertex_id);
  }
  else {
    return Vector(0, 0, 0);
  }
}

int Mesh::CreateFaceGroup(const std::string &group_name)
{
  std::map<std::string, int>::const_iterator it = face_group_name_.find(group_name);
  if (it != face_group_name_.end()) {
    return it->second;
  }

  const int new_id = static_cast<int>(face_group_name_.size());
  face_group_name_[group_name] = new_id;

  return new_id;
}

int Mesh::LookupFaceGroup(const std::string &group_name) const
{
  std::map<std::string, int>::const_iterator it = face_group_name_.find(group_name);
  if (it != face_group_name_.end()) {
    return it->second;
  } else {
    return -1;
  }
}

void Mesh::ComputeNormals()
{
  if (!HasPointPosition() || !HasFaceIndices())
    return;

  const int nverts = GetPointCount();
  const int nfaces = GetFaceCount();

  if (!HasPointNormal()) {
    AddPointNormal();
  }

  // initialize N
  for (int i = 0; i < nverts; i++) {
    SetPointNormal(i, Vector(0, 0, 0));
  }

  // compute N
  for (int i = 0; i < nfaces; i++) {
    Vector P0, P1, P2;
    get_point_positions(*this, i, P0, P1, P2);

    Vector N0, N1, N2;
    get_point_normals(*this, i, N0, N1, N2);

    const Index3 face = GetFaceIndices(i);
    const Vector Ng = TriComputeFaceNormal(P0, P1, P2);
    SetPointNormal(face.i0, N0 + Ng);
    SetPointNormal(face.i1, N1 + Ng);
    SetPointNormal(face.i2, N2 + Ng);
  }

  // normalize N
  for (int i = 0; i < nverts; i++) {
    Vector N = GetPointNormal(i);
    SetPointNormal(i, Normalize(&N));
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
  Vector P0, P1, P2;
  get_point_positions(*this, prim_id, P0, P1, P2);

  if (HasPointVelocity()) {
    Vector velocity0, velocity1, velocity2;
    get_point_velocity(*this, prim_id, velocity0, velocity1, velocity2);

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
  isect->N = compute_shading_normal(*this, prim_id, u, v);

  // TODO TMP uv handling
  // UV = (1-u-v) * UV0 + u * UV1 + v * UV2
  if (HasPointTexture()) {
    TexCoord uv0, uv1, uv2;
    get_point_texture(*this, prim_id, uv0, uv1, uv2);

    const float t = 1 - u - v;
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

bool Mesh::box_intersect(Index prim_id, const Box &box) const
{
  Vector P0, P1, P2;
  get_point_positions(*this, prim_id, P0, P1, P2);

  const Vector centroid = BoxCentroid(box);
  const Vector halfsize = .5 * BoxDiagonal(box);

  return TriBoxIntersect(P0, P1, P2, centroid, halfsize);
}

void Mesh::get_primitive_bounds(Index prim_id, Box *bounds) const
{
  Vector P0, P1, P2;
  get_point_positions(*this, prim_id, P0, P1, P2);

  TriComputeBounds(P0, P1, P2, bounds);

  if (HasPointVelocity()) {
    Vector velocity0, velocity1, velocity2;
    get_point_velocity(*this, prim_id, velocity0, velocity1, velocity2);

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

void MshGetFacePointPosition(const Mesh *mesh, int face_index,
    Vector *P0, Vector *P1, Vector *P2)
{
  get_point_positions(*mesh, face_index, *P0, *P1, *P2);
}

void MshGetFacePointNormal(const Mesh *mesh, int face_index,
    Vector *N0, Vector *N1, Vector *N2)
{
  get_point_normals(*mesh, face_index, *N0, *N1, *N2);
}

} // namespace xxx
