// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_mesh_io.h"
#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_mesh.h"

#include <cstring>

#define MSH_FILE_VERSION 1
#define MSH_FILE_MAGIC "MESH"
#define MSH_MAGIC_SIZE 4
#define MAX_ATTRNAME_SIZE 32

namespace fj {

static int error_no = MSH_ERR_NONE;
static size_t write_attriname(MeshOutput *out, const std::string &name);
static size_t write_attridata(MeshOutput *out, const std::string &name);
static size_t read_attridata(MeshInput *in);
static void set_error(int err);

// mesh input file interfaces
MeshInput *MshOpenInputFile(const char *filename)
{
  MeshInput *in = new MeshInput();

  if (in == NULL) {
    set_error(MSH_ERR_NO_MEMORY);
    return NULL;
  }

  in->file = fopen(filename, "rb");
  if (in->file == NULL) {
    set_error(MSH_ERR_FILE_NOT_EXIST);
    delete in;
    return NULL;
  }

  in->version = 0;
  in->nverts = 0;
  in->nvert_attrs = 0;
  in->nfaces = 0;
  in->nface_attrs = 0;

  return in;
}

void MshCloseInputFile(MeshInput *in)
{
  if (in == NULL)
    return;

  if (in->file != NULL) {
    fclose(in->file);
  }

  delete in;
}

int MshReadHeader(MeshInput *in)
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

  nattrs_alloc = in->nvert_attrs + in->nface_attrs;
  in->attr_names.resize(nattrs_alloc, "");

  for (i = 0; i < in->nvert_attrs + in->nface_attrs; i++) {
    char attrname[MAX_ATTRNAME_SIZE] = {'\0'};

    nreads += fread(&namesize, sizeof(size_t), 1, in->file);
    if (namesize > MAX_ATTRNAME_SIZE-1) {
      set_error(MSH_ERR_LONG_ATTRIB_NAME);
      return -1;
    }
    nreads += fread(attrname, sizeof(char), namesize, in->file);
    in->attr_names[i] = attrname;
  }

  return 0;
}

// mesh output file interfaces
MeshOutput *MshOpenOutputFile(const char *filename)
{
  MeshOutput *out = new MeshOutput();

  if (out == NULL) {
    set_error(MSH_ERR_NO_MEMORY);
    return NULL;
  }

  out->file = fopen(filename, "wb");
  if (out->file == NULL) {
    set_error(MSH_ERR_FILE_NOT_EXIST);
    delete out;
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
  out->face_group_id = NULL;

  return out;
}

void MshCloseOutputFile(MeshOutput *out)
{
  if (out == NULL)
    return;

  if (out->file != NULL) {
    fclose(out->file);
  }
  delete out;
}

void MshWriteFile(MeshOutput *out)
{
  char magic[] = MSH_FILE_MAGIC;

  // counts nvert_attrs automatically
  out->nvert_attrs = 0;
  if (out->P != NULL) out->nvert_attrs++;
  if (out->N != NULL) out->nvert_attrs++;
  if (out->Cd != NULL) out->nvert_attrs++;
  if (out->uv != NULL) out->nvert_attrs++;
  if (out->velocity != NULL) out->nvert_attrs++;
  out->nface_attrs = 0;
  if (out->indices != NULL) out->nface_attrs++;
  if (out->face_group_id != NULL) out->nface_attrs++;

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
  write_attriname(out, "face_group_id");

  write_attridata(out, "P");
  write_attridata(out, "N");
  write_attridata(out, "Cd");
  write_attridata(out, "uv");
  write_attridata(out, "velocity");
  write_attridata(out, "indices");
  write_attridata(out, "face_group_id");
}

int MshLoadFile(Mesh *mesh, const char *filename)
{
  int i, j;
  int TOTAL_ATTR_COUNT;
  MeshInput *in;

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
    const std::string &attrname = in->attr_names[i];
    if (attrname == "P") {
      mesh->SetVertexCount(in->nverts);
      mesh->AddVertexPosition();
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const double *data = (const double *) &in->data_buffer[0];
        Vector P;
        P.x = data[3*j + 0];
        P.y = data[3*j + 1];
        P.z = data[3*j + 2];
        mesh->SetVertexPosition(j, P);
      }
    }
    else if (attrname == "N") {
      mesh->SetVertexCount(in->nverts);
      mesh->AddVertexNormal();
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const double *data = (const double *) &in->data_buffer[0];
        Vector N;
        N.x = data[3*j + 0];
        N.y = data[3*j + 1];
        N.z = data[3*j + 2];
        mesh->SetVertexNormal(j, N);
      }
    }
    else if (attrname == "Cd") {
      mesh->SetVertexCount(in->nverts);
      mesh->AddVertexColor();
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const float *data = (const float *) &in->data_buffer[0];
        Color Cd;
        Cd.r = data[3*j + 0];
        Cd.g = data[3*j + 1];
        Cd.b = data[3*j + 2];
        mesh->SetVertexColor(j, Cd);
      }
    }
    else if (attrname == "uv") {
      mesh->SetVertexCount(in->nverts);
      mesh->AddVertexTexture();
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const float *data = (const float *) &in->data_buffer[0];
        TexCoord texcoord;
        texcoord.u = data[2*j + 0];
        texcoord.v = data[2*j + 1];
        mesh->SetVertexTexture(j, texcoord);
      }
    }
    else if (attrname == "velocity") {
      mesh->SetVertexCount(in->nverts);
      mesh->AddVertexVelocity();
      read_attridata(in);
      for (j = 0; j < in->nverts; j++) {
        const double *data = (const double *) &in->data_buffer[0];
        Vector velocity;
        velocity.x = data[3*j + 0];
        velocity.y = data[3*j + 1];
        velocity.z = data[3*j + 2];
        mesh->SetVertexVelocity(j, velocity);
      }
    }
    else if (attrname == "indices") {
      mesh->SetFaceCount(in->nfaces);
      mesh->AddFaceIndices();
      read_attridata(in);
      for (j = 0; j < in->nfaces; j++) {
        const Index *data = (const Index *) &in->data_buffer[0];
        Index3 tri_index;
        tri_index.i0 = data[3*j + 0];
        tri_index.i1 = data[3*j + 1];
        tri_index.i2 = data[3*j + 2];
        mesh->SetFaceIndices(j, tri_index);
      }
    }
    else if (attrname == "face_group_id") {
      mesh->SetFaceCount(in->nfaces);
      mesh->AddFaceGroupID();
      read_attridata(in);
      for (j = 0; j < in->nfaces; j++) {
        const int *data = (const int *) &in->data_buffer[0];
        const int group_id = data[j];
        mesh->SetFaceGroupID(j, group_id);
      }
    }
  }

  mesh->ComputeBounds();
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

