/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_point_cloud_io.h"
#include "fj_point_cloud.h"
#include "fj_triangle.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_mesh_io.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_noise.h"
#include "fj_mesh.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

using namespace fj;

static const char USAGE[] =
"Usage: ptcgen [options] inputfile(*.mesh) outputfile(*.ptc)\n"
"Options:\n"
"  --help         Display this information\n"
"  -v             Add velocity attribute\n"
"\n";

static void noise_position(struct Vector *P, const struct Vector *N, struct Vector *velocity);

int main(int argc, const char **argv)
{
  const char *in_filename = NULL;
  const char *out_filename = NULL;
  int add_velocity = 0;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc == 4 && strcmp(argv[1], "-v") == 0) {
    in_filename = argv[2];
    out_filename = argv[3];
    add_velocity = 1;
  }
  else if (argc == 3) {
    in_filename = argv[1];
    out_filename = argv[2];
    add_velocity = 0;
  }
  else {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  {
    struct XorShift xr;
    struct Mesh *mesh = MshNew();
    struct PtcOutputFile *out = PtcOpenOutputFile(out_filename);
    struct Vector *P = NULL;
    struct Vector *velocity = NULL;
    double *radius = NULL;
    int *point_count_list = 0;
    int total_point_count = 0;
    int face_count = 0;
    int point_id = 0;
    int i;

    MshLoadFile(mesh, in_filename);

    face_count = MshGetFaceCount(mesh);
    point_count_list = FJ_MEM_ALLOC_ARRAY(int, face_count);

    for (i = 0; i < face_count; i++) {
      struct Vector P0;
      struct Vector P1;
      struct Vector P2;
      struct Vector center;
      double noise_val = 0;
      double area = 0;
      int npt_on_face = 0;

      MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);

      center.x = (P0.x + P1.x + P2.x) / 3;
      center.y = (P0.y + P1.y + P2.y) / 3;
      center.z = (P0.z + P1.z + P2.z) / 3;
      center.x *= 2.5;
      center.y *= 2.5;
      center.z *= 2.5;
      noise_val = PerlinNoise(&center, 2, .5, 8);
      noise_val = Fit(noise_val, -.2, 1, 0, 1);

      area = TriComputeArea(&P0, &P1, &P2);

      if (add_velocity) {
        npt_on_face = (int) 2000000 * area * .02;
      } else {
        area *= noise_val;
        npt_on_face = (int) 2000000 * area;
      }

      total_point_count += npt_on_face;
      point_count_list[i] = npt_on_face;
    }
    printf("total_point_count %d\n", total_point_count);

    P        = VecAlloc(total_point_count);
    velocity = VecAlloc(total_point_count);
    radius   = FJ_MEM_ALLOC_ARRAY(double, total_point_count);

    XorInit(&xr);
    point_id = 0;
    for (i = 0; i < face_count; i++) {
      struct Vector P0;
      struct Vector P1;
      struct Vector P2;
      struct Vector N0;
      struct Vector N1;
      struct Vector N2;
      const int npt_on_face = point_count_list[i];
      int j;

      MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);
      MshGetFaceVertexNormal(mesh, i, &N0, &N1, &N2);

      for (j = 0; j < npt_on_face; j++) {
        struct Vector normal;
        struct Vector *P_out = &P[point_id];
        double u = 0, v = 0, t = 0;
        double offset = 0;

        u = XorNextFloat01(&xr);
        v = (1 - u) * XorNextFloat01(&xr);

        TriComputeNormal(&normal, &N0, &N1, &N2, u, v);

        t = 1 - u - v;
        P_out->x = t * P0.x + u * P1.x + v * P2.x;
        P_out->y = t * P0.y + u * P1.y + v * P2.y;
        P_out->z = t * P0.z + u * P1.z + v * P2.z;

        offset = XorNextFloat01(&xr);
        offset *= -.05;
        P_out->x += offset * normal.x;
        P_out->y += offset * normal.y;
        P_out->z += offset * normal.z;

        if (add_velocity) {
          struct Vector vel(0, .05 * 0, 0);
          noise_position(P_out, &normal, &vel);
          velocity[point_id] = vel;

          radius[point_id] = .01 * .2 * 3;
        } else {
          struct Vector vel;
          velocity[point_id] = vel;

          radius[point_id] = .01 * .2;
        }

        point_id++;
      }
    }

    PtcSetOutputPosition(out, P, total_point_count);
    PtcSetOutputAttributeDouble(out, "radius", radius);
    PtcSetOutputAttributeVector3(out, "velocity", velocity);

    PtcWriteFile(out);

    PtcCloseOutputFile(out);
    MshFree(mesh);

    FJ_MEM_FREE(point_count_list);
    VecFree(P);
    VecFree(velocity);
    FJ_MEM_FREE(radius);
  }

  return 0;
}

static void noise_position(struct Vector *P, const struct Vector *N, struct Vector *velocity)
{
  struct Vector noise_vec;
  struct Vector P_offset;
  double noise_amp = 0;

  PerlinNoise3d(P, 2, .5, 8, &noise_vec);

  noise_vec.x += N->x;
  noise_vec.y += N->y;
  noise_vec.z += N->z;

  P_offset.x = P->x + 1.234;
  P_offset.y = P->y - 24.31 + .2;
  P_offset.z = P->z + 123.4;

  noise_amp = PerlinNoise(&P_offset, 2, .5, 8);
  noise_amp = Fit(noise_amp, .2, 1, 0, 1);

  P->x += noise_amp * noise_vec.x;
  P->y += noise_amp * noise_vec.y;
  P->z += noise_amp * noise_vec.z;

  velocity->x = .2 * noise_amp * noise_vec.x;
  velocity->y = .2 * noise_amp * noise_vec.y;
  velocity->z = .2 * noise_amp * noise_vec.z;
}
