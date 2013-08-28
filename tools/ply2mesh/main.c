/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ply.h"
#include "TexCoord.h"
#include "Triangle.h"
#include "MeshIO.h"
#include "Vector.h"
#include "Array.h"
#include "Color.h"
#include "Mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  void *other_props;       /* other properties */
};

struct PlyFace {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {(char *) "x", PLY_FLOAT, PLY_DOUBLE, offsetof(struct PlyVertex,x) , 0, 0, 0, 0},
  {(char *) "y", PLY_FLOAT, PLY_DOUBLE, offsetof(struct PlyVertex,y) , 0, 0, 0, 0},
  {(char *) "z", PLY_FLOAT, PLY_DOUBLE, offsetof(struct PlyVertex,z) , 0, 0, 0, 0},
  {(char *) "nx", PLY_FLOAT, PLY_DOUBLE, offsetof(struct PlyVertex,r) , 0, 0, 0, 0},
  {(char *) "ny", PLY_FLOAT, PLY_DOUBLE, offsetof(struct PlyVertex,g) , 0, 0, 0, 0},
  {(char *) "nz", PLY_FLOAT, PLY_DOUBLE, offsetof(struct PlyVertex,b) , 0, 0, 0, 0},
  {(char *) "r", PLY_FLOAT, PLY_FLOAT, offsetof(struct PlyVertex,nx), 0, 0, 0, 0},
  {(char *) "g", PLY_FLOAT, PLY_FLOAT, offsetof(struct PlyVertex,ny), 0, 0, 0, 0},
  {(char *) "b", PLY_FLOAT, PLY_FLOAT, offsetof(struct PlyVertex,nz), 0, 0, 0, 0},
  {(char *) "uv1", PLY_FLOAT, PLY_FLOAT, offsetof(struct PlyVertex,uv1), 0, 0, 0, 0},
  {(char *) "uv2", PLY_FLOAT, PLY_FLOAT, offsetof(struct PlyVertex,uv2), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a face */
  {(char *) "vertex_indices", PLY_INT, PLY_INT, offsetof(struct PlyFace,verts),
    1, PLY_UCHAR, PLY_UCHAR, offsetof(struct PlyFace,nverts)},
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
  struct Vector *P = NULL;
  struct Vector *N = NULL;
  struct TexCoord *uv = NULL;
  int has_uv = 0;
  struct Array *index_array;
  struct MeshOutput *out;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  if ((out = MshOpenOutputFile(argv[2])) == NULL) {
    /* XXX */
    fprintf(stderr, "Could not open output file: %s\n", argv[2]);
    return -1;
  }
  index_array = ArrNew(sizeof(struct TriIndex));

  filename = (char *) argv[1];
  in_ply = ply_open_for_reading(filename, &nelems, &elem_names, &file_type, &version);

  for (i = 0; i < nelems; i++) {
    PlyElement *elem = in_ply->elems[i];

    if (strcmp(elem->name, "vertex") == 0) {
      nverts = elem->num;
      P = VecAlloc(nverts);

      /* setup vertex P properties */
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
      /* allocate attributes */
      if (has_uv) {
        uv = TexCoordAlloc(nverts);
      }

      /* get all vertex P properties */
      for (j = 0; j < nverts; j++) {
        struct Vector *pt;
        struct PlyVertex vert;
        ply_get_element(in_ply, &vert);

        pt = &P[j];
        VEC3_SET(pt, vert.x, vert.y, vert.z);

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
        struct PlyFace face;
        ply_get_element(in_ply, &face);

        /* n triangles in a polygon is (n vertices - 2) */
        for (k = 0; k < face.nverts - 2; k++) {
          struct TriIndex tri_index = {0, 0, 0};
          tri_index.i0 = face.verts[0];
          tri_index.i1 = face.verts[k + 1];
          tri_index.i2 = face.verts[k + 2];
          ArrPush(index_array, &tri_index);
          ntris++;
        }
      }
    }
  }

  /* initialize N */
  N = VecAlloc(nverts);
  for (i = 0; i < nverts; i++) {
    struct Vector *nml = &N[i];
    VEC3_SET(nml, 0, 0, 0);
  }
  /* compute N */
  for (i = 0; i < ntris; i++) {
    struct Vector *P0, *P1, *P2;
    struct Vector *N0, *N1, *N2;
    struct Vector Ng;
    struct TriIndex *indices = (struct TriIndex *) index_array->data;
    const int i0 = indices[i].i0;
    const int i1 = indices[i].i1;
    const int i2 = indices[i].i2;

    P0 = &P[i0];
    P1 = &P[i1];
    P2 = &P[i2];
    N0 = &N[i0];
    N1 = &N[i1];
    N2 = &N[i2];

    TriComputeFaceNormal(&Ng, P0, P1, P2);

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

  /* setup MshOutput */
  out->nverts = nverts;
  out->nvert_attrs = 2;
  out->P = P;
  out->N = N;
  out->Cd = NULL;
  out->uv = uv;
  out->nfaces = ntris;
  out->nface_attrs = 1;
  out->indices = (struct TriIndex *) index_array->data;

  MshWriteFile(out);

  /* cleanup */
  ply_close(in_ply);
  MshCloseOutputFile(out);
  ArrFree(index_array);
  VecFree(P);
  VecFree(N);
  TexCoordFree(uv);

  return 0;
}

