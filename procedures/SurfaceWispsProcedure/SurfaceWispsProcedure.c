/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "VolumeFilling.h"
#include "Turbulence.h"
#include "Progress.h"
#include "Numeric.h"
#include "Random.h"
#include "Vector.h"
#include "Volume.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>

struct SurfaceWispsProcedure {
	struct Volume *volume;
	const struct Turbulence *turbulence;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "SurfaceWispsProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyRun
};

static int set_volume(void *self, const struct PropertyValue *value);
static int set_turbulence(void *self, const struct PropertyValue *value);

static int FillWithSpecksOnSurface(struct Volume *volume,
		const struct WispsControlPoint *cp00, const struct WispsControlPoint *cp10,
		const struct WispsControlPoint *cp01, const struct WispsControlPoint *cp11,
		const struct Turbulence *turbulence);

static const struct Property MyProperties[] = {
	{PROP_VOLUME,     "volume",     set_volume},
	{PROP_TURBULENCE, "turbulence", set_turbulence},
	{PROP_NONE,       NULL,         NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A surface volume procedure."},
	{"plugin_type", "Procedure"},
	{NULL, NULL}
};

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

static void *MyNew(void)
{
	struct SurfaceWispsProcedure *surface;

	surface = (struct SurfaceWispsProcedure *) malloc(sizeof(struct SurfaceWispsProcedure));
	if (surface == NULL)
		return NULL;

	surface->volume = NULL;
	surface->turbulence = NULL;

	return surface;
}

static void MyFree(void *self)
{
	struct SurfaceWispsProcedure *surface = (struct SurfaceWispsProcedure *) self;
	if (surface == NULL)
		return;
	free(surface);
}

static int MyRun(void *self)
{
	struct SurfaceWispsProcedure *surface = (struct SurfaceWispsProcedure *) self;
	struct WispsControlPoint cp00, cp10, cp01, cp11;
	int err = 0;

	if (surface->volume == NULL) {
		return -1;
	}

	VEC3_SET(cp00.orig, 0, 0, 0);
	VEC3_SET(cp00.udir, 1, 0, 0);
	VEC3_SET(cp00.vdir, 0, 1, 0);
	VEC3_SET(cp00.wdir, 0, 0, 1);
	VEC3_SET(cp00.noise_space, 0, 0, 0);
	cp00.density = 1;
	cp00.radius = .5;
	cp00.noise_amplitude = 1;
	cp00.speck_count = 100000 * 100;
	cp00.speck_radius = .01;

	cp10 = cp00;
	cp01 = cp00;
	cp11 = cp00;

	VEC3_SET(cp00.orig, -.75, -.75, 0);
	VEC3_SET(cp10.orig,  .75, -.75, 0);
	VEC3_SET(cp01.orig, -.75,  .75, 0);
	VEC3_SET(cp11.orig,  .75,  .75, 0);

	VEC3_SET(cp00.noise_space, 0, 0, 0);
	VEC3_SET(cp10.noise_space, 1, 0, 0);
	VEC3_SET(cp01.noise_space, 0, 1, 0);
	VEC3_SET(cp11.noise_space, 1, 1, 0);

	cp11.radius = 1;

	err = FillWithSpecksOnSurface(surface->volume,
			&cp00, &cp10,
			&cp01, &cp11,
			surface->turbulence);

	return err;
}

static int set_volume(void *self, const struct PropertyValue *value)
{
	struct SurfaceWispsProcedure *surface = (struct SurfaceWispsProcedure *) self;

	if (value->volume == NULL)
		return -1;

	surface->volume = value->volume;

	return 0;
}

static int set_turbulence(void *self, const struct PropertyValue *value)
{
	struct SurfaceWispsProcedure *surface = (struct SurfaceWispsProcedure *) self;

	if (value->turbulence == NULL)
		return -1;

	surface->turbulence = value->turbulence;

	return 0;
}

static int FillWithSpecksOnSurface(struct Volume *volume,
		const struct WispsControlPoint *cp00, const struct WispsControlPoint *cp10,
		const struct WispsControlPoint *cp01, const struct WispsControlPoint *cp11,
		const struct Turbulence *turbulence)

{
	struct XorShift xr;
	int NSPECKS = 1000;
	int i = 0;

	/* TODO come up with the best place to put progress */
	struct Progress *progress = PrgNew();
	if (progress == NULL)
		return -1;

	XorInit(&xr);
	/* TODO should not be a point attribute? */
	NSPECKS = cp00->speck_count;

	PrgStart(progress, NSPECKS);

	for (i = 0; i < NSPECKS; i++) {
		struct WispsControlPoint cp_t;
		double cube[3] = {0};
		double P_speck[3] = {0};
		double P_noise_space[3] = {0};
		double noise[3] = {0};
		double s = 0;
		double t = 0;

		XorSolidCubeRand(&xr, cube);

		s = cube[0];
		t = cube[1];

		BilerpWispConstrolPoint(&cp_t, cp00, cp10, cp01, cp11, s, t);

		VEC3_COPY(P_speck, cp_t.orig);
		P_speck[0] += cp_t.radius * cube[2] * cp_t.wdir[0];
		P_speck[1] += cp_t.radius * cube[2] * cp_t.wdir[1];
		P_speck[2] += cp_t.radius * cube[2] * cp_t.wdir[2];

		P_noise_space[0] = cp_t.noise_space[0];
		P_noise_space[1] = cp_t.noise_space[1];
		P_noise_space[2] = cp_t.noise_space[2] + cube[2];
		TrbEvaluate3d(turbulence, P_noise_space, noise);

		noise[0] *= cp_t.noise_amplitude;
		noise[1] *= cp_t.noise_amplitude;
		noise[2] *= cp_t.radius * cp_t.noise_amplitude;

		P_speck[0] += noise[0] * cp_t.udir[0] + noise[1] * cp_t.vdir[0] + noise[2] * cp_t.wdir[0];
		P_speck[1] += noise[0] * cp_t.udir[1] + noise[1] * cp_t.vdir[1] + noise[2] * cp_t.wdir[1];
		P_speck[2] += noise[0] * cp_t.udir[2] + noise[1] * cp_t.vdir[2] + noise[2] * cp_t.wdir[2];

		FillWithSphere(volume, P_speck, cp_t.speck_radius, cp_t.density);
		PrgIncrement(progress);
	}
	PrgDone(progress);
	PrgFree(progress);

	return 0;
}

