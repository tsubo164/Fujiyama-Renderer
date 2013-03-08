/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "VolumeFilling.h"
#include "Turbulence.h"
#include "Progress.h"
#include "Numeric.h"
#include "Vector.h"
#include "Volume.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>

struct CloudVolumeProcedure {
	struct Volume *volume;
	struct Turbulence *turbulence;
};

static void *MyNew(void);
static void MyFree(void *self);
static int MyRun(void *self);

static const char MyPluginName[] = "PointCloudsProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyRun
};

static int set_volume(void *self, const struct PropertyValue *value);
static int set_turbulence(void *self, const struct PropertyValue *value);

static int FillWithPointClouds(struct Volume *volume,
		const struct CloudControlPoint *cp, const struct Turbulence *turbulence);

static const struct Property MyProperties[] = {
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
	struct CloudControlPoint cp;
	int err = 0;

	if (cloud->volume == NULL) {
		return -1;
	}

	VEC3_SET(&cp.orig, 0, 0, 0);
	VEC3_SET(&cp.udir, 1, 0, 0);
	VEC3_SET(&cp.vdir, 0, 1, 0);
	VEC3_SET(&cp.wdir, 0, 0, 1);
	VEC3_SET(&cp.noise_space, 0, 0, 0);
	cp.density = 5;
	cp.radius = .75;
	cp.noise_amplitude = 1;

	err = FillWithPointClouds(cloud->volume, &cp, cloud->turbulence);

	return err;
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

static int FillWithPointClouds(struct Volume *volume,
		const struct CloudControlPoint *cp, const struct Turbulence *turbulence)

{
	/* based on Production Volume Rendering (SIGGRAPH 2011) Course notes */
	int i, j, k;
	int xres, yres, zres;
	int xmin, ymin, zmin;
	int xmax, ymax, zmax;

	double thresholdwidth = 0;

	/* TODO come up with the best place to put progress */
	struct Progress *progress = PrgNew();
	if (progress == NULL)
		return -1;

	VolGetResolution(volume, &xres, &yres, &zres);
	thresholdwidth = .5 * VolGetFilterSize(volume);

	VolGetIndexRange(volume, &cp->orig, cp->radius * 1.5,
			&xmin, &ymin, &zmin,
			&xmax, &ymax, &zmax);

	PrgStart(progress,
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

				struct Vector cell_center = {0, 0, 0};
				struct Vector P_local_space = {0, 0, 0};
				struct Vector P_noise_space = {0, 0, 0};
				float pyro_value = 0;
				float value = 0;

				VolIndexToPoint(volume, i, j, k, &cell_center);
				P_local_space.x =  cell_center.x - cp->orig.x;
				P_local_space.y =  cell_center.y - cp->orig.y;
				P_local_space.z =  cell_center.z - cp->orig.z;
				distance = VEC3_LEN(&P_local_space);

				if (distance < cp->radius - thresholdwidth) {
					value = VolGetValue(volume, i, j, k);
					VolSetValue(volume, i, j, k, MAX(value, cp->density));
					PrgIncrement(progress);
					continue;
				}

				P_noise_space = P_local_space;
				VEC3_NORMALIZE(&P_noise_space);
				P_noise_space.x += cp->noise_space.x;
				P_noise_space.y += cp->noise_space.y;
				P_noise_space.z += cp->noise_space.z;

				noise_func = TrbEvaluate(turbulence, &P_noise_space);
				noise_func = ABS(noise_func);
				noise_func = Gamma(noise_func, .5);
				noise_func *= cp->noise_amplitude;

				sphere_func = distance - cp->radius;
				pyro_func = sphere_func - noise_func;
				pyro_value = Fit(pyro_func, -thresholdwidth, thresholdwidth, 1, 0);
				pyro_value *= cp->density;

				value = VolGetValue(volume, i, j, k);
				VolSetValue(volume, i, j, k, MAX(value, pyro_value));

				PrgIncrement(progress);
			}
		}
	}
	PrgDone(progress);
	PrgFree(progress);

	return 0;
}

