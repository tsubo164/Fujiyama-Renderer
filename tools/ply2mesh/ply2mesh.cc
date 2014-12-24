// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "ply.h"
#include "fj_tex_coord.h"
#include "fj_triangle.h"
#include "fj_mesh_io.h"
#include "fj_vector.h"
#include "fj_color.h"
#include "fj_mesh.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace fj;

static const char USAGE[] =
"Usage: ply2mesh [options] inputfile(*.ply) outputfile(*.mesh)\n"
"Property names in ply file should be the below\n"
"  x, y, z    : position\n"
"  nx, ny, nz : normals\n"
"  uv1, uv2   : uv\n"
"\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

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

int main(int argc, const char **argv)
{
  int i, j, k;
  PlyFile *in_ply;

  char *filename;
  int nelems;
  char **elem_names;
  int file_type;
  float version;

  int nverts = 0;
  int ntris = 0;
  std::vector<Vector> P;
  std::vector<Vector> N;
  std::vector<TexCoord> uv;
  int has_uv = 0;
  std::vector<Index3> index_array;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  MeshOutput out;

  out.Open(argv[2]);
  if (out.Fail()) {
    // XXX
    fprintf(stderr, "Could not open output file: %s\n", argv[2]);
    return -1;
  }

  filename = (char *) argv[1];
  in_ply = ply_open_for_reading(filename, &nelems, &elem_names, &file_type, &version);

  if (in_ply == NULL) {
    fprintf(stderr, "error: couldn't open input file: %s\n", argv[1]);
    return -1;
  }

  for (i = 0; i < nelems; i++) {
    PlyElement *elem = in_ply->elems[i];

    if (strcmp(elem->name, "vertex") == 0) {
      nverts = elem->num;
      P.resize(nverts);

      // setup vertex P properties
      ply_get_property(in_ply, elem->name, &vert_props[0]);
      ply_get_property(in_ply, elem->name, &vert_props[1]);
      ply_get_property(in_ply, elem->name, &vert_props[2]);

      for (j = 0; j < elem->nprops; j++) {
        PlyProperty *prop;
        prop = elem->props[j];
        if (strcmp("uv1", prop->name) == 0) {
          ply_get_property(in_ply, elem->name, &vert_props[9]);
          has_uv = 1;
        }
        else if (strcmp("uv2", prop->name) == 0) {
          ply_get_property(in_ply, elem->name, &vert_props[10]);
          has_uv = 1;
        }
      }
      // allocate attributes
      if (has_uv) {
        uv.resize(nverts);
      }

      // get all vertex P properties
      for (j = 0; j < nverts; j++) {
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

      for (j = 0; j < npolys; j++) {
        PlyFace face;
        ply_get_element(in_ply, &face);

        // n triangles in a polygon is (n vertices - 2)
        for (k = 0; k < face.nverts - 2; k++) {
          Index3 tri_index;
          tri_index.i0 = face.verts[0];
          tri_index.i1 = face.verts[k + 1];
          tri_index.i2 = face.verts[k + 2];
          index_array.push_back(tri_index);
          ntris++;
        }
      }
    }
  }

  // initialize N
  N.resize(nverts);
  for (i = 0; i < nverts; i++) {
    Vector *nml = &N[i];
    *nml = Vector();
  }
  // compute N
  for (i = 0; i < ntris; i++) {
    Vector *P0, *P1, *P2;
    Vector *N0, *N1, *N2;
    Vector Ng;
    Index3 *indices = &index_array[0];
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

  // setup MeshOutput
  out.SetPointCount(nverts);
  out.SetPointPosition(&P[0]);
  out.SetPointNormal(&N[0]);
  out.SetPointTexture(&uv[0]);
  out.SetFaceCount(ntris);
  out.SetFaceIndex3(&index_array[0]);

  out.WriteFile();

  // cleanup
  ply_close(in_ply);

  return 0;
}