static size_t write_attriname(MeshOutput *out, const std::string &name)
{
  size_t namesize;
  size_t nwrotes;

  if (name == "P" && out->P == NULL) {
    return 0;
  }
  else if (name == "N" && out->N == NULL) {
    return 0;
  }
  else if (name == "Cd" && out->Cd == NULL) {
    return 0;
  }
  else if (name == "uv" && out->uv == NULL) {
    return 0;
  }
  else if (name == "velocity" && out->velocity == NULL) {
    return 0;
  }
  else if (name == "indices" && out->indices == NULL) {
    return 0;
  }
  else if (name == "face_group_id" && out->face_group_id == NULL) {
    return 0;
  }
  else {
  }

  nwrotes = 0;
  namesize = name.length() + 1;
  nwrotes += fwrite(&namesize, sizeof(size_t), 1, out->file);
  nwrotes += fwrite(&name[0], sizeof(char), namesize, out->file);

  return nwrotes;
}

static size_t write_attridata(MeshOutput *out, const std::string &name)
{
  size_t datasize = 0;
  size_t nwrotes = 0;
  int i;

  if (name == "P") {
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
  else if (name == "N") {
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
  else if (name == "Cd") {
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
  else if (name == "uv") {
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
  else if (name == "velocity") {
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
  else if (name == "indices") {
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
  else if (name == "face_group_id") {
    if (out->face_group_id == NULL)
      return 0;
    datasize = sizeof(int) * out->nfaces;
    nwrotes += fwrite(&datasize, sizeof(size_t), 1, out->file);
    for (i = 0; i < out->nfaces; i++) {
      const int id = out->face_group_id[i];
      nwrotes += fwrite(&id, sizeof(int), 1, out->file);
    }
  }
  return nwrotes;
}

static size_t read_attridata(MeshInput *in)
{
  size_t nreads = 0;
  size_t datasize = 0;
  nreads += fread(&datasize, sizeof(size_t), 1, in->file);

  in->data_buffer.resize(datasize);

  nreads += fread(&in->data_buffer[0], sizeof(char), datasize, in->file);

  return nreads;
}

} // namespace xxx
