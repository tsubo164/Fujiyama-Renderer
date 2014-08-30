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
static void set_error(int err);

template<typename T>
inline void read_(std::ifstream &file, T *dst, int64_t count)
{
  file.read(reinterpret_cast<char*>(dst), sizeof(*dst) * count);
}

MeshInput::MeshInput() :
    version_(0),
    nverts_(0),
    nvert_attrs_(0),
    nfaces_(0),
    nface_attrs_(0)
{
}

MeshInput::~MeshInput()
{
  Close();
}

int MeshInput::Open(const std::string &filename)
{
  file_.open(filename.c_str(), std::ios::in | std::ios::binary);

  if (Fail()) {
    return -1;
  }

  return 0;
}

void MeshInput::Close()
{
  file_.close();
}

bool MeshInput::Fail() const
{
  return file_.fail();
}

int MeshInput::ReadHeader()
{
  char magic[MSH_MAGIC_SIZE] = {'\0'};

  read_(file_, magic, MSH_MAGIC_SIZE);
  if (memcmp(magic, MSH_FILE_MAGIC, MSH_MAGIC_SIZE) != 0) {
    set_error(MSH_ERR_BAD_MAGIC_NUMBER);
    return -1;
  }

  read_(file_, &version_, 1);
  if (version_ != MSH_FILE_VERSION) {
    set_error(MSH_ERR_BAD_FILE_VERSION);
    return -1;
  }

  read_(file_, &nverts_,      1);
  read_(file_, &nvert_attrs_, 1);
  read_(file_, &nfaces_,      1);
  read_(file_, &nface_attrs_, 1);

  const int TOTAL_ATTR_COUNT = GetVertexAttributeCount() + GetFaceAttributeCount();
  attr_names_.resize(TOTAL_ATTR_COUNT, "");

  for (int i = 0; i < TOTAL_ATTR_COUNT; i++) {
    char attrname[MAX_ATTRNAME_SIZE] = {'\0'};
    size_t namesize = 1;

    read_(file_, &namesize, 1);
    if (namesize > MAX_ATTRNAME_SIZE-1) {
      set_error(MSH_ERR_LONG_ATTRIB_NAME);
      return -1;
    }
    read_(file_, attrname, namesize);
    attr_names_[i] = attrname;
  }

  return 0;
}

void MeshInput::ReadAttributeData()
{
  size_t datasize = 0;
  read_(file_, &datasize, 1);

  data_buffer_.resize(datasize);

  read_(file_, &data_buffer_[0], datasize);
}

int MeshInput::GetVertexCount() const
{
  return nverts_;
}

int MeshInput::GetVertexAttributeCount() const
{
  return nvert_attrs_;
}

int MeshInput::GetFaceCount() const
{
  return nfaces_;
}

int MeshInput::GetFaceAttributeCount() const
{
  return nface_attrs_;
}

const char *MeshInput::GetDataBuffer() const
{
  return &data_buffer_[0];
}

const std::string MeshInput::GetAttributeName(int i) const
{
  if (i < 0 || i >= static_cast<int>(attr_names_.size())) {
    return std::string("");
  }
  return attr_names_[i];
}

template<typename T>
inline void write_(std::ofstream &file, const T *src, int64_t count)
{
  file.write(reinterpret_cast<const char*>(src), sizeof(*src) * count);
}

MeshOutput::MeshOutput() :
    version(MSH_FILE_VERSION),
    nverts(0),
    nvert_attrs(0),
    nfaces(0),
    nface_attrs(0),
    P(NULL),
    N(NULL),
    Cd(NULL),
    uv(NULL),
    velocity(NULL),
    indices(NULL),
    face_group_id(NULL)
{
}

MeshOutput::~MeshOutput()
{
  Close();
}

int MeshOutput::Open(const std::string &filename)
{
  file_.open(filename.c_str(), std::ios::out | std::ios::binary);

  if (Fail()) {
    return -1;
  }

  return 0;
}

void MeshOutput::Close()
{
  file_.close();
  printf("********* CLOSED ==================================================\n");
}

bool MeshOutput::Fail() const
{
  return file_.fail();
}

void MeshOutput::SetVertexCount(int count)
{
  if (count < 0) {
    return;
  }
  nverts = count;
}

void MeshOutput::SetVertexPosition(const Vector *position)
{
  P = position;
  if (P != NULL) {
    nvert_attrs++;
  }
}

void MeshOutput::SetVertexNormal(const Vector *normal)
{
  N = normal;
  if (N != NULL) {
    nvert_attrs++;
  }
}

