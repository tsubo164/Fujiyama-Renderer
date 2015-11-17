// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_compatibility.h"
#include "fj_numeric.h"
#include "fj_mesh_io.h"
#include "fj_vector.h"
#include "fj_noise.h"
#include "fj_mesh.h"
#include "fj_box.h"

#include <vector>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cfloat>

using namespace fj;

static const char USAGE[] =
"Add velocity attribute to mesh. The values are based on 3d noise.\n"
"Usage: velgen [options] inputfile(*.mesh) outputfile(*.mesh)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
  const char *infile;
  const char *outfile;

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

  MeshOutput out;

  out.Open(outfile);
  if (out.Fail()) {
    // TODO ERROR HANDLING
    fprintf(stderr, "Could not open output file: %s\n", argv[2]);
    return -1;
  }

  Mesh mesh;

  if (MshLoadFile(&mesh, infile)) {
    const char *err_msg = NULL;

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

  nverts = mesh.GetPointCount();
  printf("nverts: %d\n", nverts);
  nfaces = mesh.GetFaceCount();
  printf("nfaces: %d\n", nfaces);

  std::vector<Vector> P(nverts);
  std::vector<Vector> N(nverts);
  std::vector<Vector> velocity(nverts);
  std::vector<Index3> indices(nfaces);

  {
    Box bounds;
    bounds.ReverseInfinite();

    for (i = 0; i < nverts; i++) {
      Vector pos;
      pos = mesh.GetPointPosition(i);
      BoxAddPoint(&bounds, pos);
    }
    zmin = bounds.min.z;
    zmax = bounds.max.z;
  }

  for (i = 0; i < nverts; i++) {
    Vector Q;
    Vector noise_vec;
    Vector pos;
    Vector nml;
    Vector vel;
    double vscale = 1;
    double freq = .2;
    double znml = 0;

    pos = mesh.GetPointPosition(i);
    nml = mesh.GetPointNormal(i);

    znml = (pos.z - zmin) / (zmax - zmin);

    vscale = 1 - SmoothStep(znml, .2, .7);
    vscale *= .2;

    Q.x = pos.x * freq;
    Q.y = pos.y * freq;
    Q.z = pos.z * freq;
    noise_vec = PerlinNoise3d(Q, 2, .5, 1);

    vel.x = vscale * noise_vec.x;
    vel.y = vscale * noise_vec.y;
    vel.z = vscale * noise_vec.z;

    P[i] = pos;
    N[i] = nml;
    velocity[i] = vel;
  }

  for (i = 0; i < nfaces; i++) {
    indices[i] = mesh.GetFaceIndices(i);
  }

  // setup MshOutput
  out.SetPointCount(nverts);
  out.SetPointPosition(&P[0]);
  out.SetPointNormal(&N[0]);
  out.SetPointVelocity(&velocity[0]);
  out.SetFaceCount(nfaces);
  out.SetFaceIndex3(&indices[0]);

  out.WriteFile();

  return 0;
}
