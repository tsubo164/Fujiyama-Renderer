// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"

using namespace fj;

class SurfaceWispsProcedure : public Procedure {
public:
  SurfaceWispsProcedure() : volume(NULL), turbulence(NULL) {}
  virtual ~SurfaceWispsProcedure() {}

public:
  Volume *volume;
  const Turbulence *turbulence;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "SurfaceWispsProcedure";

static int set_volume(void *self, const PropertyValue &value);
static int set_turbulence(void *self, const PropertyValue &value);

static int FillWithSpecksOnSurface(Volume *volume,
    const WispsControlPoint *cp00, const WispsControlPoint *cp10,
    const WispsControlPoint *cp01, const WispsControlPoint *cp11,
    const Turbulence *turbulence);

static const Property MyPropertyList[] = {
  Property("volume",     PropVolume(NULL),     set_volume),
  Property("turbulence", PropTurbulence(NULL), set_turbulence),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "A surface volume procedure."},
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
  SurfaceWispsProcedure *surface = new SurfaceWispsProcedure();

  return surface;
}

static void MyDeleteFunction(void *self)
{
  SurfaceWispsProcedure *surface = (SurfaceWispsProcedure *) self;
  if (surface == NULL)
    return;
  delete surface;
}

int SurfaceWispsProcedure::run() const
{
  if (volume == NULL) {
    return -1;
  }

  WispsControlPoint cp00, cp10, cp01, cp11;
  cp00.orig = Vector(0, 0, 0);
  cp00.udir = Vector(1, 0, 0);
  cp00.vdir = Vector(0, 1, 0);
  cp00.wdir = Vector(0, 0, 1);
  cp00.noise_space = Vector(0, 0, 0);
  cp00.density = 1;
  cp00.radius = .5;
  cp00.noise_amplitude = 1;
  cp00.speck_count = 100000 * 100;
  cp00.speck_radius = .01;

  cp10 = cp00;
  cp01 = cp00;
  cp11 = cp00;

  cp00.orig = Vector(-.75, -.75, 0);
  cp10.orig = Vector( .75, -.75, 0);
  cp01.orig = Vector(-.75,  .75, 0);
  cp11.orig = Vector( .75,  .75, 0);

  cp00.noise_space = Vector(0, 0, 0);
  cp10.noise_space = Vector(1, 0, 0);
  cp01.noise_space = Vector(0, 1, 0);
  cp11.noise_space = Vector(1, 1, 0);

  cp11.radius = 1;

  const int err = FillWithSpecksOnSurface(volume,
      &cp00, &cp10,
      &cp01, &cp11,
      turbulence);

  return err;
}

static int set_volume(void *self, const PropertyValue &value)
{
  SurfaceWispsProcedure *surface = (SurfaceWispsProcedure *) self;

  if (value.volume == NULL)
    return -1;

  surface->volume = value.volume;

  return 0;
}

static int set_turbulence(void *self, const PropertyValue &value)
{
  SurfaceWispsProcedure *surface = (SurfaceWispsProcedure *) self;

  if (value.turbulence == NULL)
    return -1;

  surface->turbulence = value.turbulence;

  return 0;
}

static int FillWithSpecksOnSurface(Volume *volume,
    const WispsControlPoint *cp00, const WispsControlPoint *cp10,
    const WispsControlPoint *cp01, const WispsControlPoint *cp11,
    const Turbulence *turbulence)

{
  XorShift rng;
  int NSPECKS = 1000;
  int i = 0;

  // TODO come up with the best place to put progress
  Progress progress;

  // TODO should not be a point attribute?
  NSPECKS = cp00->speck_count;

  progress.Start(NSPECKS);

  for (i = 0; i < NSPECKS; i++) {
    WispsControlPoint cp_t;
    Vector P_speck;
    Vector P_noise_space;
    Vector noise;
    double s = 0;
    double t = 0;

    const Vector cube = rng.SolidCubeRand();

    s = cube.x;
    t = cube.y;

    BilerpWispConstrolPoint(&cp_t, cp00, cp10, cp01, cp11, s, t);

    P_speck = cp_t.orig;
    P_speck.x += cp_t.radius * cube.z * cp_t.wdir.x;
    P_speck.y += cp_t.radius * cube.z * cp_t.wdir.y;
    P_speck.z += cp_t.radius * cube.z * cp_t.wdir.z;

    P_noise_space.x = cp_t.noise_space.x;
    P_noise_space.y = cp_t.noise_space.y;
    P_noise_space.z = cp_t.noise_space.z + cube.z;
    noise = turbulence->Evaluate3d(P_noise_space);

    noise.x *= cp_t.noise_amplitude;
    noise.y *= cp_t.noise_amplitude;
    noise.z *= cp_t.radius * cp_t.noise_amplitude;

    P_speck.x += noise.x * cp_t.udir.x + noise.y * cp_t.vdir.x + noise.z * cp_t.wdir.x;
    P_speck.y += noise.x * cp_t.udir.y + noise.y * cp_t.vdir.y + noise.z * cp_t.wdir.y;
    P_speck.z += noise.x * cp_t.udir.z + noise.y * cp_t.vdir.z + noise.z * cp_t.wdir.z;

    FillWithSphere(volume, &P_speck, cp_t.speck_radius, cp_t.density);
    progress.Increment();
  }
  progress.Done();

  return 0;
}
