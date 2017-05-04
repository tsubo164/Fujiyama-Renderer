// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_triangle.h"
#include "fj_random.h"
#include "fj_noise.h"
#include <iostream>

using namespace fj;

class VelocityGeneratorProcedure : Procedure {
public:
  VelocityGeneratorProcedure() : mesh(NULL) {}
  virtual ~VelocityGeneratorProcedure() {}

public:
  Mesh *mesh;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "VelocityGeneratorProcedure";

static int set_mesh(void *self, const PropertyValue &value);

static int generate_velocity(Mesh &mesh);

static const Property MyPropertyList[] = {
  Property("mesh",    PropMesh(NULL),  set_mesh),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "Generate velocity on mesh."},
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
  VelocityGeneratorProcedure *velgen = new VelocityGeneratorProcedure();

  return velgen;
}

static void MyDeleteFunction(void *self)
{
  VelocityGeneratorProcedure *velgen = (VelocityGeneratorProcedure *) self;
  if (velgen == NULL)
    return;
  delete velgen;
}

int VelocityGeneratorProcedure::run() const
{
  if (mesh == NULL) {
    std::cout << "WARNNING: No mesh assigned.\n";
    return -1;
  }

  const int err = generate_velocity(*mesh);

  return err;
}

static int set_mesh(void *self, const PropertyValue &value)
{
  VelocityGeneratorProcedure *velgen = (VelocityGeneratorProcedure *) self;

  if (value.mesh == NULL)
    return -1;

  velgen->mesh = value.mesh;

  return 0;
}

static int generate_velocity(Mesh &mesh)
{
  const int POINT_COUNT = mesh.GetPointCount();
  const double zmin = mesh.GetBounds().min.z;
  const double zmax = mesh.GetBounds().max.z;

  printf("Point Count: %d\n", POINT_COUNT);
  mesh.AddPointVelocity();

  for (int i = 0; i < POINT_COUNT; i++) {
    const Vector pos = mesh.GetPointPosition(i);

    const double znml = (pos.z - zmin) / (zmax - zmin);
    const double vscale = .2 * (1 - SmoothStep(.2, .7, znml));

    const Vector noise_vec = PerlinNoise3d(.2 * pos, 2, .5, 1);
    const Vector vel = vscale * noise_vec;

    mesh.SetPointVelocity(i, vel);
  }

  mesh.ComputeNormals();
  mesh.ComputeBounds();

  return 0;
}
