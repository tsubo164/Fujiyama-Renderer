// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "ObjParser.h"
#include "fj_tex_coord.h"
#include "fj_triangle.h"
#include "fj_mesh_io.h"
#include "fj_vector.h"
#include "fj_mesh.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <cstring>

using namespace fj;

static const char USAGE[] =
"Usage: obj2mesh [options] inputfile(*.obj) outputfile(*.mesh)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

class ObjBuffer : public obj::ObjParser {
public:
  ObjBuffer() :
      nverts(0), nfaces(0),
      current_group_id(0)
  {
    // default group name and id
    group_name_to_id[""] = 0;
  }
  virtual ~ObjBuffer() {}

public:
  long nverts;
  long nfaces;

  std::vector<Vector>   P;
  std::vector<Vector>   N;
  std::vector<TexCoord> uv;
  std::vector<Index3>   vertex_indices;
  std::vector<Index3>   texture_indices;
  std::vector<Index3>   normal_indices;

  std::vector<int>      face_group_id;
  std::map<std::string, int> group_name_to_id;
  int current_group_id;

private:
  // overriding callbacks
  virtual void read_v (int ncomponents, double x, double y, double z, double w)
  {
    P.push_back(Vector(x, y, z));
    nverts++;
  }
  virtual void read_vt(int ncomponents, double x, double y, double z, double w)
  {
    uv.push_back(TexCoord(x, y));
  }
  virtual void read_vn(int ncomponents, double x, double y, double z, double w)
  {
    // TODO IMPLEMENT THIS
    //N.push_back(Vector(x, y, z));
  }

  virtual void read_f(long index_count,
    const long *v_indices,
    const long *vt_indices,
    const long *vn_indices)
  {
    const int ntriangles = index_count - 2;

    for (int i = 0; i < ntriangles; i++) {
      if (v_indices != NULL) {
        const Index3 tri_index(
            v_indices[0],
            v_indices[i + 1],
            v_indices[i + 2]);
        vertex_indices.push_back(tri_index);
      }

      if (vt_indices != NULL) {
        const Index3 tri_index(
            vt_indices[0],
            vt_indices[i + 1],
            vt_indices[i + 2]);
        texture_indices.push_back(tri_index);
      }

      if (vn_indices != NULL) {
        const Index3 tri_index(
            vn_indices[0],
            vn_indices[i + 1],
            vn_indices[i + 2]);
        normal_indices.push_back(tri_index);
      }

      set_face_group_id();
    }

    nfaces += ntriangles;
  }

  virtual void read_g(const std::vector<std::string> &group_name_list)
  {
#if 0
    if (nfaces > 0 && face_group_id.empty()) {
      // fill the previous faces with default_group_id
      const int default_group_id = 0;
      group_name_to_id[""] = default_group_id;
      face_group_id.resize(nfaces, default_group_id);
    }
#endif

    current_group_id = lookup_group_or_create_new(group_name_list[0]);
  }

  void set_face_group_id()
  {
#if 0
    if (current_group_id < 0) {
      return;
    }
#endif
    face_group_id.push_back(current_group_id);
  }

  int lookup_group_or_create_new(const std::string &group_name)
  {
    int return_id = 0;

    std::map<std::string, int>::const_iterator it = group_name_to_id.find(group_name);
    if (it == group_name_to_id.end()) {
      // the size is the next id
      return_id = group_name_to_id.size();
      group_name_to_id[group_name] = return_id;
    } else {
      return_id = it->second;
    }
    return return_id;
  }
};

extern int ObjBufferToMeshFile(ObjBuffer *buffer, const char *filename);
extern int ObjBufferComputeNormals(ObjBuffer *buffer);

int main(int argc, const char **argv)
{
  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    std::cerr << USAGE;
    return 0;
  }

  if (argc != 3) {
    std::cerr << "error: invalid number of arguments.\n";
    std::cerr << USAGE;
    return -1;
  }

  const char *in_filename = argv[1];
  const char *out_filename = argv[2];

  std::ifstream ifs(in_filename);
  if (!ifs) {
    std::cerr << "error: couldn't open input file: " << in_filename << "\n";
    return -1;
  }

  ObjBuffer buffer;
  int err = buffer.Parse(ifs);
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

  std::cout << "nverts: " << buffer.nverts << "\n";
  std::cout << "nfaces: " << buffer.nfaces << "\n";
  std::cout << "face group count: " << buffer.group_name_to_id.size() << "\n";
  for (std::map<std::string, int>::const_iterator it = buffer.group_name_to_id.begin();
    it != buffer.group_name_to_id.end(); ++it) {
    std::cout << "  name: [" << it->first << "] -> ID: " << it->second << "\n";
  }

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
  // TODO TEST
  if (!buffer->face_group_id.empty()) {
    out->face_group_id = &buffer->face_group_id[0];
  }

  MshWriteFile(out);
  MshCloseOutputFile(out);

  return 0;
}

int ObjBufferComputeNormals(ObjBuffer *buffer)
{
  const int nverts = buffer->nverts;
  const int nfaces = buffer->nfaces;
  std::vector<Vector> &P = buffer->P;
  std::vector<Vector> &N = buffer->N;
  std::vector<Index3> &indices = buffer->vertex_indices;

  if (P.empty() || indices.empty()) {
    return -1;
  }

  if (!N.empty()) {
    return 0;
  }

  N.resize(nverts);

  // accumulate N
  for (int i = 0; i < nfaces; i++) {
    const int i0 = indices[i].i0;
    const int i1 = indices[i].i1;
    const int i2 = indices[i].i2;

    const Vector Ng = TriComputeFaceNormal(P[i0], P[i1], P[i2]);

    N[i0] += Ng;
    N[i1] += Ng;
    N[i2] += Ng;
  }

  // normalize N
  for (int i = 0; i < nverts; i++) {
    Normalize(&N[i]);
  }

  return 0;
}
