#include "Procedure.h"
#include "Vector.h"
#include "Numeric.h"
#include "Noise.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

struct ReadPlyProcedure {
	float diffuse[3];
};

static void *MyNew(void);
static void MyFree(void *self);
static const struct Property *MyPropertyList(void);
static int MyProcess(void *self);

static const char MyPluginName[] = "ReadPlyProcedure";
static const struct ProcedureFunctionTable MyFunctionTable = {
	MyPropertyList,
	MyProcess
};

static int set_diffuse(void *self, const struct PropertyValue *value);

static const struct Property MyProperties[] = {
	{"diffuse",   set_diffuse},
	{NULL,        NULL}
};

static const struct MetaInfo MyMetainfo[] = {
	{"help", "A readply shader."},
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
	struct ReadPlyProcedure *readply;

	readply = (struct ReadPlyProcedure *) malloc(sizeof(struct ReadPlyProcedure));
	if (readply == NULL)
		return NULL;

	VEC3_SET(readply->diffuse, 1, 1, 1);

	return readply;
}

static void MyFree(void *self)
{
	struct ReadPlyProcedure *readply = (struct ReadPlyProcedure *) self;
	if (readply == NULL)
		return;
	free(readply);
}

static int MyProcess(void *self)
{
	struct ReadPlyProcedure *readply;

	readply = (struct ReadPlyProcedure *) self;

	return 0;
}

static int set_diffuse(void *self, const struct PropertyValue *value)
{
	struct ReadPlyProcedure *readply = (struct ReadPlyProcedure *) self;
#if 0
	float diffuse[3];

	diffuse[0] = MAX(0, value->vector[0]);
	diffuse[1] = MAX(0, value->vector[1]);
	diffuse[2] = MAX(0, value->vector[2]);
	VEC3_COPY(readply->diffuse, diffuse);
#endif
	readply = NULL;

	return 0;
}