void MeshOutput::SetVertexColor(const Color *color)
{
  Cd = color;
  if (Cd != NULL) {
    nvert_attrs++;
  }
}

void MeshOutput::SetVertexTexture(const TexCoord *texcoord)
{
  uv = texcoord;
  if (uv != NULL) {
    nvert_attrs++;
  }
}

void MeshOutput::SetVertexVelocity(const Vector *vel)
{
  velocity = vel;
  if (velocity != NULL) {
    nvert_attrs++;
  }
}

void MeshOutput::SetFaceCount(int count)
{
  if (count < 0) {
    return;
  }
  nfaces = count;
}

void MeshOutput::SetFaceIndex3(const Index3 *index)
{
  indices = index;
  if (indices != NULL) {
    nface_attrs++;
  }
}

void MeshOutput::SetFaceGroupID(const int *id)
{
  face_group_id = id;
  if (indices != NULL) {
    nface_attrs++;
  }
}

void MeshOutput::WriteFile()
{
  char magic[] = MSH_FILE_MAGIC;

  // counts nvert_attrs automatically
  nvert_attrs = 0;
  if (P != NULL) nvert_attrs++;
  if (N != NULL) nvert_attrs++;
  if (Cd != NULL) nvert_attrs++;
  if (uv != NULL) nvert_attrs++;
  if (velocity != NULL) nvert_attrs++;
  nface_attrs = 0;
  if (indices != NULL) nface_attrs++;
  if (face_group_id != NULL) nface_attrs++;

  write_(file_, magic, MSH_MAGIC_SIZE);
  write_(file_, &version,     1);
  write_(file_, &nverts,      1);
  write_(file_, &nvert_attrs, 1);
  write_(file_, &nfaces,      1);
  write_(file_, &nface_attrs, 1);

  write_attriname(this, "P");
  write_attriname(this, "N");
  write_attriname(this, "Cd");
  write_attriname(this, "uv");
  write_attriname(this, "velocity");
  write_attriname(this, "indices");
  write_attriname(this, "face_group_id");

  write_attridata(this, "P");
  write_attridata(this, "N");
  write_attridata(this, "Cd");
  write_attridata(this, "uv");
  write_attridata(this, "velocity");
  write_attridata(this, "indices");
  write_attridata(this, "face_group_id");
}

// mesh output file interfaces
MeshOutput *MshOpenOutputFile(const char *filename)
{
  MeshOutput *out = new MeshOutput();

  if (out == NULL) {
    set_error(MSH_ERR_NO_MEMORY);
    return NULL;
  }

  out->file_.open(filename, std::ios::out | std::ios::binary);
  if (out->file_.fail()) {
    set_error(MSH_ERR_FILE_NOT_EXIST);
    delete out;
    return NULL;
  }

  return out;
}

