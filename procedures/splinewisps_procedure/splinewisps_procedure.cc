/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_procedure.h"
#include "fj_volume_filling.h"
#include "fj_turbulence.h"
#include "fj_progress.h"
#include "fj_numeric.h"
#include "fj_memory.h"
#include "fj_random.h"
#include "fj_vector.h"
#include "fj_volume.h"

#include <string.h>
#include <float.h>

struct SplineWispsProcedure {
  struct Volume *volume;
  const struct Turbulence *turbulence;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "SplineWispsProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
  MyRun
};

static int set_volume(void *self, const struct PropertyValue *value);
static int set_turbulence(void *self, const struct PropertyValue *value);

static int FillWithSpecksAlongLine(struct Volume *volume,
    const struct WispsControlPoint *cp0, const struct WispsControlPoint *cp1,
    const struct Turbulence *turbulence);

static const struct Property MyProperties[] = {
  {PROP_VOLUME,     "volume",     {0, 0, 0, 0}, set_volume},
  {PROP_TURBULENCE, "turbulence", {0, 0, 0, 0}, set_turbulence},
  {PROP_NONE,       NULL,         {0, 0, 0, 0}, NULL}
};

static const struct MetaInfo MyMetainfo[] = {
  {"help", "A spline volume procedure."},
  {"plugin_type", "Procedure"},
  {NULL, NULL}
};

extern "C" {
int Initialize(struct PluginInfo *info)
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
  struct SplineWispsProcedure *spline;

  spline = FJ_MEM_ALLOC(struct SplineWispsProcedure);
  if (spline == NULL)
    return NULL;

  spline->volume = NULL;
  spline->turbulence = NULL;

  return spline;
}

static void MyFree(void *self)
{
  struct SplineWispsProcedure *spline = (struct SplineWispsProcedure *) self;
  if (spline == NULL)
    return;
  FJ_MEM_FREE(spline);
}

static int MyRun(void *self)
{
  struct SplineWispsProcedure *spline = (struct SplineWispsProcedure *) self;
  struct WispsControlPoint cp0, cp1;
  int err = 0;

  if (spline->volume == NULL) {
    return -1;
  }

  VEC3_SET(&cp0.orig, -.75, -.5, .75);
  VEC3_SET(&cp0.udir, 1, 0, 0);
  VEC3_SET(&cp0.vdir, 0, 1, 0);
  VEC3_SET(&cp0.wdir, 0, 0, 1);
  VEC3_SET(&cp0.noise_space, 0, 0, 0);
  cp0.density = 1;
  cp0.radius = .5;
  cp0.noise_amplitude = 1;
  cp0.speck_count = 100000 * 100;
  cp0.speck_radius = .01 * .5;

  VEC3_SET(&cp1.orig, .75, .5, -.75);
  VEC3_SET(&cp1.udir, 1, 0, 0);
  VEC3_SET(&cp1.vdir, 0, 1, 0);
  VEC3_SET(&cp1.wdir, 0, 0, 1);
  VEC3_SET(&cp1.noise_space, 0, 0, 1);
  cp1.density = 1;
  cp1.radius = .25;
  cp1.noise_amplitude = 1;
  cp1.speck_count = 100000 * 100;
  cp1.speck_radius = .01 * .5;

  cp0.wdir.x = cp1.orig.x - cp0.orig.x;
  cp0.wdir.y = cp1.orig.y - cp0.orig.y;
  cp0.wdir.z = cp1.orig.z - cp0.orig.z;
  VEC3_CROSS(&cp0.udir, &cp0.wdir, &cp0.vdir);
  VEC3_CROSS(&cp0.vdir, &cp0.udir, &cp0.wdir);
  VEC3_NORMALIZE(&cp0.udir);
  VEC3_NORMALIZE(&cp0.vdir);
  VEC3_NORMALIZE(&cp0.wdir);

  cp1.udir = cp0.udir;
  cp1.vdir = cp0.vdir;
  cp1.wdir = cp0.wdir;

  err = FillWithSpecksAlongLine(spline->volume, &cp0, &cp1, spline->turbulence);

  return err;
}

static int set_volume(void *self, const struct PropertyValue *value)
{
  struct SplineWispsProcedure *spline = (struct SplineWispsProcedure *) self;

  if (value->volume == NULL)
    return -1;

  spline->volume = value->volume;

  return 0;
}

static int set_turbulence(void *self, const struct PropertyValue *value)
{
  struct SplineWispsProcedure *spline = (struct SplineWispsProcedure *) self;

  if (value->turbulence == NULL)
    return -1;

  spline->turbulence = value->turbulence;

  return 0;
}

static int FillWithSpecksAlongLine(struct Volume *volume,
    const struct WispsControlPoint *cp0, const struct WispsControlPoint *cp1,
    const struct Turbulence *turbulence)
{
  struct XorShift xr;
  int NSPECKS = 1000;
  int i = 0;

  /* TODO come up with the best place to put progress */
  struct Progress progress;

  XorInit(&xr);
  /* TODO should not be a point attribute? */
  NSPECKS = cp0->speck_count;

  PrgStart(&progress, NSPECKS);

  for (i = 0; i < NSPECKS; i++) {
    struct WispsControlPoint cp_t;
    struct Vector2 disk = {0, 0};
    struct Vector P_speck = {0, 0, 0};
    struct Vector P_noise_space = {0, 0, 0};
    struct Vector noise = {0, 0, 0};
    double line_t = 0;

    /*
    XorHollowDiskRand(&xr, disk);
    XorGaussianDiskRand(&xr, disk);
    */
    XorSolidDiskRand(&xr, &disk);
    line_t = XorNextFloat01(&xr);

    LerpWispConstrolPoint(&cp_t, cp0, cp1, line_t);

    P_speck = cp_t.orig;
    P_speck.x += cp_t.radius * disk.x * cp_t.udir.x + cp_t.radius * disk.y * cp_t.vdir.x;
    P_speck.y += cp_t.radius * disk.x * cp_t.udir.y + cp_t.radius * disk.y * cp_t.vdir.y;
    P_speck.z += cp_t.radius * disk.x * cp_t.udir.z + cp_t.radius * disk.y * cp_t.vdir.z;

    P_noise_space.x = cp_t.noise_space.x + disk.x;
    P_noise_space.y = cp_t.noise_space.y + disk.y;
    P_noise_space.z = cp_t.noise_space.z;
    TrbEvaluate3d(turbulence, &P_noise_space, &noise);

    noise.x *= cp_t.radius * cp_t.noise_amplitude;
    noise.y *= cp_t.radius * cp_t.noise_amplitude;
    noise.z *= 1;

    P_speck.x += noise.x * cp_t.udir.x + noise.y * cp_t.vdir.x + noise.z * cp_t.wdir.x;
    P_speck.y += noise.x * cp_t.udir.y + noise.y * cp_t.vdir.y + noise.z * cp_t.wdir.y;
    P_speck.z += noise.x * cp_t.udir.z + noise.y * cp_t.vdir.z + noise.z * cp_t.wdir.z;

    FillWithSphere(volume, &P_speck, cp_t.speck_radius, cp_t.density);
    PrgIncrement(&progress);
  }
  PrgDone(&progress);

  return 0;
}

