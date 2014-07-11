// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_MESHIO_H
#define FJ_MESHIO_H

#include "fj_compatibility.h"
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
  MeshInput() {}
  ~MeshInput() {}

public:
  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int nfaces;
  int nface_attrs;

  std::vector<char> data_buffer;

  std::vector<std::string> attr_names;
};

class FJ_API MeshOutput {
public:
  MeshOutput() {}
  ~MeshOutput() {}

public:
  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int nfaces;
  int nface_attrs;

  Vector *P;
  Vector *N;
  Color *Cd;
  TexCoord *uv;
  Vector *velocity;
  Index3 *indices;
  int *face_group_id;
};

// mesh input file interfaces
FJ_API MeshInput *MshOpenInputFile(const char *filename);
FJ_API void MshCloseInputFile(MeshInput *in);
FJ_API int MshReadHeader(MeshInput *in);

// mesh output file interfaces
FJ_API MeshOutput *MshOpenOutputFile(const char *filename);
FJ_API void MshCloseOutputFile(MeshOutput *out);
FJ_API void MshWriteFile(MeshOutput *out);

// high level interface for loading mesh file
// TODO AUTOMATICALLY ADD AND COMPUTE N IF NOT EXIST
FJ_API int MshLoadFile(Mesh *mesh, const char *filename);

// error no interfaces
FJ_API int MshGetErrorNo(void);

} // namespace xxx

#endif // FJ_XXX_H