void MshCloseOutputFile(MeshOutput *out)
{
  if (out == NULL)
    return;

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

  write_(out->file_, magic, MSH_MAGIC_SIZE);
  write_(out->file_, &out->version,     1);
  write_(out->file_, &out->nverts,      1);
  write_(out->file_, &out->nvert_attrs, 1);
  write_(out->file_, &out->nfaces,      1);
  write_(out->file_, &out->nface_attrs, 1);

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
  printf("************* TEST ****************\n");
  MeshInput in;

  in.Open(filename);
  if (in.Fail()) {
    return -1;
  }

  if (in.ReadHeader()) {
    return -1;
  }

  const int TOTAL_ATTR_COUNT = in.GetVertexAttributeCount() + in.GetFaceAttributeCount();

  for (int i = 0; i < TOTAL_ATTR_COUNT; i++) {
    const std::string attrname = in.GetAttributeName(i);

    if (attrname == "P") {
      mesh->SetVertexCount(in.GetVertexCount());
      mesh->AddVertexPosition();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetVertexCount(); j++) {
        const double *data = (const double *) in.GetDataBuffer();
        Vector P;
        P.x = data[3*j + 0];
        P.y = data[3*j + 1];
        P.z = data[3*j + 2];
        mesh->SetVertexPosition(j, P);
      }
    }
    else if (attrname == "N") {
      mesh->SetVertexCount(in.GetVertexCount());
      mesh->AddVertexNormal();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetVertexCount(); j++) {
        const double *data = (const double *) in.GetDataBuffer();
        Vector N;
        N.x = data[3*j + 0];
        N.y = data[3*j + 1];
        N.z = data[3*j + 2];
        mesh->SetVertexNormal(j, N);
      }
    }
    else if (attrname == "Cd") {
      mesh->SetVertexCount(in.GetVertexCount());
      mesh->AddVertexColor();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetVertexCount(); j++) {
        const float *data = (const float *) in.GetDataBuffer();
        Color Cd;
        Cd.r = data[3*j + 0];
        Cd.g = data[3*j + 1];
        Cd.b = data[3*j + 2];
        mesh->SetVertexColor(j, Cd);
      }
    }
    else if (attrname == "uv") {
      mesh->SetVertexCount(in.GetVertexCount());
      mesh->AddVertexTexture();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetVertexCount(); j++) {
        const float *data = (const float *) in.GetDataBuffer();
        TexCoord texcoord;
        texcoord.u = data[2*j + 0];
        texcoord.v = data[2*j + 1];
        mesh->SetVertexTexture(j, texcoord);
      }
    }
    else if (attrname == "velocity") {
      mesh->SetVertexCount(in.GetVertexCount());
      mesh->AddVertexVelocity();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetVertexCount(); j++) {
        const double *data = (const double *) in.GetDataBuffer();
        Vector velocity;
        velocity.x = data[3*j + 0];
        velocity.y = data[3*j + 1];
        velocity.z = data[3*j + 2];
        mesh->SetVertexVelocity(j, velocity);
      }
    }
    else if (attrname == "indices") {
      mesh->SetFaceCount(in.GetFaceCount());
      mesh->AddFaceIndices();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetFaceCount(); j++) {
        const Index *data = (const Index *) in.GetDataBuffer();
        Index3 tri_index;
        tri_index.i0 = data[3*j + 0];
        tri_index.i1 = data[3*j + 1];
        tri_index.i2 = data[3*j + 2];
        mesh->SetFaceIndices(j, tri_index);
      }
    }
    else if (attrname == "face_group_id") {
      mesh->SetFaceCount(in.GetFaceCount());
      mesh->AddFaceGroupID();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetFaceCount(); j++) {
        const int *data = (const int *) in.GetDataBuffer();
        const int group_id = data[j];
        mesh->SetFaceGroupID(j, group_id);
      }
    }
  }

  mesh->ComputeBounds();

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
  write_(out->file_, &namesize, 1);
  write_(out->file_, &name[0], namesize);

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
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nverts; i++) {
      double P[3] = {0, 0, 0};
      P[0] = out->P[i].x;
      P[1] = out->P[i].y;
      P[2] = out->P[i].z;
      write_(out->file_, P, 3);
    }
  }
  else if (name == "N") {
    if (out->N == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nverts; i++) {
      double N[3] = {0, 0, 0};
      N[0] = out->N[i].x;
      N[1] = out->N[i].y;
      N[2] = out->N[i].z;
      write_(out->file_, N, 3);
    }
  }
  else if (name == "Cd") {
    if (out->Cd == NULL)
      return 0;
    datasize = 3 * sizeof(float) * out->nverts;
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nverts; i++) {
      float Cd[3] = {0, 0, 0};
      Cd[0] = out->Cd[i].r;
      Cd[1] = out->Cd[i].g;
      Cd[2] = out->Cd[i].b;
      write_(out->file_, Cd, 3);
    }
  }
  else if (name == "uv") {
    if (out->uv == NULL)
      return 0;
    datasize = 2 * sizeof(float) * out->nverts;
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nverts; i++) {
      float uv[2] = {0, 0};
      uv[0] = out->uv[i].u;
      uv[1] = out->uv[i].v;
      write_(out->file_, uv, 2);
    }
  }
  else if (name == "velocity") {
    if (out->velocity == NULL)
      return 0;
    datasize = 3 * sizeof(double) * out->nverts;
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nverts; i++) {
      double velocity[3] = {0, 0, 0};
      velocity[0] = out->velocity[i].x;
      velocity[1] = out->velocity[i].y;
      velocity[2] = out->velocity[i].z;
      write_(out->file_, velocity, 3);
    }
  }
  else if (name == "indices") {
    if (out->indices == NULL)
      return 0;
    datasize = 3 * sizeof(int) * out->nfaces;
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nfaces; i++) {
      Index indices[3] = {0, 0, 0};
      indices[0] = out->indices[i].i0;
      indices[1] = out->indices[i].i1;
      indices[2] = out->indices[i].i2;
      write_(out->file_, indices, 3);
    }
  }
  else if (name == "face_group_id") {
    if (out->face_group_id == NULL)
      return 0;
    datasize = sizeof(int) * out->nfaces;
    write_(out->file_, &datasize, 1);
    for (i = 0; i < out->nfaces; i++) {
      const int id = out->face_group_id[i];
      write_(out->file_, &id, 1);
    }
  }
  return nwrotes;
}

} // namespace xxx
