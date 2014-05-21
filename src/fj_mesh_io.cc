/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_mesh_io.h"
#include "fj_string_function.h"
#include "fj_tex_coord.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_mesh.h"

#include <string.h>

#define MSH_FILE_VERSION 1
#define MSH_FILE_MAGIC "MESH"
#define MSH_MAGIC_SIZE 4
#define MAX_ATTRNAME_SIZE 32

namespace fj {

static int error_no = MSH_ERR_NONE;
static size_t write_attriname(struct MeshOutput *out, const char *name);
static size_t write_attridata(struct MeshOutput *out, const char *name);
static size_t read_attridata(struct MeshInput *in);
static void set_error(int err);

/* mesh input file interfaces */
struct MeshInput *MshOpenInputFile(const char *filename)
{
  struct MeshInput *in = FJ_MEM_ALLOC(struct MeshInput);

  if (in == NULL) {
    set_error(MSH_ERR_NO_MEMORY);
    return NULL;
  }

  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    set_error(MSH_ERR_FILE_NOT_EXIST);
    FJ_MEM_FREE(in);
    return NULL;
  }

  in->version = 0;
  in->nverts = 0;
  in->nvert_attrs = 0;
  in->nfaces = 0;
  in->nface_attrs = 0;
  in->data_buffer = NULL;
  in->buffer_size = 0;

  in->attr_names = NULL;

  return in;
}

void MshCloseInputFile(struct MeshInput *in)
{
  char **name = NULL;
  if (in == NULL)
    return;

  if (in->attr_names != NULL) {
    /* make sure in->attr_names is not NULL because
    need to dereference like *name != NULL */
    for (name = in->attr_names; *name != NULL; name++) {
      *name = StrFree(*name);
    }
    FJ_MEM_FREE(in->attr_names);
  }

  if (in->file != NULL) {
    fclose(in->file);
  }

  if (in->data_buffer != NULL) {
    FJ_MEM_FREE(in->data_buffer);
  }

  FJ_MEM_FREE(in);
}

int MshReadHeader(struct MeshInput *in)
{
  int i;
  size_t nreads = 0;
  size_t namesize = 1;
  char magic[MSH_MAGIC_SIZE];
  int nattrs_alloc;

  nreads += fread(magic, sizeof(char), MSH_MAGIC_SIZE, in->file);
  if (memcmp(magic, MSH_FILE_MAGIC, MSH_MAGIC_SIZE) != 0) {
    set_error(MSH_ERR_BAD_MAGIC_NUMBER);
    return -1;
  }
  nreads += fread(&in->version, sizeof(int), 1, in->file);
  if (in->version != MSH_FILE_VERSION) {
    set_error(MSH_ERR_BAD_FILE_VERSION);
    return -1;
  }
  nreads += fread(&in->nverts, sizeof(int), 1, in->file);
  nreads += fread(&in->nvert_attrs, sizeof(int), 1, in->file);
  nreads += fread(&in->nfaces, sizeof(int), 1, in->file);
  nreads += fread(&in->nface_attrs, sizeof(int), 1, in->file);

  nattrs_alloc = in->nvert_attrs + in->nface_attrs + 1; /* for sentinel */
  in->attr_names = FJ_MEM_ALLOC_ARRAY(char *, nattrs_alloc);
  for (i = 0; i < nattrs_alloc; i++) {
    in->attr_names[i] = NULL;
  }

  for (i = 0; i < in->nvert_attrs + in->nface_attrs; i++) {
    char attrname[MAX_ATTRNAME_SIZE] = {'\0'};

    nreads += fread(&namesize, sizeof(size_t), 1, in->file);
    if (namesize > MAX_ATTRNAME_SIZE-1) {
      set_error(MSH_ERR_LONG_ATTRIB_NAME);
      return -1;
    }
    nreads += fread(attrname, sizeof(char), namesize, in->file);
    in->attr_names[i] = StrDup(attrname);
  }

  return 0;
}

/* mesh output file interfaces */
struct MeshOutput *MshOpenOutputFile(const char *filename)
{
  struct MeshOutput *out = FJ_MEM_ALLOC(struct MeshOutput);

  if (out == NULL) {
    set_error(MSH_ERR_NO_MEMORY);
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(MSH_ERR_FILE_NOT_EXIST);
    FJ_MEM_FREE(out);
    return NULL;
  }

  out->version = MSH_FILE_VERSION;
  out->nverts = 0;
  out->nvert_attrs = 0;
  out->nfaces = 0;
  out->nface_attrs = 0;
  out->P = NULL;
  out->N = NULL;
  out->Cd = NULL;
  out->uv = NULL;
  out->velocity = NULL;
  out->indices = NULL;

