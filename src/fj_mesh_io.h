// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_MESHIO_H
#define FJ_MESHIO_H

#include "fj_compatibility.h"
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>

namespace fj {

class TexCoord;
class Index3;
class Vector;
class Color;
class Mesh;

enum MshErrorNo {
  MSH_ERR_NONE = 0,
  MSH_ERR_FILE_NOT_EXIST,
  MSH_ERR_BAD_MAGIC_NUMBER,
  MSH_ERR_BAD_FILE_VERSION,
  MSH_ERR_LONG_ATTRIB_NAME,
  MSH_ERR_NO_MEMORY
};

class FJ_API MeshInput {
public:
  MeshInput();
  ~MeshInput();

  int Open(const std::string &filename);
  void Close();
  bool Fail() const;

  int ReadHeader();
  void ReadAttributeData();

  int GetVertexAttributeCount() const;
  int GetPointCount() const;
  int GetPointAttributeCount() const;
  int GetFaceCount() const;
  int GetFaceAttributeCount() const;

  int GetFaceGroupCount() const;

  const char *GetDataBuffer() const;
  const std::string GetAttributeName(int i) const;

private:
  std::ifstream file_;

  int version_;
  int vertex_attr_count_;
  int point_count_;
  int point_attr_count_;
  int face_count_;
  int face_attr_count_;

  int face_group_count_;

  std::vector<char> data_buffer_;
  std::vector<std::string> attr_names_;
};

class FJ_API MeshOutput {
public:
  MeshOutput();
  ~MeshOutput();

  int Open(const std::string &filename);
  void Close();
  bool Fail() const;

  void SetPointCount(int count);
  void SetPointPosition(const Vector *position);
  void SetPointNormal(const Vector *normal);
  void SetPointColor(const Color *color);
  void SetPointTexture(const TexCoord *texcoord);
  void SetPointVelocity(const Vector *vel);
  void SetFaceCount(int count);
  void SetFaceIndex3(const Index3 *index);
  void SetFaceGroupID(const int *id);

  void SetVertexNormal(
      const Vector *value, int value_count,
      const Index3 *index, int index_count);
  void SetFaceGroupName(const std::string *name);
  void SetFaceGroupNameCount(int count);

  void WriteFile();

private:
  void write_attribute_name(const std::string &name);
  void write_attribute_data(const std::string &name);

  std::ofstream file_;

  int version_;
  int vertex_attr_count_;
  int point_count_;
  int point_attr_count_;
  int face_count_;
  int face_attr_count_;

  int face_group_count_;

  const Vector *P_;
  const Vector *N_;
  const Color *Cd_;
  const TexCoord *uv_;
  const Vector *velocity_;
  const Index3 *indices_;
  const int *face_group_id_;

  const std::string *face_group_name_;

  const Vector *vertex_normal_value_;
  const Index3 *vertex_normal_index_;
  int vertex_normal_value_count_;
  int vertex_normal_index_count_;
};

// high level interface for loading mesh file
// TODO AUTOMATICALLY ADD AND COMPUTE N IF NOT EXIST
FJ_API int MshLoadFile(Mesh *mesh, const char *filename);

// error no interfaces
FJ_API int MshGetErrorNo(void);

} // namespace xxx

#endif // FJ_XXX_H
