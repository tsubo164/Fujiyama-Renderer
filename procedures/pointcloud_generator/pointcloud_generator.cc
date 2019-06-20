// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_triangle.h"
#include "fj_random.h"
#include "fj_noise.h"
#include <iostream>

using namespace fj;

class PointCloudGenerator : Procedure {
public:
  PointCloudGenerator() : mesh(NULL), add_velocity(false) {}
  virtual ~PointCloudGenerator() {}

public:
  Mesh *mesh;
  PointCloud *pointcloud;
  bool add_velocity;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "PointCloudGenerator";

static int set_mesh(void *self, const PropertyValue &value);
static int set_pointcloud(void *self, const PropertyValue &value);
static int set_add_velocity(void *self, const PropertyValue &value);

static int generate_pointcloud(const Mesh &mesh, PointCloud &pointcloud, bool add_velocity);
static void noise_position(Vector &P, const Vector &N, Vector &velocity);

static const Property MyPropertyList[] = {
  Property("mesh",         PropMesh(NULL),       set_mesh),
  Property("pointcloud",   PropPointCloud(NULL), set_pointcloud),
  Property("add_velocity", PropScalar(0),        set_add_velocity),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "PointCloud Generator procedure."},
  {"plugin_type", "Procedure"},
  {NULL, NULL}
};

extern "C" {
FJ_PLUGIN_API int Initialize(PluginInfo *info)
{
  return PlgSetupInfo(info,
      PLUGIN_API_VERSION,
      PROCEDURE_PLUGIN_TYPE,
      MyPluginName,
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  PointCloudGenerator *ptcgen = new PointCloudGenerator();

  return ptcgen;
}

static void MyDeleteFunction(void *self)
{
  PointCloudGenerator *ptcgen = (PointCloudGenerator *) self;
  if (ptcgen == NULL)
    return;
  delete ptcgen;
}

int PointCloudGenerator::run() const
{
  if (mesh == NULL) {
    return -1;
  }
  if (pointcloud == NULL) {
    return -1;
  }

  const int err = generate_pointcloud(*mesh, *pointcloud, add_velocity);

  return err;
}

static int set_mesh(void *self, const PropertyValue &value)
{
  PointCloudGenerator *ptcgen = (PointCloudGenerator *) self;

  if (value.mesh == NULL)
    return -1;

  ptcgen->mesh = value.mesh;

  return 0;
}

static int set_pointcloud(void *self, const PropertyValue &value)
{
  PointCloudGenerator *ptcgen = (PointCloudGenerator *) self;

  if (value.pointcloud == NULL)
    return -1;

  ptcgen->pointcloud = value.pointcloud;

  return 0;
}

static int set_add_velocity(void *self, const PropertyValue &value)
{
  PointCloudGenerator *ptcgen = (PointCloudGenerator *) self;

  ptcgen->add_velocity = value.vector[0] > 0;

  return 0;
}

static int generate_pointcloud(const Mesh &mesh, PointCloud &pointcloud, bool add_velocity)
{
  pointcloud.Clear();

  int total_point_count = 0;

  const int face_count = mesh.GetFaceCount();
  std::vector<int> point_count_list(face_count);

  for (int i = 0; i < face_count; i++) {
    Vector P0, P1, P2;
    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);

    const Vector center = (P0 + P1 + P2) / 3;
    Real noise_val = PerlinNoise(2.5 * center, 2, .5, 8);
    noise_val = Fit(noise_val, -.2, 1, 0, 1);

    const Real area = TriComputeArea(P0, P1, P2);

    int npt_on_face = 0;
    if (add_velocity) {
      npt_on_face = static_cast<int>(2000000 * area * .02);
    } else {
      npt_on_face = static_cast<int>(2000000 * area * noise_val);
    }

    total_point_count += npt_on_face;
    point_count_list[i] = npt_on_face;
  }
  printf("total_point_count %d\n", total_point_count);

  pointcloud.SetPointCount(total_point_count);
  pointcloud.AddPointPosition();
  pointcloud.AddPointRadius();
  if (add_velocity) {
    pointcloud.AddPointVelocity();
  }

  XorShift rng;
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

      const Real u = rng.NextFloat01();
      const Real v = (1 - u) * rng.NextFloat01();

      const Vector normal = TriComputeNormal(N0, N1, N2, u, v);

      const Real t = 1 - u - v;
      P_out = t * P0 + u * P1 + v * P2;

      const Real offset = -.05 * rng.NextFloat01();
      P_out += offset * normal;

      if (add_velocity) {
        radius = .01 * .2 * 3;
        Vector vel;
        noise_position(P_out, normal, vel);
        pointcloud.SetPointVelocity(point_id, vel);
      } else {
        radius = .01 * .2;
      }

      pointcloud.SetPointPosition(point_id, P_out);
      pointcloud.SetPointRadius(point_id, radius);

      point_id++;
    }
  }
  pointcloud.ComputeBounds();

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
