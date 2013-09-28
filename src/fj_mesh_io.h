/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_MESHIO_H
#define FJ_MESHIO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TexCoord;
struct Vector;
struct Color;
struct Mesh;

enum MshErrorNo {
  MSH_ERR_NONE = 0,
  MSH_ERR_FILE_NOT_EXIST,
  MSH_ERR_BAD_MAGIC_NUMBER,
  MSH_ERR_BAD_FILE_VERSION,
  MSH_ERR_LONG_ATTRIB_NAME,
  MSH_ERR_NO_MEMORY
};

struct MeshInput {
  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int nfaces;
  int nface_attrs;

  char *data_buffer;
  size_t buffer_size;

  char **attr_names;
};

struct MeshOutput {
  FILE *file;
  int version;
  int nverts;
  int nvert_attrs;
  int nfaces;
  int nface_attrs;

  struct Vector *P;
  struct Vector *N;
  struct Color *Cd;
  struct TexCoord *uv;
  struct TriIndex *indices;
};

/* mesh input file interfaces */
extern struct MeshInput *MshOpenInputFile(const char *filename);
extern void MshCloseInputFile(struct MeshInput *in);
extern int MshReadHeader(struct MeshInput *in);

/* mesh output file interfaces */
extern struct MeshOutput *MshOpenOutputFile(const char *filename);
extern void MshCloseOutputFile(struct MeshOutput *out);
extern void MshWriteFile(struct MeshOutput *out);

/* high level interface for loading mesh file */
extern int MshLoadFile(struct Mesh *mesh, const char *filename);

/* error no interfaces */
extern int MshGetErrorNo(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */

