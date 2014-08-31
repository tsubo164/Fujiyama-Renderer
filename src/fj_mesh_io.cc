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
    version_(MSH_FILE_VERSION),
    nverts_(0),
    nvert_attrs_(0),
    nfaces_(0),
    nface_attrs_(0),
    P_(NULL),
    N_(NULL),
    Cd_(NULL),
    uv_(NULL),
    velocity_(NULL),
    indices_(NULL),
    face_group_id_(NULL)
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
  nverts_ = count;
}

void MeshOutput::SetVertexPosition(const Vector *position)
{
  if (P_ == NULL && position != NULL) {
    nvert_attrs_++;
  }
  P_ = position;
}

void MeshOutput::SetVertexNormal(const Vector *normal)
{
  if (N_ == NULL && normal != NULL) {
    nvert_attrs_++;
  }
  N_ = normal;
}

void MeshOutput::SetVertexColor(const Color *color)
{
  if (Cd_ == NULL && color != NULL) {
    nvert_attrs_++;
  }
  Cd_ = color;
}

void MeshOutput::SetVertexTexture(const TexCoord *texcoord)
{
  if (uv_ == NULL && texcoord != NULL) {
    nvert_attrs_++;
  }
  uv_ = texcoord;
}

void MeshOutput::SetVertexVelocity(const Vector *vel)
{
  if (velocity_ == NULL && vel != NULL) {
    nvert_attrs_++;
  }
  velocity_ = vel;
}

void MeshOutput::SetFaceCount(int count)
{
  if (count < 0) {
    return;
  }
  nfaces_ = count;
}

void MeshOutput::SetFaceIndex3(const Index3 *index)
{
  if (indices_ == NULL && index != NULL) {
    nface_attrs_++;
  }
  indices_ = index;
}

void MeshOutput::SetFaceGroupID(const int *id)
{
  if (face_group_id_ == NULL && id != NULL) {
    nface_attrs_++;
  }
  face_group_id_ = id;
}

void MeshOutput::WriteFile()
{
  char magic[] = MSH_FILE_MAGIC;

  write_(file_, magic, MSH_MAGIC_SIZE);
  write_(file_, &version_,     1);
  write_(file_, &nverts_,      1);
  write_(file_, &nvert_attrs_, 1);
  write_(file_, &nfaces_,      1);
  write_(file_, &nface_attrs_, 1);

  write_attribute_name("P");
  write_attribute_name("N");
  write_attribute_name("Cd");
  write_attribute_name("uv");
  write_attribute_name("velocity");
  write_attribute_name("indices");
  write_attribute_name("face_group_id");

  write_attribute_data("P");
  write_attribute_data("N");
  write_attribute_data("Cd");
  write_attribute_data("uv");
  write_attribute_data("velocity");
  write_attribute_data("indices");
  write_attribute_data("face_group_id");
}

void MeshOutput::write_attribute_name(const std::string &name)
{
  if (name == "P" && P_ == NULL) {
    return;
  }
  else if (name == "N" && N_ == NULL) {
    return;
  }
  else if (name == "Cd" && Cd_ == NULL) {
    return;
  }
  else if (name == "uv" && uv_ == NULL) {
    return;
  }
  else if (name == "velocity" && velocity_ == NULL) {
    return;
  }
  else if (name == "indices" && indices_ == NULL) {
    return;
  }
  else if (name == "face_group_id" && face_group_id_ == NULL) {
    return;
  }
  else {
  }

  const size_t namesize = name.length() + 1;

  write_(file_, &namesize, 1);
  write_(file_, &name[0], namesize);
}

void MeshOutput::write_attribute_data(const std::string &name)
{
  if (name == "P") {
    if (P_ == NULL)
      return;
    const size_t datasize = 3 * sizeof(double) * nverts_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nverts_; i++) {
      double pos[3] = {0, 0, 0};
      pos[0] = P_[i].x;
      pos[1] = P_[i].y;
      pos[2] = P_[i].z;
      write_(file_, pos, 3);
    }
  }
  else if (name == "N") {
    if (N_ == NULL)
      return;
    const size_t datasize = 3 * sizeof(double) * nverts_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nverts_; i++) {
      double nml[3] = {0, 0, 0};
      nml[0] = N_[i].x;
      nml[1] = N_[i].y;
      nml[2] = N_[i].z;
      write_(file_, nml, 3);
    }
  }
  else if (name == "Cd") {
    if (Cd_ == NULL)
      return;
    const size_t datasize = 3 * sizeof(float) * nverts_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nverts_; i++) {
      float col[3] = {0, 0, 0};
      col[0] = Cd_[i].r;
      col[1] = Cd_[i].g;
      col[2] = Cd_[i].b;
      write_(file_, col, 3);
    }
  }
  else if (name == "uv") {
    if (uv_ == NULL)
      return;
    const size_t datasize = 2 * sizeof(float) * nverts_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nverts_; i++) {
      float texcoord[2] = {0, 0};
      texcoord[0] = uv_[i].u;
      texcoord[1] = uv_[i].v;
      write_(file_, texcoord, 2);
    }
  }
  else if (name == "velocity") {
    if (velocity_ == NULL)
      return;
    const size_t datasize = 3 * sizeof(double) * nverts_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nverts_; i++) {
      double vel[3] = {0, 0, 0};
      vel[0] = velocity_[i].x;
      vel[1] = velocity_[i].y;
      vel[2] = velocity_[i].z;
      write_(file_, vel, 3);
    }
  }
  else if (name == "indices") {
    if (indices_ == NULL)
      return;
    const size_t datasize = 3 * sizeof(int) * nfaces_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nfaces_; i++) {
      Index idx[3] = {0, 0, 0};
      idx[0] = indices_[i].i0;
      idx[1] = indices_[i].i1;
      idx[2] = indices_[i].i2;
      write_(file_, idx, 3);
    }
  }
  else if (name == "face_group_id") {
    if (face_group_id_ == NULL)
      return;
    const size_t datasize = sizeof(int) * nfaces_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < nfaces_; i++) {
      const int id = face_group_id_[i];
      write_(file_, &id, 1);
    }
  }
}

int MshLoadFile(Mesh *mesh, const char *filename)
{
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

} // namespace xxx
