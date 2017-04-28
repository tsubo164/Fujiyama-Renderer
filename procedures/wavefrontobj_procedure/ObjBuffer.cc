// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "ObjBuffer.h"

int ObjBufferToMesh(const ObjBuffer &buffer, Mesh &mesh)
{
  const int vertex_count = buffer.vertex_count;
  const int face_count = buffer.face_count;
  const std::vector<Vector> &P = buffer.vertex_position;
  const std::vector<Vector> &Np = buffer.point_normal;
  const std::vector<Vector> &Nv = buffer.vertex_normal;
  const std::vector<Index3> &indices = buffer.position_indices;
  const std::vector<int> &face_group_id = buffer.face_group_id;

  // P
  mesh.SetPointCount(vertex_count);
  mesh.AddPointPosition();
  for (int i = 0; i < vertex_count; i++) {
    mesh.SetPointPosition(i, P[i]);
  }
  // indices
  mesh.SetFaceCount(face_count);
  mesh.AddFaceIndices();
  for (int i = 0; i < face_count; i++) {
    mesh.SetFaceIndices(i, indices[i]);
  }

  // N
  if (!Np.empty()) {
    mesh.AddPointNormal();
    for (int i = 0; i < vertex_count; i++) {
      mesh.SetPointNormal(i, Np[i]);
    }
  } else if (!Nv.empty()) {
    // vertex normal values
    Mesh::VertexAttributeAccessor<Vector> vertex_normal = mesh.GetVertexNormal();
    Index value_count = buffer.vertex_normal.size();
    vertex_normal.ResizeValue(value_count);
    for (Index i = 0; i < value_count; i++) {
      vertex_normal.SetValue(i, Nv[i]);
    }
    // vertex normal indices
    Index index_count = buffer.normal_indices.size();
    vertex_normal.ResizeIndex(index_count * 3);
    for (Index i = 0; i < index_count; i++) {
      vertex_normal.SetIndex(i * 3 + 0, buffer.normal_indices[i][0]);
      vertex_normal.SetIndex(i * 3 + 1, buffer.normal_indices[i][1]);
      vertex_normal.SetIndex(i * 3 + 2, buffer.normal_indices[i][2]);
    }
  }

  // face group id
  mesh.AddFaceGroupID();
  for (int i = 0; i < face_count; i++) {
    mesh.SetFaceGroupID(i, face_group_id[i]);
  }

  // flatten group names
  std::vector<std::string> group_names(buffer.group_name_to_id.size());
  for (std::map<std::string,int>::const_iterator it = buffer.group_name_to_id.begin();
      it != buffer.group_name_to_id.end();
      ++it) {
    const int id = it->second;
    group_names[id] = it->first;
  }

  // set group names
  if (!group_names.empty()) {
    for (std::size_t i = 0; i < group_names.size(); i++) {
      mesh.CreateFaceGroup(group_names[i]);
    }
  }

  mesh.ComputeBounds();

  return 0;
}

int ObjBufferComputeNormals(ObjBuffer &buffer)
{
  const int vertex_count = buffer.vertex_count;
  const int face_count = buffer.face_count;
  std::vector<Vector> &P = buffer.vertex_position;
  std::vector<Vector> &N = buffer.point_normal;
  std::vector<Index3> &indices = buffer.position_indices;

  if (P.empty() || indices.empty()) {
    return -1;
  }

  if (!buffer.vertex_normal.empty()) {
    return 0;
  }

  N.resize(vertex_count);

  // accumulate N
  for (int i = 0; i < face_count; i++) {
    const int i0 = indices[i].i0;
    const int i1 = indices[i].i1;
    const int i2 = indices[i].i2;

    const Vector Ng = TriComputeFaceNormal(P[i0], P[i1], P[i2]);

    N[i0] += Ng;
    N[i1] += Ng;
    N[i2] += Ng;
  }

  // normalize N
  for (int i = 0; i < vertex_count; i++) {
    N[i] = Normalize(N[i]);
  }

  return 0;
}
