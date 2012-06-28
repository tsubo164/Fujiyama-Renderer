/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "Turbulence.h"
#include "Progress.h"
#include "Numeric.h"
#include "Random.h"
#include "Vector.h"
#include "Volume.h"
#include "Box.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>

/* TODO test */
struct Point {
	double orig[3];
	double udir[3];
	double vdir[3];
	double wdir[3];
	double radius;
	double speck_count;
	double speck_radius;
};

struct CloudVolumeProcedure {
	int resolution[3];
	double bounds[6];

	double radius;

	struct Volume *volume;
	struct Turbulence *turbulence;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "CloudVolumeProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyRun
};

static int set_resolution(void *self, const struct PropertyValue *value);
static int set_bounds_min(void *self, const struct PropertyValue *value);
static int set_bounds_max(void *self, const struct PropertyValue *value);
static int set_radius(void *self, const struct PropertyValue *value);
static int set_volume(void *self, const struct PropertyValue *value);
static int set_turbulence(void *self, const struct PropertyValue *value);

static int FillWithPointClouds(struct CloudVolumeProcedure *cloud);
static int FillWithSpecks(struct CloudVolumeProcedure *cloud);
static int FillWithSplineSpecks(struct CloudVolumeProcedure *cloud);
static void fill_with_sphere(struct Volume *volume, const double *center, double radius);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3,    "resolution", set_resolution},
	{PROP_VECTOR3,    "bounds_min", set_bounds_min},
	{PROP_VECTOR3,    "bounds_max", set_bounds_max},
	{PROP_SCALAR,     "radius",     set_radius},
	{PROP_VOLUME,     "volume",     set_volume},
	{PROP_TURBULENCE, "turbulence", set_turbulence},
	{PROP_NONE,       NULL,         NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A cloud volume procedure."},
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
	struct CloudVolumeProcedure *cloud;

	cloud = (struct CloudVolumeProcedure *) malloc(sizeof(struct CloudVolumeProcedure));
	if (cloud == NULL)
		return NULL;

	VEC3_SET(cloud->resolution, 10, 10, 10);
	BOX3_SET(cloud->bounds, -1, -1, -1, 1, 1, 1);

	cloud->radius = 1;

	cloud->volume = NULL;
	cloud->turbulence = NULL;

	return cloud;
}

static void MyFree(void *self)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	if (cloud == NULL)
		return;
	free(cloud);
}

static int MyRun(void *self)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	int err = 0;

	if (cloud->volume == NULL) {
		return -1;
	}

	VolSetBounds(cloud->volume, cloud->bounds);
	VolResize(cloud->volume,
			cloud->resolution[0], cloud->resolution[1], cloud->resolution[2]);

	if (0) {
		err = FillWithPointClouds(cloud);
	} else if (0) {
		err = FillWithSpecks(cloud);
	} else  {
		err = FillWithSplineSpecks(cloud);
	}

	return err;
}

static int set_resolution(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float resolution[3] = {0};

	resolution[0] = MAX(1, value->vector[0]);
	resolution[1] = MAX(1, value->vector[1]);
	resolution[2] = MAX(1, value->vector[2]);
	VEC3_COPY(cloud->resolution, resolution);

	return 0;
}

static int set_bounds_min(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float bounds_min[3] = {0};

	bounds_min[0] = value->vector[0];
	bounds_min[1] = value->vector[1];
	bounds_min[2] = value->vector[2];
	VEC3_COPY(cloud->bounds, bounds_min);

	return 0;
}

static int set_bounds_max(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float bounds_max[3] = {0};

	bounds_max[0] = value->vector[0];
	bounds_max[1] = value->vector[1];
	bounds_max[2] = value->vector[2];
	VEC3_COPY(cloud->bounds+3, bounds_max);

	return 0;
}

static int set_radius(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float radius = 0;

	radius = MAX(.001, value->vector[0]);
	cloud->radius = radius;

	return 0;
}

static int set_volume(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;

	if (value->volume == NULL)
		return -1;

	cloud->volume = value->volume;

	return 0;
}

static int set_turbulence(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;

	if (value->turbulence == NULL)
		return -1;

	cloud->turbulence = value->turbulence;

	return 0;
}

static int FillWithPointClouds(struct CloudVolumeProcedure *cloud)
{
	/* based on Production Volume Rendering (SIGGRAPH 2011) Course notes */
	int i, j, k;
	int xres, yres, zres;
	double thresholdwidth = 0;

	/* TODO come up with the best place to put progress */
	struct Progress *progress = PrgNew();
	if (progress == NULL)
		return -1;

	VolGetResolution(cloud->volume, &xres, &yres, &zres);
	thresholdwidth = VolGetFilterSize(cloud->volume) / cloud->radius;

	PrgStart(progress, xres * yres * zres);

	for (k = 0; k < zres; k++) {
		for (j = 0; j < yres; j++) {
			for (i = 0; i < xres; i++) {
				double sphere_func = 0;
				double noise_func = 0;
				double dist = 0;

				double amp = 1;
				double P[3] = {0};
				float value = 0;

				VolIndexToPoint(cloud->volume, i, j, k, P);

				dist = VEC3_LEN(P);
				sphere_func = dist - cloud->radius;

				VEC3_NORMALIZE(P);
				noise_func = amp * TrbEvaluate(cloud->turbulence, P);

				noise_func = ABS(noise_func);
				noise_func = Gamma(noise_func, .5);

				value = Fit(sphere_func - noise_func,
						-thresholdwidth, thresholdwidth, 1, 0);

				VolSetValue(cloud->volume, i, j, k, 5*value);

				PrgIncrement(progress);
			}
		}
	}
	PrgDone(progress);
	PrgFree(progress);

	return 0;
}

