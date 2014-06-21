// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_MESHIO_H
#define FJ_MESHIO_H

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

class MeshInput {
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

class MeshOutput {
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
};

// mesh input file interfaces
extern MeshInput *MshOpenInputFile(const char *filename);
extern void MshCloseInputFile(MeshInput *in);
extern int MshReadHeader(MeshInput *in);

// mesh output file interfaces
extern MeshOutput *MshOpenOutputFile(const char *filename);
extern void MshCloseOutputFile(MeshOutput *out);
extern void MshWriteFile(MeshOutput *out);

// high level interface for loading mesh file
// TODO AUTOMATICALLY ADD AND COMPUTE N IF NOT EXIST
extern int MshLoadFile(Mesh *mesh, const char *filename);

// error no interfaces
extern int MshGetErrorNo(void);

} // namespace xxx

#endif // FJ_XXX_H
