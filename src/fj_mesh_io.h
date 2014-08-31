// Copyright (c) 2011-2014 Hiroshi Tsubokawa
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

  int GetVertexCount() const;
  int GetVertexAttributeCount() const;
  int GetFaceCount() const;
  int GetFaceAttributeCount() const;

  const char *GetDataBuffer() const;
  const std::string GetAttributeName(int i) const;

private:
  std::ifstream file_;

  int version_;
  int nverts_;
  int nvert_attrs_;
  int nfaces_;
  int nface_attrs_;

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

  void SetVertexCount(int count);
  void SetVertexPosition(const Vector *position);
  void SetVertexNormal(const Vector *normal);
  void SetVertexColor(const Color *color);
  void SetVertexTexture(const TexCoord *texcoord);
  void SetVertexVelocity(const Vector *vel);
  void SetFaceCount(int count);
  void SetFaceIndex3(const Index3 *index);
  void SetFaceGroupID(const int *id);

  void WriteFile();

private:
  void write_attribute_name(const std::string &name);
  void write_attribute_data(const std::string &name);

  std::ofstream file_;

  int version_;
  int nverts_;
  int nvert_attrs_;
  int nfaces_;
  int nface_attrs_;

  const Vector *P_;
  const Vector *N_;
  const Color *Cd_;
  const TexCoord *uv_;
  const Vector *velocity_;
  const Index3 *indices_;
  const int *face_group_id_;
};

// high level interface for loading mesh file
// TODO AUTOMATICALLY ADD AND COMPUTE N IF NOT EXIST
FJ_API int MshLoadFile(Mesh *mesh, const char *filename);

// error no interfaces
FJ_API int MshGetErrorNo(void);

} // namespace xxx

#endif // FJ_XXX_H
