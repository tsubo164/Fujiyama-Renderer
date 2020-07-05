// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef OBJ_BUFFER_H
#define OBJ_BUFFER_H

#include "ObjParser.h"
#include "fj_tex_coord.h"
#include "fj_triangle.h"
#include "fj_vector.h"
#include "fj_types.h"
#include "fj_mesh.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <cstring>

using namespace fj;

class ObjBuffer : public obj::ObjParser {
public:
  ObjBuffer() :
      vertex_count(0), face_count(0),
      current_group_id(0)
  {
    // default group name and id
    group_name_to_id[""] = 0;
  }
  virtual ~ObjBuffer() {}

public:
  long vertex_count;
  long face_count;

  std::vector<Vector>   vertex_position;
  std::vector<Vector>   vertex_normal;
  std::vector<TexCoord> vertex_texture;
  std::vector<Index3>   position_indices;
  std::vector<Index3>   texture_indices;
  std::vector<Index3>   normal_indices;

  std::vector<Vector>   point_normal;

  std::vector<int>      face_group_id;
  std::map<std::string, int> group_name_to_id;
  int current_group_id;

private:
  // overriding callbacks
  virtual void read_v (int ncomponents, double x, double y, double z, double w)
  {
    vertex_position.push_back(Vector(x, y, z));
    vertex_count++;
  }
  virtual void read_vt(int ncomponents, double x, double y, double z, double w)
  {
    vertex_texture.push_back(TexCoord(x, y));
  }
  virtual void read_vn(int ncomponents, double x, double y, double z, double w)
  {
    vertex_normal.push_back(Vector(x, y, z));
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
        position_indices.push_back(tri_index);
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

    face_count += ntriangles;
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

extern int ObjBufferToMesh(const ObjBuffer &buffer, Mesh &mesh);
extern int ObjBufferComputeNormals(ObjBuffer &buffer);

#endif // XXX_H