  return out;
}

void MshCloseOutputFile(struct MeshOutput *out)
{
  if (out == NULL)
    return;

  if (out->file != NULL) {
    fclose(out->file);
  }
  FJ_MEM_FREE(out);
}

void MshWriteFile(struct MeshOutput *out)
{
  char magic[] = MSH_FILE_MAGIC;

  /* counts nvert_attrs automatically */
  out->nvert_attrs = 0;
  if (out->P != NULL) out->nvert_attrs++;
  if (out->N != NULL) out->nvert_attrs++;
  if (out->Cd != NULL) out->nvert_attrs++;
  if (out->uv != NULL) out->nvert_attrs++;
  if (out->velocity != NULL) out->nvert_attrs++;
  out->nface_attrs = 0;
  if (out->indices != NULL) out->nface_attrs++;

  fwrite(magic, sizeof(char), MSH_MAGIC_SIZE, out->file);
  fwrite(&out->version, sizeof(int), 1, out->file);
  fwrite(&out->nverts, sizeof(int), 1, out->file);
  fwrite(&out->nvert_attrs, sizeof(int), 1, out->file);
  fwrite(&out->nfaces, sizeof(int), 1, out->file);
  fwrite(&out->nface_attrs, sizeof(int), 1, out->file);

  write_attriname(out, "P");
  write_attriname(out, "N");
  write_attriname(out, "Cd");
  write_attriname(out, "uv");
  write_attriname(out, "velocity");
  write_attriname(out, "indices");

  write_attridata(out, "P");
  write_attridata(out, "N");
  write_attridata(out, "Cd");
  write_attridata(out, "uv");
  write_attridata(out, "velocity");
  write_attridata(out, "indices");
}

int MshLoadFile(struct Mesh *mesh, const char *filename)
{
  int i, j;
  int TOTAL_ATTR_COUNT;
  struct MeshInput *in;

  in = MshOpenInputFile(filename);
  if (in == NULL) {
    return -1;
  }

  if (MshReadHeader(in)) {
    MshCloseInputFile(in);
    return -1;
  }

  TOTAL_ATTR_COUNT = in->nvert_attrs + in->nface_attrs;

  for (i = 0; i < TOTAL_ATTR_COUNT; i++) {
    const char *attrname;
    attrname = in->attr_names[i];
    if (strcmp(attrname, "P") == 0) {
      MshAllocateVertex(mesh, "P", in->nverts);
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const double *data = (const double *) in->data_buffer;
        struct Vector P;
        P.x = data[3*j + 0];
        P.y = data[3*j + 1];
        P.z = data[3*j + 2];
        MshSetVertexPosition(mesh, j, &P);
      }
    }
    else if (strcmp(attrname, "N") == 0) {
      MshAllocateVertex(mesh, "N", in->nverts);
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const double *data = (const double *) in->data_buffer;
        struct Vector N;
        N.x = data[3*j + 0];
        N.y = data[3*j + 1];
        N.z = data[3*j + 2];
        MshSetVertexNormal(mesh, j, &N);
      }
    }
    else if (strcmp(attrname, "Cd") == 0) {
      MshAllocateVertex(mesh, "Cd", in->nverts);
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const float *data = (const float *) in->data_buffer;
        struct Color Cd;
        Cd.r = data[3*j + 0];
        Cd.g = data[3*j + 1];
        Cd.b = data[3*j + 2];
        MshSetVertexColor(mesh, j, &Cd);
      }
    }
    else if (strcmp(attrname, "uv") == 0) {
      MshAllocateVertex(mesh, "uv", in->nverts);
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const float *data = (const float *) in->data_buffer;
        struct TexCoord texcoord = {0, 0};
        texcoord.u = data[2*j + 0];
        texcoord.v = data[2*j + 1];
        MshSetVertexTexture(mesh, j, &texcoord);
      }
    }
    else if (strcmp(attrname, "velocity") == 0) {
      MshAllocateVertex(mesh, "velocity", in->nverts);
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const double *data = (const double *) in->data_buffer;
        struct Vector velocity;
        velocity.x = data[3*j + 0];
        velocity.y = data[3*j + 1];
        velocity.z = data[3*j + 2];
        MshSetVertexVelocity(mesh, j, &velocity);
      }
    }
    else if (strcmp(attrname, "indices") == 0) {
      MshAllocateFace(mesh, "indices", in->nfaces);
      read_attridata(in);
      for (j = 0; j < in->nfaces; j++) {
        const Index *data = (const Index *) in->data_buffer;
        struct TriIndex tri_index = {0, 0, 0};
        tri_index.i0 = data[3*j + 0];
        tri_index.i1 = data[3*j + 1];
        tri_index.i2 = data[3*j + 2];
        MshSetFaceVertexIndices(mesh, j, &tri_index);
      }
    }
  }

  MshComputeBounds(mesh);
  MshCloseInputFile(in);

  return 0;
}

