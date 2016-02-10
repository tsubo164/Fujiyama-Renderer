// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_point_cloud_io.h"
#include "fj_point_cloud.h"
#include "fj_triangle.h"
#include "fj_numeric.h"
#include "fj_mesh_io.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_noise.h"
#include "fj_mesh.h"

#include <vector>
#include <cstdio>
#include <cctype>
#include <cstring>

using namespace fj;

static const char USAGE[] =
"Usage: ptcgen [options] inputfile(*.mesh) outputfile(*.ptc)\n"
"Options:\n"
"  --help         Display this information\n"
"  -v             Add velocity attribute\n"
"\n";

static void noise_position(Vector &P, const Vector &N, Vector &velocity);
static int save_point_cloud(
    const char *in_filename, const char *out_filename, bool add_velocity);

int main(int argc, const char **argv)
{
  const char *in_filename = NULL;
  const char *out_filename = NULL;
  bool add_velocity = false;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc == 4 && strcmp(argv[1], "-v") == 0) {
    in_filename = argv[2];
    out_filename = argv[3];
    add_velocity = true;
  }
  else if (argc == 3) {
    in_filename = argv[1];
    out_filename = argv[2];
    add_velocity = false;
  }
  else {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  save_point_cloud(in_filename, out_filename, add_velocity);
  return 0;
}

static int save_point_cloud(
    const char *in_filename, const char *out_filename, bool add_velocity)
{
  XorShift rng;
  int total_point_count = 0;
  int face_count = 0;

  Mesh mesh;
  const int err = MshLoadFile(&mesh, in_filename);
  if (err) {
    fprintf(stderr, "error: couldn't open input file: %s\n", in_filename);
    return -1;
  }

  face_count = mesh.GetFaceCount();
  std::vector<int> point_count_list(face_count);

  for (int i = 0; i < face_count; i++) {
    Vector P0, P1, P2;
    Real noise_val = 0;
    int npt_on_face = 0;

    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);

    const Vector center = (P0 + P1 + P2) / 3;
    noise_val = PerlinNoise(2.5 * center, 2, .5, 8);
    noise_val = Fit(noise_val, -.2, 1, 0, 1);

    const Real area = TriComputeArea(P0, P1, P2);

    if (add_velocity) {
      npt_on_face = (int) 2000000 * area * .02;
    } else {
      npt_on_face = (int) 2000000 * area * noise_val;
    }

    total_point_count += npt_on_face;
    point_count_list[i] = npt_on_face;
  }
  printf("total_point_count %d\n", total_point_count);

  // Output point cloud
  PointCloud ptc;
  ptc.SetPointCount(total_point_count);
  ptc.AddPointPosition();
  ptc.AddPointRadius();
  if (add_velocity) {
    ptc.AddPointVelocity();
  }

  int point_id = 0;
  for (int i = 0; i < face_count; i++) {
    Vector P0, P1, P2;
    Vector N0, N1, N2;
    const int npt_on_face = point_count_list[i];

    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);
    MshGetFacePointNormal(&mesh, i, &N0, &N1, &N2);

    for (int j = 0; j < npt_on_face; j++) {
      Vector P_out;
      Real radius = 0;

      const Real u = XorNextFloat01(&rng);
      const Real v = (1 - u) * XorNextFloat01(&rng);

      const Vector normal = TriComputeNormal(N0, N1, N2, u, v);

      const Real t = 1 - u - v;
      P_out = t * P0 + u * P1 + v * P2;

      const Real offset = -.05 * XorNextFloat01(&rng);
      P_out += offset * normal;

      if (add_velocity) {
        radius = .01 * .2 * 3;

        Vector vel;
        noise_position(P_out, normal, vel);
        ptc.SetPointVelocity(point_id, vel);
      } else {
        radius = .01 * .2;
      }

      ptc.SetPointPosition(point_id, P_out);
      ptc.SetPointRadius(point_id, radius);

      point_id++;
    }
  }

  PtcSaveFile(ptc, out_filename);

  return 0;
}

static void noise_position(Vector &P, const Vector &N, Vector &velocity)
{
  Vector noise_vec = PerlinNoise3d(P, 2, .5, 8);
  noise_vec += N;

  const Vector P_offset = P + Vector(1.234, -24.31 + .2, 123.4);

  Real noise_amp = PerlinNoise(P_offset, 2, .5, 8);
  noise_amp = Fit(noise_amp, .2, 1, 0, 1);

  P += noise_amp * noise_vec;
  velocity = .2 * noise_amp * noise_vec;
}