static int FillWithSpecks(struct CloudVolumeProcedure *cloud)
{
	const int NSPECKS = 100000 * 10 / 5;
	double uvec[3] = {1, 0, 0};
	double vvec[3] = {0, 1, 0};
	double wvec[3] = {0, 0, 1};
	struct XorShift xr;
	int i;

	XorInit(&xr);

	for (i = 0; i < NSPECKS; i++) {
		double uvw[3] = {0};
		double P_speck[3] = {0};
		double disp_vec[3] = {0};

		const double amp = 1 * .5;
		const double radius = .5;

		XorSolidSphereRand(&xr, uvw);

		P_speck[0] = uvw[0] * uvec[0] + uvw[1] * vvec[0] + uvw[2] * wvec[0];
		P_speck[1] = uvw[0] * uvec[1] + uvw[1] * vvec[1] + uvw[2] * wvec[1];
		P_speck[2] = uvw[0] * uvec[2] + uvw[1] * vvec[2] + uvw[2] * wvec[2];

		TrbEvaluate3d(cloud->turbulence, uvw, disp_vec);

		P_speck[0] = radius * P_speck[0] + amp * disp_vec[0];
		P_speck[1] = radius * P_speck[1] + amp * disp_vec[1];
		P_speck[2] = radius * P_speck[2] + amp * disp_vec[2];

		fill_with_sphere(cloud->volume, P_speck, .01);
	}

	return 0;
}

static int FillWithSplineSpecks(struct CloudVolumeProcedure *cloud)
{
	const int NSPECKS = 100000;

	double vert0[3] = {-.75, -.5, .75};
	double vert1[3] = {.75, .5, -.75};

	double uvec[3] = {1, 0, 0};
	double vvec[3] = {0, 1, 0};
	double wvec[3] = {0, 0, 1};
	struct XorShift xr;
	int i;

	XorInit(&xr);

	VEC3_SUB(wvec, vert1, vert0);
	VEC3_CROSS(uvec, wvec, vvec);
	VEC3_CROSS(vvec, uvec, wvec);
	VEC3_NORMALIZE(uvec);
	VEC3_NORMALIZE(vvec);

	for (i = 0; i < NSPECKS; i++) {
		double uvw[3] = {0};
		double P_speck[3] = {0};
		double disp_vec[3] = {0};

		const double amp = 1 * .25;
		const double radius = .1+.3;

		/*
		XorSolidDiskRand(&xr, uvw);
		XorGaussianDiskRand(&xr, uvw);
		*/
		XorHollowDiskRand(&xr, uvw);

		uvw[0] *= radius;
		uvw[1] *= radius;
		uvw[2] = XorNextFloat01(&xr);

		P_speck[0] = vert0[0] + uvw[0] * uvec[0] + uvw[1] * vvec[0] + uvw[2] * wvec[0];
		P_speck[1] = vert0[1] + uvw[0] * uvec[1] + uvw[1] * vvec[1] + uvw[2] * wvec[1];
		P_speck[2] = vert0[2] + uvw[0] * uvec[2] + uvw[1] * vvec[2] + uvw[2] * wvec[2];

		TrbEvaluate3d(cloud->turbulence, uvw, disp_vec);

		disp_vec[0] *= amp;
		disp_vec[1] *= amp;
		disp_vec[2] *= 0;

		P_speck[0] += disp_vec[0] * uvec[0] + disp_vec[1] * vvec[0] + disp_vec[2] * wvec[0];
		P_speck[1] += disp_vec[0] * uvec[1] + disp_vec[1] * vvec[1] + disp_vec[2] * wvec[1];
		P_speck[2] += disp_vec[0] * uvec[2] + disp_vec[1] * vvec[2] + disp_vec[2] * wvec[2];

		fill_with_sphere(cloud->volume, P_speck, .01);
	}

	return 0;
}

static void fill_with_sphere(struct Volume *volume, const double *center, double radius)
{
	int i, j, k;
	int xmin, ymin, zmin;
	int xmax, ymax, zmax;
	double P_min[3] = {0};
	double P_max[3] = {0};
	double thresholdwidth = 0;

	P_min[0] = center[0] - radius;
	P_min[1] = center[1] - radius;
	P_min[2] = center[2] - radius;
	P_max[0] = center[0] + radius;
	P_max[1] = center[1] + radius;
	P_max[2] = center[2] + radius;

	VolPointToIndex(volume, P_min, &xmin, &ymin, &zmin);
	VolPointToIndex(volume, P_max, &xmax, &ymax, &zmax);

	zmax += 1;
	ymax += 1;
	zmax += 1;

	thresholdwidth = VolGetFilterSize(volume) / radius;

	for (k = zmin; k < zmax; k++) {
		for (j = ymin; j < ymax; j++) {
			for (i = xmin; i < xmax; i++) {
				double P[3] = {0};
				float value = 0;

				VolIndexToPoint(volume, i, j, k, P);

				P[0] -= center[0];
				P[1] -= center[1];
				P[2] -= center[2];

				value = VolGetValue(volume, i, j, k);
				value += Fit(VEC3_LEN(P) - radius,
						-thresholdwidth, thresholdwidth, 1, 0);
				VolSetValue(volume, i, j, k, value);
			}
		}
	}
}

