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
	{PROP_VOLUME,     "volume",     set_volume},
	{PROP_TURBULENCE, "turbulence", set_turbulence},
	{PROP_NONE,       NULL,         NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A spline volume procedure."},
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
	struct SplineWispsProcedure *spline;

	spline = (struct SplineWispsProcedure *) malloc(sizeof(struct SplineWispsProcedure));
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
	free(spline);
}

static int MyRun(void *self)
{
	struct SplineWispsProcedure *spline = (struct SplineWispsProcedure *) self;
	struct WispsControlPoint cp0, cp1;
	int err = 0;

	if (spline->volume == NULL) {
		return -1;
	}

	VEC3_SET(cp0.orig, -.75, -.5, .75);
	VEC3_SET(cp0.udir, 1, 0, 0);
	VEC3_SET(cp0.vdir, 0, 1, 0);
	VEC3_SET(cp0.wdir, 0, 0, 1);
	VEC3_SET(cp0.noise_space, 0, 0, 0);
	cp0.density = 1;
	cp0.radius = .5;
	cp0.noise_amplitude = 1;
	cp0.speck_count = 100000 * 100;
	cp0.speck_radius = .01 * .5;

	VEC3_SET(cp1.orig, .75, .5, -.75);
	VEC3_SET(cp1.udir, 1, 0, 0);
	VEC3_SET(cp1.vdir, 0, 1, 0);
	VEC3_SET(cp1.wdir, 0, 0, 1);
	VEC3_SET(cp1.noise_space, 0, 0, 1);
	cp1.density = 1;
	cp1.radius = .25;
	cp1.noise_amplitude = 1;
	cp1.speck_count = 100000 * 100;
	cp1.speck_radius = .01 * .5;

	VEC3_SUB(cp0.wdir, cp1.orig, cp0.orig);
	VEC3_CROSS(cp0.udir, cp0.wdir, cp0.vdir);
	VEC3_CROSS(cp0.vdir, cp0.udir, cp0.wdir);
	VEC3_NORMALIZE(cp0.udir);
	VEC3_NORMALIZE(cp0.vdir);
	VEC3_NORMALIZE(cp0.wdir);

	VEC3_COPY(cp1.udir, cp0.udir);
	VEC3_COPY(cp1.vdir, cp0.vdir);
	VEC3_COPY(cp1.wdir, cp0.wdir);

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
	struct Progress *progress = PrgNew();
	if (progress == NULL)
		return -1;

	XorInit(&xr);
	/* TODO should not be a point attribute? */
	NSPECKS = cp0->speck_count;

	PrgStart(progress, NSPECKS);

	for (i = 0; i < NSPECKS; i++) {
		struct WispsControlPoint cp_t;
		double disk[2] = {0};
		double P_speck[3] = {0};
		double P_noise_space[3] = {0};
		double noise[3] = {0};
		double line_t = 0;

		/*
		XorHollowDiskRand(&xr, disk);
		XorGaussianDiskRand(&xr, disk);
		*/
		XorSolidDiskRand(&xr, disk);
		line_t = XorNextFloat01(&xr);

		LerpWispConstrolPoint(&cp_t, cp0, cp1, line_t);

		VEC3_COPY(P_speck, cp_t.orig);
		P_speck[0] += cp_t.radius * disk[0] * cp_t.udir[0] + cp_t.radius * disk[1] * cp_t.vdir[0];
		P_speck[1] += cp_t.radius * disk[0] * cp_t.udir[1] + cp_t.radius * disk[1] * cp_t.vdir[1];
		P_speck[2] += cp_t.radius * disk[0] * cp_t.udir[2] + cp_t.radius * disk[1] * cp_t.vdir[2];

		P_noise_space[0] = cp_t.noise_space[0] + disk[0];
		P_noise_space[1] = cp_t.noise_space[1] + disk[1];
		P_noise_space[2] = cp_t.noise_space[2];
		TrbEvaluate3d(turbulence, P_noise_space, noise);

		noise[0] *= cp_t.radius * cp_t.noise_amplitude;
		noise[1] *= cp_t.radius * cp_t.noise_amplitude;
		noise[2] *= 1;

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

