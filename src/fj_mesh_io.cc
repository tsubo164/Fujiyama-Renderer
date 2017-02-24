// Copyright (c) 2011-2017 Hiroshi Tsubokawa
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
    vertex_attr_count_(0),
    point_count_(0),
    point_attr_count_(0),
    face_count_(0),
    face_attr_count_(0),

    face_group_count_(0)
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

  read_(file_, &vertex_attr_count_, 1);
  read_(file_, &point_count_,       1);
  read_(file_, &point_attr_count_,  1);
  read_(file_, &face_count_,        1);
  read_(file_, &face_attr_count_,   1);

  read_(file_, &face_group_count_,  1);

  const int TOTAL_ATTR_COUNT = GetPointAttributeCount() + GetFaceAttributeCount()
      + GetVertexAttributeCount();
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

int MeshInput::GetVertexAttributeCount() const
{
  return vertex_attr_count_;
}

int MeshInput::GetPointCount() const
{
  return point_count_;
}

int MeshInput::GetPointAttributeCount() const
{
  return point_attr_count_;
}

int MeshInput::GetFaceCount() const
{
  return face_count_;
}

int MeshInput::GetFaceAttributeCount() const
{
  return face_attr_count_;
}

int MeshInput::GetFaceGroupCount() const
{
  return face_group_count_;
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

inline void write(std::ofstream &file, const std::string &src)
{
  const std::string &name = src;
  const size_t namesize = name.length() + 1;

  write_(file, &namesize, 1);
  write_(file, &name[0], namesize);
}

MeshOutput::MeshOutput() :
    version_(MSH_FILE_VERSION),
    vertex_attr_count_(0),
    point_count_(0),
    point_attr_count_(0),
    face_count_(0),
    face_attr_count_(0),

    face_group_count_(0),

    P_(NULL),
    N_(NULL),
    Cd_(NULL),
    uv_(NULL),
    velocity_(NULL),
    indices_(NULL),
    face_group_id_(NULL),

    face_group_name_(NULL),

    vertex_normal_value_(NULL),
    vertex_normal_index_(NULL),
    vertex_normal_value_count_(0),
    vertex_normal_index_count_(0)
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

void MeshOutput::SetPointCount(int count)
{
  if (count < 0) {
    return;
  }
  point_count_ = count;
}

void MeshOutput::SetPointPosition(const Vector *position)
{
  if (P_ == NULL && position != NULL) {
    point_attr_count_++;
  }
  P_ = position;
}

void MeshOutput::SetPointNormal(const Vector *normal)
{
  if (N_ == NULL && normal != NULL) {
    point_attr_count_++;
  }
  N_ = normal;
}

void MeshOutput::SetPointColor(const Color *color)
{
  if (Cd_ == NULL && color != NULL) {
    point_attr_count_++;
  }
  Cd_ = color;
}

void MeshOutput::SetPointTexture(const TexCoord *texcoord)
{
  if (uv_ == NULL && texcoord != NULL) {
    point_attr_count_++;
  }
  uv_ = texcoord;
}

void MeshOutput::SetPointVelocity(const Vector *vel)
{
  if (velocity_ == NULL && vel != NULL) {
    point_attr_count_++;
  }
  velocity_ = vel;
}

void MeshOutput::SetFaceCount(int count)
{
  if (count < 0) {
    return;
  }
  face_count_ = count;
}

void MeshOutput::SetFaceIndex3(const Index3 *index)
{
  if (indices_ == NULL && index != NULL) {
    face_attr_count_++;
  }
  indices_ = index;
}

void MeshOutput::SetFaceGroupID(const int *id)
{
  if (face_group_id_ == NULL && id != NULL) {
    face_attr_count_++;
  }
  face_group_id_ = id;
}

void MeshOutput::SetVertexNormal(
    const Vector *value, int value_count,
    const Index3 *index, int index_count)
{
  if (vertex_normal_value_ == NULL && value != NULL) {
    vertex_attr_count_++;
  }
  vertex_normal_value_ = value;
  vertex_normal_index_ = index;
  vertex_normal_value_count_ = value_count;
  vertex_normal_index_count_ = index_count;
}

void MeshOutput::SetFaceGroupName(const std::string *name)
{
  if (face_group_name_ == NULL && name != NULL) {
    face_attr_count_++;
  }
  face_group_name_ = name;
}

void MeshOutput::SetFaceGroupNameCount(int count)
{
  if (count < 0) {
    return;
  }
  face_group_count_ = count;
}

void MeshOutput::WriteFile()
{
  char magic[] = MSH_FILE_MAGIC;

  write_(file_, magic, MSH_MAGIC_SIZE);
  write_(file_, &version_,           1);
  write_(file_, &vertex_attr_count_, 1);
  write_(file_, &point_count_,       1);
  write_(file_, &point_attr_count_,  1);
  write_(file_, &face_count_,        1);
  write_(file_, &face_attr_count_,   1);

  write_(file_, &face_group_count_,  1);

  write_attribute_name("P");
  write_attribute_name("N");
  write_attribute_name("Cd");
  write_attribute_name("uv");
  write_attribute_name("velocity");
  write_attribute_name("indices");
  write_attribute_name("face_group_id");

  write_attribute_name("face_group_name");

  write_attribute_name("vertex_normal");

  write_attribute_data("P");
  write_attribute_data("N");
  write_attribute_data("Cd");
  write_attribute_data("uv");
  write_attribute_data("velocity");
  write_attribute_data("indices");
  write_attribute_data("face_group_id");

  write_attribute_data("face_group_name");

  write_attribute_data("vertex_normal");
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
  else if (name == "face_group_name" && face_group_name_ == NULL) {
    return;
  }
  else if (name == "vertex_normal" && vertex_normal_value_ == NULL) {
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
    const size_t datasize = 3 * sizeof(double) * point_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < point_count_; i++) {
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
    const size_t datasize = 3 * sizeof(double) * point_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < point_count_; i++) {
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
    const size_t datasize = 3 * sizeof(float) * point_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < point_count_; i++) {
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
    const size_t datasize = 2 * sizeof(float) * point_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < point_count_; i++) {
      float texcoord[2] = {0, 0};
      texcoord[0] = uv_[i].u;
      texcoord[1] = uv_[i].v;
      write_(file_, texcoord, 2);
    }
  }
  else if (name == "velocity") {
    if (velocity_ == NULL)
      return;
    const size_t datasize = 3 * sizeof(double) * point_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < point_count_; i++) {
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
    const size_t datasize = 3 * sizeof(int) * face_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < face_count_; i++) {
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
    const size_t datasize = sizeof(int) * face_count_;
    write_(file_, &datasize, 1);
    for (int i = 0; i < face_count_; i++) {
      const int id = face_group_id_[i];
      write_(file_, &id, 1);
    }
  }
  else if (name == "face_group_name") {
    if (face_group_name_ == NULL)
      return;
    size_t datasize = 0;
    for (int i = 0; i < face_group_count_; i++) {
      const std::string &name = face_group_name_[i];
      const std::size_t namesize = name.length() + 1;
      const std::size_t thisdata = sizeof(namesize) + sizeof(char) * namesize;
      datasize += thisdata;
    }
    write_(file_, &datasize, 1);
    for (int i = 0; i < face_group_count_; i++) {
      const std::string &name = face_group_name_[i];
      write(file_, name);
    }
  }
  else if (name == "vertex_normal") {
    if (vertex_normal_value_ == NULL)
      return;
    const size_t datasize =
        sizeof(vertex_normal_value_count_) +
        3 * sizeof(double) * vertex_normal_value_count_ +
        sizeof(vertex_normal_index_count_) +
        3 * sizeof(double) * vertex_normal_index_count_;
    write_(file_, &datasize, 1);

    // write vertex normal values
    write_(file_, &vertex_normal_value_count_, 1);
    for (int i = 0; i < vertex_normal_value_count_; i++) {
      double value[3] = {0, 0, 0};
      value[0] = vertex_normal_value_[i].x;
      value[1] = vertex_normal_value_[i].y;
      value[2] = vertex_normal_value_[i].z;
      write_(file_, value, 3);
    }

    // write vertex normal indices
    write_(file_, &vertex_normal_index_count_, 1);
    for (int i = 0; i < vertex_normal_index_count_; i++) {
      Index index[3] = {0, 0, 0};
      index[0] = vertex_normal_index_[i].i0;
      index[1] = vertex_normal_index_[i].i1;
      index[2] = vertex_normal_index_[i].i2;
      write_(file_, index, 3);
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

  const int TOTAL_ATTR_COUNT = in.GetPointAttributeCount() + in.GetFaceAttributeCount()
      + in.GetVertexAttributeCount();

  for (int i = 0; i < TOTAL_ATTR_COUNT; i++) {
    const std::string attrname = in.GetAttributeName(i);

    if (attrname == "P") {
      mesh->SetPointCount(in.GetPointCount());
      mesh->AddPointPosition();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetPointCount(); j++) {
        const double *data = (const double *) in.GetDataBuffer();
        Vector P;
        P.x = data[3*j + 0];
        P.y = data[3*j + 1];
        P.z = data[3*j + 2];
        mesh->SetPointPosition(j, P);
      }
    }
    else if (attrname == "N") {
      mesh->SetPointCount(in.GetPointCount());
      mesh->AddPointNormal();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetPointCount(); j++) {
        const double *data = (const double *) in.GetDataBuffer();
        Vector N;
        N.x = data[3*j + 0];
        N.y = data[3*j + 1];
        N.z = data[3*j + 2];
        mesh->SetPointNormal(j, N);
      }
    }
    else if (attrname == "Cd") {
      mesh->SetPointCount(in.GetPointCount());
      mesh->AddPointColor();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetPointCount(); j++) {
        const float *data = (const float *) in.GetDataBuffer();
        Color Cd;
        Cd.r = data[3*j + 0];
        Cd.g = data[3*j + 1];
        Cd.b = data[3*j + 2];
        mesh->SetPointColor(j, Cd);
      }
    }
    else if (attrname == "uv") {
      mesh->SetPointCount(in.GetPointCount());
      mesh->AddPointTexture();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetPointCount(); j++) {
        const float *data = (const float *) in.GetDataBuffer();
        TexCoord texcoord;
        texcoord.u = data[2*j + 0];
        texcoord.v = data[2*j + 1];
        mesh->SetPointTexture(j, texcoord);
      }
    }
    else if (attrname == "velocity") {
      mesh->SetPointCount(in.GetPointCount());
      mesh->AddPointVelocity();
      in.ReadAttributeData();
      for (int j = 0; j < in.GetPointCount(); j++) {
        const double *data = (const double *) in.GetDataBuffer();
        Vector velocity;
        velocity.x = data[3*j + 0];
        velocity.y = data[3*j + 1];
        velocity.z = data[3*j + 2];
        mesh->SetPointVelocity(j, velocity);
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
    else if (attrname == "face_group_name") {
      in.ReadAttributeData();
      const char *data = (const char *) in.GetDataBuffer();

      for (int j = 0; j < in.GetFaceGroupCount(); j++) {
        const std::size_t *sdata = (const std::size_t *) data;
        const std::size_t namesize = *sdata;
        sdata++;
        const char *cdata = (const char *) sdata;
        const std::string name(cdata);
        cdata += namesize;
        mesh->CreateFaceGroup(name);
        data = cdata;
      }
    }
    else if (attrname == "vertex_normal") {
      in.ReadAttributeData();
      const Index *data = (const Index *) in.GetDataBuffer();
      const Index value_count = *data;
      data++;

      // read vertex normal values
      const double *ddata = (const double *) data;
      Mesh::VertexAttributeAccessor<Vector> vertex_normal = mesh->GetVertexNormal();
      vertex_normal.ResizeValue(value_count);
      for (Index j = 0; j < value_count; j++) {
        const Vector value(
            ddata[0],
            ddata[1],
            ddata[2]);
        vertex_normal.SetValue(j, value);
        ddata += 3;
      }

      // read vertex normal indices
      const Index *idata = (const Index *) ddata;
      const Index index_count = *idata;
      idata++;
      vertex_normal.ResizeIndex(index_count * 3);
      for (Index j = 0; j < index_count; j++) {
        vertex_normal.SetIndex(j * 3 + 0, idata[0]);
        vertex_normal.SetIndex(j * 3 + 1, idata[1]);
        vertex_normal.SetIndex(j * 3 + 2, idata[2]);
        idata += 3;
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
