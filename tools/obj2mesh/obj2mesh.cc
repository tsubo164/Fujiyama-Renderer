// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "ObjParser.h"
#include "fj_tex_coord.h"
#include "fj_triangle.h"
#include "fj_mesh_io.h"
#include "fj_vector.h"
#include "fj_array.h"
#include "fj_mesh.h"

#include <vector>
#include <cstdio>
#include <cstring>

using namespace fj;

static const char USAGE[] =
"Usage: obj2mesh [options] inputfile(*.obj) outputfile(*.mesh)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

class ObjBuffer {
public:
  ObjBuffer() : nverts(0), nfaces(0) {}
  ~ObjBuffer() {}

public:
  std::vector<Vector>   P;
  std::vector<Vector>   N;
  std::vector<TexCoord> uv;
  std::vector<Index3>   vertex_indices;
  std::vector<Index3>   texture_indices;
  std::vector<Index3>   normal_indices;

  long nverts;
  long nfaces;
};

extern int ObjBufferFromFile(ObjBuffer *buffer, const char *filename);
extern int ObjBufferToMeshFile(ObjBuffer *buffer, const char *filename);
extern int ObjBufferComputeNormals(ObjBuffer *buffer);

int main(int argc, const char **argv)
{
  const char *in_filename = NULL;
  const char *out_filename = NULL;
  int err = 0;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  in_filename = argv[1];
  out_filename = argv[2];

  ObjBuffer buffer;

  err = ObjBufferFromFile(&buffer, in_filename);
  if (err) {
    // TODO error handling
    return -1;
  }

  err = ObjBufferComputeNormals(&buffer);
  if (err) {
    // TODO error handling
    return -1;
  }

  err = ObjBufferToMeshFile(&buffer, out_filename);
  if (err) {
    // TODO error handling
    return -1;
  }

  printf("nverts: %ld\n", buffer.nverts);
  printf("nfaces: %ld\n", buffer.nfaces);

  return 0;
}

static int read_vertx(
    void *interpreter,
    int scanned_ncomponents,
    double x,
    double y,
    double z,
    double w)
{
  ObjBuffer *buffer = (ObjBuffer *) interpreter;
  Vector P;

  P.x = x;
  P.y = y;
  P.z = z;
  buffer->P.push_back(P);

  buffer->nverts++;
  return 0;
}

static int read_texture(
    void *interpreter,
    int scanned_ncomponents,
    double x,
    double y,
    double z,
    double w)
{
  ObjBuffer *buffer = (ObjBuffer *) interpreter;
  TexCoord uv;

  uv.u = x;
  uv.v = y;
  buffer->uv.push_back(uv);

  return 0;
}

static int read_normal(
    void *interpreter,
    int scanned_ncomponents,
    double x,
    double y,
    double z,
    double w)
{
  ObjBuffer *buffer = (ObjBuffer *) interpreter;
  Vector N;

  N.x = x;
  N.y = y;
  N.z = z;
  buffer->N.push_back(N);

  return 0;
}

static int read_face(
    void *interpreter,
    long index_count,
    const long *vertex_indices,
    const long *texture_indices,
    const long *normal_indices)
{
  ObjBuffer *buffer = (ObjBuffer *) interpreter;
  int i;

  const int ntriangles = index_count - 2;

  for (i = 0; i < ntriangles; i++) {
    if (vertex_indices != NULL) {
      Index3 tri_index;
      tri_index.i0 = vertex_indices[0] - 1;
      tri_index.i1 = vertex_indices[i + 1] - 1;
      tri_index.i2 = vertex_indices[i + 2] - 1;
      buffer->vertex_indices.push_back(tri_index);
    }

    if (texture_indices != NULL) {
      Index3 tri_index;
      tri_index.i0 = texture_indices[0] - 1;
      tri_index.i1 = texture_indices[i + 1] - 1;
      tri_index.i2 = texture_indices[i + 2] - 1;
      buffer->texture_indices.push_back(tri_index);
    }

    if (normal_indices != NULL) {
      Index3 tri_index;
      tri_index.i0 = normal_indices[0] - 1;
      tri_index.i1 = normal_indices[i + 1] - 1;
      tri_index.i2 = normal_indices[i + 2] - 1;
      buffer->normal_indices.push_back(tri_index);
    }
  }

  buffer->nfaces += ntriangles;
  return 0;
}

int ObjBufferFromFile(ObjBuffer *buffer, const char *filename)
{
  ObjParser *parser = NULL;
  int err = 0;

  if (buffer == NULL)
    return -1;

  if (filename == NULL)
    return -1;

  parser = ObjParserNew(
      buffer,
      read_vertx,
      read_texture,
      read_normal,
      read_face);

  err = ObjParse(parser, filename);
  if (err) {
    return -1;
  }

  ObjParserFree(parser);
  return 0;
}

int ObjBufferToMeshFile(ObjBuffer *buffer, const char *filename)
{
  MeshOutput *out = MshOpenOutputFile(filename);
  if (out == NULL) {
    return -1;
  }

  out->nverts = buffer->nverts;
  out->P  = &buffer->P[0];
  out->N  = &buffer->N[0];
  out->uv = &buffer->uv[0];
  out->nfaces = buffer->nfaces;
  out->nface_attrs = 1;
  out->indices = &buffer->vertex_indices[0];

  MshWriteFile(out);
  MshCloseOutputFile(out);
  return 0;
}

int ObjBufferComputeNormals(ObjBuffer *buffer)
{
  const int nverts = buffer->nverts;
  const int nfaces = buffer->nfaces;
  Vector *P = &buffer->P[0];
  Vector *N = &buffer->N[0];
  Index3 *indices = &buffer->vertex_indices[0];
  int i;

  if (buffer->P.empty() || buffer->vertex_indices.empty())
    return -1;

  if (buffer->N.empty()) {
    buffer->N.resize(nverts);
    N = &buffer->N[0];
  }

  // initialize N
  for (i = 0; i < nverts; i++) {
    Vector *nml = &N[i];
    *nml = Vector();
  }

  // compute N
  for (i = 0; i < nfaces; i++) {
    Vector *P0, *P1, *P2;
    Vector *N0, *N1, *N2;
    Vector Ng;
    const int i0 = indices[i].i0;
    const int i1 = indices[i].i1;
    const int i2 = indices[i].i2;

    P0 = &P[i0];
    P1 = &P[i1];
    P2 = &P[i2];
    N0 = &N[i0];
    N1 = &N[i1];
    N2 = &N[i2];

    Ng = TriComputeFaceNormal(*P0, *P1, *P2);

    N0->x += Ng.x;
    N0->y += Ng.y;
    N0->z += Ng.z;

    N1->x += Ng.x;
    N1->y += Ng.y;
    N1->z += Ng.z;

    N2->x += Ng.x;
    N2->y += Ng.y;
    N2->z += Ng.z;
  }

  // normalize N
  for (i = 0; i < nverts; i++) {
    Vector *nml = &N[i];
    Normalize(nml);
  }

  return 0;
}