int MshGetErrorNo(void)
{
  return error_no;
}

static void set_error(int err)
{
  error_no = err;
}

static size_t write_attriname(struct MeshOutput *out, const char *name)
{
  size_t namesize;
  size_t nwrotes;

  if (strcmp(name, "P") == 0 && out->P == NULL) {
    return 0;
  }
  else if (strcmp(name, "N") == 0 && out->N == NULL) {
    return 0;
  }
  else if (strcmp(name, "Cd") == 0 && out->Cd == NULL) {
    return 0;
  }
  else if (strcmp(name, "uv") == 0 && out->uv == NULL) {
    return 0;
  }
  else if (strcmp(name, "velocity") == 0 && out->velocity == NULL) {
    return 0;
  }
  else if (strcmp(name, "indices") == 0 && out->indices == NULL) {
    return 0;
  }

  nwrotes = 0;
  namesize = strlen(name) + 1;
  nwrotes += fwrite(&namesize, sizeof(size_t), 1, out->file);
  nwrotes += fwrite(name, sizeof(char), namesize, out->file);

  return nwrotes;
}

static size_t write_attridata(struct MeshOutput *out, const char *name)
{
  size_t datasize = 0;
  size_t nwrotes = 0;
  int i;

  if (strcmp(name, "P") == 0) {
    if (out->P == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nverts; i++) {
      double P[3] = {0, 0, 0};
      P[0] = out->P[i].x;
      P[1] = out->P[i].y;
      P[2] = out->P[i].z;
      nwrotes += fwrite(P, sizeof(double), 3, out->file);
    }
  }
  else if (strcmp(name, "N") == 0) {
    if (out->N == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nverts; i++) {
      double N[3] = {0, 0, 0};
      N[0] = out->N[i].x;
      N[1] = out->N[i].y;
      N[2] = out->N[i].z;
      nwrotes += fwrite(N, sizeof(double), 3, out->file);
    }
  }
  else if (strcmp(name, "Cd") == 0) {
    if (out->Cd == NULL)
      return 0;
    datasize = 3 * sizeof(float) * out->nverts;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nverts; i++) {
      float Cd[3] = {0, 0, 0};
      Cd[0] = out->Cd[i].r;
      Cd[1] = out->Cd[i].g;
      Cd[2] = out->Cd[i].b;
      nwrotes += fwrite(Cd, sizeof(float), 3, out->file);
    }
  }
  else if (strcmp(name, "uv") == 0) {
    if (out->uv == NULL)
      return 0;
    datasize = 2 * sizeof(float) * out->nverts;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nverts; i++) {
      float uv[2] = {0, 0};
      uv[0] = out->uv[i].u;
      uv[1] = out->uv[i].v;
      nwrotes += fwrite(uv, sizeof(float), 2, out->file);
    }
  }
  else if (strcmp(name, "velocity") == 0) {
    if (out->velocity == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nverts; i++) {
      double velocity[3] = {0, 0, 0};
      velocity[0] = out->velocity[i].x;
      velocity[1] = out->velocity[i].y;
      velocity[2] = out->velocity[i].z;
      nwrotes += fwrite(velocity, sizeof(double), 3, out->file);
    }
  }
  else if (strcmp(name, "indices") == 0) {
    if (out->indices == NULL)
      return 0;
    datasize = 3 * sizeof(int) * out->nfaces;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nfaces; i++) {
      Index indices[3] = {0, 0, 0};
      indices[0] = out->indices[i].i0;
      indices[1] = out->indices[i].i1;
      indices[2] = out->indices[i].i2;
      nwrotes += fwrite(indices, sizeof(Index), 3, out->file);
    }
  }
  return nwrotes;
}

static size_t read_attridata(struct MeshInput *in)
{
  size_t nreads = 0;
  size_t datasize = 0;
  nreads += fread(&datasize, sizeof(size_t), 1, in->file);

  if (in->buffer_size < datasize) {
    in->data_buffer = FJ_MEM_REALLOC_ARRAY(in->data_buffer, char, datasize);
    in->buffer_size = datasize;
  }

  nreads += fread(in->data_buffer, sizeof(char), datasize, in->file);

  return nreads;
}

} // namespace xxx
