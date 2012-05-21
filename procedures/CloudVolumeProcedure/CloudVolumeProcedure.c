#include "Procedure.h"
#include "Vector.h"
/* TODO temp Volume.h */
#include "Volume.h"
#include "Numeric.h"
#include "Noise.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

struct CloudVolumeProcedure {
	float diffuse[3];
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

static int set_volume(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{PROP_VOLUME, "volume", set_volume},
	{PROP_NONE,   NULL,     NULL}
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

	VEC3_SET(cloud->diffuse, 1, 1, 1);
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
	struct CloudVolumeProcedure *cloud;
	double bounds[6] = {-.5, -.5, -.5, .5, .5, .5};

	cloud = (struct CloudVolumeProcedure *) self;

	if (cloud->volume == NULL) {
		return -1;
	}

	VolSetBounds(cloud->volume, bounds);
	VolResize(cloud->volume, 10, 10, 10);

	{
		int i, j, k;
		for (k = 0; k < 10; k++) {
			for (j = 0; j < 10; j++) {
				for (i = 0; i < 10; i++) {
					VolSetValue(cloud->volume, i, j, k, 5);
				}
			}
		}
		/*
		*/
	}

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

