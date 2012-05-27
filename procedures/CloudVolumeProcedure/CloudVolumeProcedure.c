/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Procedure.h"
#include "Progress.h"
#include "Numeric.h"
#include "Vector.h"
#include "Volume.h"
#include "Noise.h"
#include "Box.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

struct CloudVolumeProcedure {
	int resolution[3];
	double bounds[6];

	double radius;
	double amplitude;
	double frequency[3];
	double offset[3];

	struct Volume *volume;
};

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static int MyRun(void *self);

static const char MyPluginName[] = "CloudVolumeProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyPropertyList,
	MyRun
};

static int set_resolution(void *self, const struct PropertyValue *value);
static int set_bounds_min(void *self, const struct PropertyValue *value);
static int set_bounds_max(void *self, const struct PropertyValue *value);
static int set_radius(void *self, const struct PropertyValue *value);
static int set_amplitude(void *self, const struct PropertyValue *value);
static int set_frequency(void *self, const struct PropertyValue *value);
static int set_offset(void *self, const struct PropertyValue *value);
static int set_volume(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VECTOR3, "resolution", set_resolution},
	{PROP_VECTOR3, "bounds_min", set_bounds_min},
	{PROP_VECTOR3, "bounds_max", set_bounds_max},
	{PROP_SCALAR,  "radius",     set_radius},
	{PROP_SCALAR,  "amplitude",  set_amplitude},
	{PROP_VECTOR3, "frequency",  set_frequency},
	{PROP_VECTOR3, "offset",     set_offset},
	{PROP_VOLUME,  "volume",     set_volume},
	{PROP_NONE,    NULL,         NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A cloud volume procedure."},
	{"plugin_type", "Procedure"},
	{NULL, NULL}
};

static const struct Property *MyPropertyList(void)
{
	return MyProperties;
}

int Initialize(struct PluginInfo *info)
{
	info->api_version = PLUGIN_API_VERSION;
	info->name = MyPluginName;
	info->create_instance = MyNew;
	info->delete_instance = MyFree;
	info->vtbl = &MyFunctionTable;
	info->meta = MyMetainfo;

	return 0;
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
	cloud->amplitude = 1;
	VEC3_SET(cloud->frequency, 1, 1, 1);
	VEC3_SET(cloud->offset, 0, 0, 0);

	cloud->volume = NULL;

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

	if (cloud->volume == NULL) {
		return -1;
	}

	VolSetBounds(cloud->volume, cloud->bounds);
	VolResize(cloud->volume,
			cloud->resolution[0], cloud->resolution[1], cloud->resolution[2]);

	{
		/* based on Production Volume Rendering (SIGGRAPH 2011) Course notes */
		int i, j, k;
		int res[3] = {1, 1, 1};
		double bbox[6] = {0};

		double voxelsize[3] = {0};
		double filtersize = 0;
		double thresholdwidth = 0;

		/* TODO come up with the best place to put progress */
		struct Progress *progress = PrgNew();
		if (progress == NULL)
			return -1;

		VolGetResolution(cloud->volume, res);
		VolGetBounds(cloud->volume, bbox);

		voxelsize[0] = BOX3_XSIZE(bbox) / res[0];
		voxelsize[1] = BOX3_YSIZE(bbox) / res[1];
		voxelsize[2] = BOX3_ZSIZE(bbox) / res[2];
		filtersize = VEC3_LEN(voxelsize);
		thresholdwidth = filtersize * .5 / .5;

		PrgStart(progress, res[0] * res[1] * res[2]);

		for (k = 0; k < res[2]; k++) {
			for (j = 0; j < res[1]; j++) {
				for (i = 0; i < res[0]; i++) {
					const double x = (i + .5) / res[0] * BOX3_XSIZE(bbox) + bbox[0];
					const double y = (j + .5) / res[1] * BOX3_YSIZE(bbox) + bbox[1];
					const double z = (k + .5) / res[2] * BOX3_ZSIZE(bbox) + bbox[2];
					const double dist = sqrt(x*x + y*y + z*z);

					const double sphere_func = dist - cloud->radius;

					double noise[3] = {0};
					double amp[3] = {1, 1, 1};
					double freq[3] = {1, 1, 1};
					double offset[3] = {0, 0, 0};
					double P[3];
					float value = 0;

					VEC3_COPY(freq, cloud->frequency);
					VEC3_COPY(offset, cloud->offset);

					VEC3_SET(P, x, y, z);
					VEC3_NORMALIZE(P);

					PerlinNoise(P, amp, freq, offset, 2, .5, 8, noise);

					noise[0] = ABS(noise[0]);
					noise[0] = Gamma(noise[0], .5);
					noise[0] *= cloud->amplitude;

					value = Fit(sphere_func - noise[0],
							-thresholdwidth, thresholdwidth, 1, 0);

					VolSetValue(cloud->volume, i, j, k, 5*value);

					PrgIncrement(progress);
				}
			}
		}
		PrgDone(progress);
		PrgFree(progress);
	}

	return 0;
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

static int set_amplitude(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float amplitude = 0;

	amplitude = value->vector[0];
	cloud->amplitude = amplitude;

	return 0;
}

static int set_frequency(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float frequency[3] = {0};

	frequency[0] = value->vector[0];
	frequency[1] = value->vector[1];
	frequency[2] = value->vector[2];
	VEC3_COPY(cloud->frequency, frequency);

	return 0;
}

static int set_offset(void *self, const struct PropertyValue *value)
{
	struct CloudVolumeProcedure *cloud = (struct CloudVolumeProcedure *) self;
	float offset[3] = {0};

	offset[0] = value->vector[0];
	offset[1] = value->vector[1];
	offset[2] = value->vector[2];
	VEC3_COPY(cloud->offset, offset);

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

