// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_volume_filling.h"
#include "fj_turbulence.h"
#include "fj_progress.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_volume.h"

#include <cstring>
#include <cfloat>

using namespace fj;

class CloudVolumeProcedure {
public:
  CloudVolumeProcedure() : volume(NULL), turbulence(NULL) {}
  ~CloudVolumeProcedure() {}

public:
  Volume *volume;
  Turbulence *turbulence;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "PointCloudsProcedure";
static const ProcedureFunctionTable MyFunctionTable = {
  MyRun
};

static int set_volume(void *self, const PropertyValue *value);
static int set_turbulence(void *self, const PropertyValue *value);

static int FillWithPointClouds(Volume *volume,
    const CloudControlPoint *cp, const Turbulence *turbulence);

static const Property MyProperties[] = {
  {PROP_VOLUME,     "volume",     {0, 0, 0, 0}, set_volume},
  {PROP_TURBULENCE, "turbulence", {0, 0, 0, 0}, set_turbulence},
  {PROP_NONE,       NULL,         {0, 0, 0, 0}, NULL}
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A cloud volume procedure."},
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
      MyNew,
      MyFree,
      &MyFunctionTable,
      MyProperties,
      MyMetainfo);
}
} // extern "C"

static void *MyNew(void)
{
  CloudVolumeProcedure *cloud = new CloudVolumeProcedure();

  return cloud;
}

static void MyFree(void *self)
{
  CloudVolumeProcedure *cloud = (CloudVolumeProcedure *) self;
  if (cloud == NULL)
    return;
  delete cloud;
}

static int MyRun(void *self)
{
  CloudVolumeProcedure *cloud = (CloudVolumeProcedure *) self;
  CloudControlPoint cp;
  int err = 0;

  if (cloud->volume == NULL) {
    return -1;
  }

  cp.orig = Vector(0, 0, 0);
  cp.udir = Vector(1, 0, 0);
  cp.vdir = Vector(0, 1, 0);
  cp.wdir = Vector(0, 0, 1);
  cp.noise_space = Vector(0, 0, 0);
  cp.density = 5;
  cp.radius = .75;
  cp.noise_amplitude = 1;

  err = FillWithPointClouds(cloud->volume, &cp, cloud->turbulence);

  return err;
}

static int set_volume(void *self, const PropertyValue *value)
{
  CloudVolumeProcedure *cloud = (CloudVolumeProcedure *) self;

  if (value->volume == NULL)
    return -1;

  cloud->volume = value->volume;

  return 0;
}

static int set_turbulence(void *self, const PropertyValue *value)
{
  CloudVolumeProcedure *cloud = (CloudVolumeProcedure *) self;

  if (value->turbulence == NULL)
    return -1;

  cloud->turbulence = value->turbulence;

  return 0;
}

static int FillWithPointClouds(Volume *volume,
    const CloudControlPoint *cp, const Turbulence *turbulence)

{
  // based on Production Volume Rendering (SIGGRAPH 2011) Course notes
  int i, j, k;
  int xres, yres, zres;
  int xmin, ymin, zmin;
  int xmax, ymax, zmax;

  double thresholdwidth = 0;

  // TODO come up with the best place to put progress
  Progress progress;

  volume->GetResolution(&xres, &yres, &zres);
  thresholdwidth = .5 * volume->GetFilterSize();

  VolGetIndexRange(volume, &cp->orig, cp->radius * 1.5,
      &xmin, &ymin, &zmin,
      &xmax, &ymax, &zmax);

  progress.Start(
      (xmax - xmin + 1) *
      (ymax - ymin + 1) *
      (zmax - zmin + 1));

  for (k = zmin; k <= zmax; k++) {
    for (j = ymin; j <= ymax; j++) {
      for (i = xmin; i <= xmax; i++) {
        double sphere_func = 0;
        double noise_func = 0;
        double pyro_func = 0;
        double distance = 0;

        Vector cell_center;
        Vector P_local_space;
        Vector P_noise_space;
        float pyro_value = 0;
        float value = 0;

        cell_center = volume->IndexToPoint(i, j, k);
        P_local_space.x =  cell_center.x - cp->orig.x;
        P_local_space.y =  cell_center.y - cp->orig.y;
        P_local_space.z =  cell_center.z - cp->orig.z;
        distance = Length(P_local_space);

        if (distance < cp->radius - thresholdwidth) {
          value = volume->GetValue(i, j, k);
          volume->SetValue(i, j, k, Max(value, cp->density));
          progress.Increment();
          continue;
        }

        P_noise_space = P_local_space;
        Normalize(&P_noise_space);
        P_noise_space.x += cp->noise_space.x;
        P_noise_space.y += cp->noise_space.y;
        P_noise_space.z += cp->noise_space.z;

        noise_func = turbulence->Evaluate(P_noise_space);
        noise_func = Abs(noise_func);
        noise_func = Gamma(noise_func, .5);
        noise_func *= cp->noise_amplitude;

        sphere_func = distance - cp->radius;
        pyro_func = sphere_func - noise_func;
        pyro_value = Fit(pyro_func, -thresholdwidth, thresholdwidth, 1, 0);
        pyro_value *= cp->density;

        value = volume->GetValue(i, j, k);
        volume->SetValue(i, j, k, Max(value, pyro_value));

        progress.Increment();
      }
    }
  }
  progress.Done();

  return 0;
}
