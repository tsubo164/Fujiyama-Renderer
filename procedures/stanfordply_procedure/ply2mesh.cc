// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "ply.h"
#include "fj_tex_coord.h"
#include "fj_triangle.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_mesh.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace fj;

struct PlyVertex {
  double x,y,z;
  double nx,ny,nz;
  float r,g,b;
  float uv1, uv2;
  void *other_props;       // other properties
};

struct PlyFace {
  unsigned char nverts;    // number of vertex indices in list
  int *verts;              // vertex index list
  void *other_props;       // other properties
};

PlyProperty vert_props[] = { // list of property information for a vertex
  {(char *) "x", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,x) , 0, 0, 0, 0},
  {(char *) "y", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,y) , 0, 0, 0, 0},
  {(char *) "z", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,z) , 0, 0, 0, 0},
  {(char *) "nx", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,r) , 0, 0, 0, 0},
  {(char *) "ny", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,g) , 0, 0, 0, 0},
  {(char *) "nz", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,b) , 0, 0, 0, 0},
  {(char *) "r", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,nx), 0, 0, 0, 0},
  {(char *) "g", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,ny), 0, 0, 0, 0},
  {(char *) "b", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,nz), 0, 0, 0, 0},
  {(char *) "uv1", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,uv1), 0, 0, 0, 0},
  {(char *) "uv2", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,uv2), 0, 0, 0, 0},
};

PlyProperty face_props[] = { // list of property information for a face
  {(char *) "vertex_indices", PLY_INT, PLY_INT, offsetof(PlyFace,verts),
    1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyFace,nverts)},
};

int ReadPlyFile(const char *filename, Mesh &mesh)
{
  PlyFile *in_ply;

  int nelems;
  char **elem_names;
  int file_type;
  float version;

  int nverts = 0;
  int ntris = 0;
  bool has_uv = false;
  std::vector<Vector> P;
  std::vector<TexCoord> uv;
  std::vector<Index3> indices;

  // The API takes non-const
  in_ply = ply_open_for_reading(const_cast<char *>(filename),
      &nelems, &elem_names, &file_type, &version);

  if (in_ply == NULL) {
    fprintf(stderr, "error: couldn't open input file: %s\n", filename);
    return -1;
  }

  for (int i = 0; i < nelems; i++) {
    PlyElement *elem = in_ply->elems[i];

    if (strcmp(elem->name, "vertex") == 0) {
      nverts = elem->num;
      P.resize(nverts);

      // setup vertex P properties
      ply_get_property(in_ply, elem->name, &vert_props[0]);
      ply_get_property(in_ply, elem->name, &vert_props[1]);
      ply_get_property(in_ply, elem->name, &vert_props[2]);

      for (int j = 0; j < elem->nprops; j++) {
        PlyProperty *prop;
        prop = elem->props[j];
        if (strcmp("uv1", prop->name) == 0) {
          ply_get_property(in_ply, elem->name, &vert_props[9]);
          has_uv = true;
        }
        else if (strcmp("uv2", prop->name) == 0) {
          ply_get_property(in_ply, elem->name, &vert_props[10]);
          has_uv = true;
        }
      }
      // allocate attributes
      if (has_uv) {
        uv.resize(nverts);
      }

      // get all vertex P properties
      for (int j = 0; j < nverts; j++) {
        Vector *pt;
        PlyVertex vert;
        ply_get_element(in_ply, &vert);

        pt = &P[j];
        *pt = Vector(vert.x, vert.y, vert.z);

        if (has_uv) {
          uv[j].u = vert.uv1;
          uv[j].v = vert.uv2;
        }
      }
    }
    else if (strcmp(elem->name, "face") == 0) {
      int npolys = elem->num;
      ply_get_property(in_ply, elem->name, &face_props[0]); 

      for (int j = 0; j < npolys; j++) {
        PlyFace face;
        ply_get_element(in_ply, &face);

        // n triangles in a polygon is (n vertices - 2)
        for (int k = 0; k < face.nverts - 2; k++) {
          Index3 tri_index;
          tri_index.i0 = face.verts[0];
          tri_index.i1 = face.verts[k + 1];
          tri_index.i2 = face.verts[k + 2];
          indices.push_back(tri_index);
          ntris++;
        }
      }
    }
  }

  const int vertex_count = nverts;
  const int face_count = ntris;

  // P
  mesh.SetPointCount(vertex_count);
  mesh.AddPointPosition();
  for (int i = 0; i < vertex_count; i++) {
    mesh.SetPointPosition(i, P[i]);
  }
  // indices
  mesh.SetFaceCount(face_count);
  mesh.AddFaceIndices();
  for (int i = 0; i < face_count; i++) {
    mesh.SetFaceIndices(i, indices[i]);
  }
  // uv
  if (!uv.empty()) {
    mesh.AddPointTexture();
    for (int i = 0; i < vertex_count; i++) {
      mesh.SetPointTexture(i, uv[i]);
    }
  }

  mesh.ComputeNormals();
  mesh.ComputeBounds();

  // cleanup
  ply_close(in_ply);

  return 0;
}
