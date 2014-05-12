/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjParser.h"
#include "fj_tex_coord.h"
#include "fj_triangle.h"
#include "fj_memory.h"
#include "fj_mesh_io.h"
#include "fj_vector.h"
#include "fj_array.h"
#include "fj_mesh.h"

#include <stdio.h>
#include <string.h>

using namespace fj;

static const char USAGE[] =
"Usage: obj2mesh [options] inputfile(*.obj) outputfile(*.mesh)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

struct ObjBuffer {
  struct Array *P;
  struct Array *N;
  struct Array *uv;
  struct Array *vertex_indices;
  struct Array *texture_indices;
  struct Array *normal_indices;

  long nverts;
  long nfaces;
};

extern struct ObjBuffer *ObjBufferNew(void);
extern void ObjBufferFree(struct ObjBuffer *buffer);
extern int ObjBufferFromFile(struct ObjBuffer *buffer, const char *filename);
extern int ObjBufferToMeshFile(const struct ObjBuffer *buffer, const char *filename);
extern int ObjBufferComputeNormals(struct ObjBuffer *buffer);

int main(int argc, const char **argv)
{
  struct ObjBuffer *buffer = NULL;
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

  buffer = ObjBufferNew();
  if (buffer == NULL) {
    /* TODO error handling */
    return -1;
  }

  err = ObjBufferFromFile(buffer, in_filename);
  if (err) {
    /* TODO error handling */
    return -1;
  }

  err = ObjBufferComputeNormals(buffer);
  if (err) {
    /* TODO error handling */
    return -1;
  }

  err = ObjBufferToMeshFile(buffer, out_filename);
  if (err) {
    /* TODO error handling */
    return -1;
  }

  printf("nverts: %ld\n", buffer->nverts);
  printf("nfaces: %ld\n", buffer->nfaces);

  ObjBufferFree(buffer);
  return 0;
}

struct ObjBuffer *ObjBufferNew(void)
{
  struct ObjBuffer *buffer = FJ_MEM_ALLOC(struct ObjBuffer);

  if (buffer == NULL)
    return NULL;

  buffer->P = ArrNew(sizeof(struct Vector));
  buffer->N = ArrNew(sizeof(struct Vector));
  buffer->uv = ArrNew(sizeof(struct TexCoord));
  buffer->vertex_indices = ArrNew(sizeof(struct TriIndex));
  buffer->texture_indices = ArrNew(sizeof(struct TriIndex));
  buffer->normal_indices = ArrNew(sizeof(struct TriIndex));

  buffer->nverts = 0;
  buffer->nfaces = 0;

  return buffer;
}

void ObjBufferFree(struct ObjBuffer *buffer)
{
  if (buffer == NULL)
    return;

  ArrFree(buffer->P);
  ArrFree(buffer->N);
  ArrFree(buffer->uv);
  ArrFree(buffer->vertex_indices);
  ArrFree(buffer->texture_indices);
  ArrFree(buffer->normal_indices);

  FJ_MEM_FREE(buffer);
}

static int read_vertx(
    void *interpreter,
    int scanned_ncomponents,
    double x,
    double y,
    double z,
    double w)
{
  struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;
  struct Vector P;

  P.x = x;
  P.y = y;
  P.z = z;
  ArrPush(buffer->P, &P);

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
  struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;
  struct TexCoord uv = {0, 0};

  uv.u = x;
  uv.v = y;
  ArrPush(buffer->uv, &uv);

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
  struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;
  struct Vector N;

  N.x = x;
  N.y = y;
  N.z = z;
  ArrPush(buffer->N, &N);

  return 0;
}

static int read_face(
    void *interpreter,
    long index_count,
    const long *vertex_indices,
    const long *texture_indices,
    const long *normal_indices)
{
  struct ObjBuffer *buffer = (struct ObjBuffer *) interpreter;
  int i;

  const int ntriangles = index_count - 2;

  for (i = 0; i < ntriangles; i++) {
    if (vertex_indices != NULL) {
      struct TriIndex tri_index = {0, 0, 0};
      tri_index.i0 = vertex_indices[0] - 1;
      tri_index.i1 = vertex_indices[i + 1] - 1;
      tri_index.i2 = vertex_indices[i + 2] - 1;
      ArrPush(buffer->vertex_indices, &tri_index);
    }

    if (texture_indices != NULL) {
      struct TriIndex tri_index = {0, 0, 0};
      tri_index.i0 = texture_indices[0] - 1;
      tri_index.i1 = texture_indices[i + 1] - 1;
      tri_index.i2 = texture_indices[i + 2] - 1;
      ArrPush(buffer->texture_indices, &tri_index);
    }

    if (normal_indices != NULL) {
      struct TriIndex tri_index = {0, 0, 0};
      tri_index.i0 = normal_indices[0] - 1;
      tri_index.i1 = normal_indices[i + 1] - 1;
      tri_index.i2 = normal_indices[i + 2] - 1;
      ArrPush(buffer->normal_indices, &tri_index);
    }
  }

  buffer->nfaces += ntriangles;
  return 0;
}

int ObjBufferFromFile(struct ObjBuffer *buffer, const char *filename)
{
  struct ObjParser *parser = NULL;
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

int ObjBufferToMeshFile(const struct ObjBuffer *buffer, const char *filename)
{
  struct MeshOutput *out = MshOpenOutputFile(filename);
  if (out == NULL) {
    return -1;
  }

  out->nverts = buffer->nverts;
  out->P = (struct Vector *) buffer->P->data;
  out->N = (struct Vector *) buffer->N->data;
  out->uv = (struct TexCoord *) buffer->uv->data;
  out->nfaces = buffer->nfaces;
  out->nface_attrs = 1;
  out->indices = (struct TriIndex *) buffer->vertex_indices->data;

  MshWriteFile(out);
  MshCloseOutputFile(out);
  return 0;
}

int ObjBufferComputeNormals(struct ObjBuffer *buffer)
{
  const int nverts = buffer->nverts;
  const int nfaces = buffer->nfaces;
  struct Vector *P = (struct Vector *) buffer->P->data;
  struct Vector *N = (struct Vector *) buffer->N->data;
  struct TriIndex *indices = (struct TriIndex *) buffer->vertex_indices->data;
  int i;

  if (P == NULL || indices == NULL)
    return -1;

  if (N == NULL) {
    ArrResize(buffer->N, 3 * nverts);
    N = (struct Vector *) buffer->N->data;
  }

  /* initialize N */
  for (i = 0; i < nverts; i++) {
    struct Vector *nml = &N[i];
    VEC3_SET(nml, 0, 0, 0);
  }

  /* compute N */
  for (i = 0; i < nfaces; i++) {
    struct Vector *P0, *P1, *P2;
    struct Vector *N0, *N1, *N2;
    struct Vector Ng;
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

  /* normalize N */
  for (i = 0; i < nverts; i++) {
    struct Vector *nml = &N[i];
    VEC3_NORMALIZE(nml);
  }

  return 0;
}

