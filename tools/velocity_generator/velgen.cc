/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_compatibility.h"
#include "fj_numeric.h"
#include "fj_mesh_io.h"
#include "fj_memory.h"
#include "fj_vector.h"
#include "fj_noise.h"
#include "fj_mesh.h"
#include "fj_box.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <float.h>

using namespace fj;

static const char USAGE[] =
"Add velocity attribute to mesh. The values are based on 3d noise.\n"
"Usage: velgen [options] inputfile(*.mesh) outputfile(*.mesh)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
  struct MeshOutput *out;
  struct Mesh *mesh;
  const char *infile;
  const char *outfile;

  struct Vector *P = NULL;
  struct Vector *N = NULL;
  struct Vector *velocity = NULL;
  Index3 *indices = NULL;

  int nfaces = 0;
  int nverts = 0;
  int i;

  double zmin, zmax;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }
  infile = argv[1];
  outfile = argv[2];

  if ((out = MshOpenOutputFile(outfile)) == NULL) {
    /* TODO ERROR HANDLING */
    fprintf(stderr, "Could not open output file: %s\n", argv[2]);
    return -1;
  }

  mesh = MshNew();
  if (mesh == NULL) {
    fprintf(stderr, "fatal error: MshNew returned NULL.\n");
    return -1;
  }

  if (MshLoadFile(mesh, infile)) {
    const char *err_msg = NULL;
    MshFree(mesh);

    switch (MshGetErrorNo()) {
    case MSH_ERR_NONE:
      err_msg = "";
      break;
    case MSH_ERR_FILE_NOT_EXIST:
      err_msg = "mesh file not found";
      break;
    case MSH_ERR_BAD_MAGIC_NUMBER:
      err_msg = "invalid magic number";
      break;
    case MSH_ERR_BAD_FILE_VERSION:
      err_msg = "invalid file format version";
      break;
    case MSH_ERR_LONG_ATTRIB_NAME:
      err_msg = "too long attribute name was detected";
      break;
    case MSH_ERR_NO_MEMORY:
      err_msg = "no memory to allocate";
      break;
    default:
      err_msg = "";
      break;
    }
    fprintf(stderr, "error: %s: %s\n", err_msg, infile);
    return -1;
  }

  nverts = mesh->GetVertexCount();
  printf("nverts: %d\n", nverts);
  nfaces = mesh->GetFaceCount();
  printf("nfaces: %d\n", nfaces);

  P = VecAlloc(nverts);
  N = VecAlloc(nverts);
  velocity = VecAlloc(nverts);
  indices = FJ_MEM_ALLOC_ARRAY(Index3, nfaces);

  {
    struct Box bounds(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (i = 0; i < nverts; i++) {
      struct Vector pos;
      pos = mesh->GetVertexPosition(i);
      BoxAddPoint(&bounds, pos);
    }
    zmin = bounds.min.z;
    zmax = bounds.max.z;
  }

  for (i = 0; i < nverts; i++) {
    struct Vector Q;
    struct Vector noise_vec;
    struct Vector pos;
    struct Vector nml;
    struct Vector vel;
    double vscale = 1;
    double freq = .2;
    double znml = 0;

    pos = mesh->GetVertexPosition(i);
    nml = mesh->GetVertexNormal(i);

    znml = (pos.z - zmin) / (zmax - zmin);

    vscale = 1 - SmoothStep(znml, .2, .7);
    vscale *= .2;

    Q.x = pos.x * freq;
    Q.y = pos.y * freq;
    Q.z = pos.z * freq;
    PerlinNoise3d(&Q, 2, .5, 1, &noise_vec);

    vel.x = vscale * noise_vec.x;
    vel.y = vscale * noise_vec.y;
    vel.z = vscale * noise_vec.z;

    P[i] = pos;
    N[i] = nml;
    velocity[i] = vel;
  }

  for (i = 0; i < nfaces; i++) {
    indices[i] = mesh->GetFaceIndices(i);
  }

  /* setup MshOutput */
  out->nverts = nverts;
  out->nvert_attrs = 2;
  out->P = P;
  out->N = N;
  out->Cd = NULL;
  out->uv = NULL;
  out->velocity = velocity;
  out->nfaces = nfaces;
  out->nface_attrs = 1;
  out->indices = indices;

  MshWriteFile(out);

  /* clean up */
  MshFree(mesh);
  VecFree(P);
  VecFree(N);
  VecFree(velocity);
  FJ_MEM_FREE(indices);

  return 0;
}
