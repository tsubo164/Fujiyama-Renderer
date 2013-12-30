/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
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

#include "fj_geometry.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

static const char USAGE[] =
"Usage: ptcgen [options] inputfile(*.mesh) outputfile(*.ptc)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

/*
static void write_vec3_callback(const void *data, GeoSize element, int index,
    struct AttributeComponent *value)
{
  const struct Vector *vec = (const struct Vector *) data;

  switch (index) {
  case 0:
    value->real = vec[element].x;
    break;
  case 1:
    value->real = vec[element].y;
    break;
  case 2:
    value->real = vec[element].z;
    break;
  default:
    value->real = 0.;
    break;
  }
}
*/

int main(int argc, const char **argv)
{
  const char *in_filename = NULL;
  const char *out_filename = NULL;

  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("%s", USAGE);
    return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    fprintf(stderr, "%s", USAGE);
    return -1;
  }

  in_filename = argv[1];
  out_filename = argv[2];

  {
    /* TODO TEST Geometry */
    struct Geometry *geo = GeoNew();

    struct XorShift xr;
    struct Mesh *mesh = MshNew();
    struct PtcOutputFile *out = PtcOpenOutputFile(out_filename);
    struct Vector *P = NULL;
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
      struct Vector P0 = {0, 0, 0};
      struct Vector P1 = {0, 0, 0};
      struct Vector P2 = {0, 0, 0};
      struct Vector center = {0, 0, 0};
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
      area *= noise_val;
      npt_on_face = (int) 2000000 * area;

      total_point_count += npt_on_face;
      point_count_list[i] = npt_on_face;
    }
    printf("total_point_count %d\n", total_point_count);

    GeoSetPointCount(geo, total_point_count);
    GeoSetPrimitiveCount(geo, 1);

    P = GeoAddAttributeVector3(geo, "P", GEO_POINT);
    radius = GeoAddAttributeDouble(geo, "radius", GEO_POINT);
    /*
    P = FJ_MEM_ALLOC_ARRAY(struct Vector, total_point_count);
    radius = FJ_MEM_ALLOC_ARRAY(double, total_point_count);
    */

    XorInit(&xr);
    point_id = 0;
    for (i = 0; i < face_count; i++) {
      struct Vector P0 = {0, 0, 0};
      struct Vector P1 = {0, 0, 0};
      struct Vector P2 = {0, 0, 0};
      struct Vector N0 = {0, 0, 0};
      struct Vector N1 = {0, 0, 0};
      struct Vector N2 = {0, 0, 0};
      const int npt_on_face = point_count_list[i];
      int j;

      MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);
      MshGetFaceVertexNormal(mesh, i, &N0, &N1, &N2);

      for (j = 0; j < npt_on_face; j++) {
        struct Vector normal = {0, 0, 0};
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

        radius[point_id] = .01 * .2;

        point_id++;
      }
    }

    radius = GeoGetAttributeDouble(geo, "radius", GEO_POINT);

    PtcSetOutputPosition(out, P, total_point_count);
    PtcSetOutputAttributeDouble(out, "radius", radius);

    PtcWriteFile(out);

    PtcCloseOutputFile(out);
    MshFree(mesh);
    /* TODO TEST Geometry */
    GeoWriteFile(geo, "../test.fjgeo");
    GeoFree(geo);

    FJ_MEM_FREE(point_count_list);
    /*
    FJ_MEM_FREE(P);
    FJ_MEM_FREE(radius);
    */
  }

  {
    /* TODO TEST Geometry */
    struct Geometry *geo = GeoNew();
    int err = 0;

    /* TODO TEST Geometry */
    err = GeoReadFile(geo, "../test.fjgeo");
    if (err) {
      printf("error!\n");
    }
    GeoFree(geo);
  }

  return 0;

#if 0
  struct GeoOutputFile *geo = GeoOpenOutputFile("../test.fjgeo");
  struct Vector P[] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
  };
  double radius[] = {.01, .01, .01};

  GeoSetOutputPrimitiveType(geo, "PTCLOUD");

  GeoSetOutputPointCount(geo, 3);
  GeoSetOutputPrimitiveCount(geo, 1);

  GeoSetOutputPointAttributeVector3(geo, "P", P);
  GeoSetOutputPointAttributeDouble(geo, "radius", radius);

  GeoSetOutputAttribute(geo,
      "P",
      P,
      CLASS_POINT,
      GEO_Double,
      3,
      3,
      write_vec3_callback);

  GeoWriteFile(geo);

  GeoCloseOutputFile(geo);

  {
    struct GeoInputFile *geo = GeoOpenInputFile("../test.fjgeo");

    GeoReadHeader(geo);

    printf("point_count: %ld\n", GeoGetInputPointCount(geo));
    printf("primitive_count: %ld\n", GeoGetInputPrimitiveCount(geo));

    GeoCloseInputFile(geo);
  }

  return 0;
#endif
}
