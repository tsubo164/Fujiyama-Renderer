/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Property.h"
#include "Numeric.h"
#include "Vector.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int prop_is_valid(const struct Property *prop);
static int compare_property_sample(const void *ptr0, const void *ptr1);
static void push_sample(struct PropertySampleList *list, const struct PropertySample *sample);
static void sort_by_sample_time(struct PropertySampleList *list);

struct PropertyValue PropScalar(double v0)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_SCALAR;
	value.vector[0] = v0;

	return value;
}

struct PropertyValue PropVector2(double v0, double v1)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_VECTOR2;
	value.vector[0] = v0;
	value.vector[1] = v1;

	return value;
}

struct PropertyValue PropVector3(double v0, double v1, double v2)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_VECTOR3;
	value.vector[0] = v0;
	value.vector[1] = v1;
	value.vector[2] = v2;

	return value;
}

struct PropertyValue PropVector4(double v0, double v1, double v2, double v3)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_VECTOR4;
	value.vector[0] = v0;
	value.vector[1] = v1;
	value.vector[2] = v2;
	value.vector[3] = v3;

	return value;
}

struct PropertyValue PropTurbulence(struct Turbulence *turbulence)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_TURBULENCE;
	value.turbulence = turbulence;

	return value;
}

struct PropertyValue PropTexture(struct Texture *texture)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_TEXTURE;
	value.texture = texture;

	return value;
}

struct PropertyValue PropVolume(struct Volume *volume)
{
	struct PropertyValue value = INIT_PROPERTYVALUE;

	value.type = PROP_VOLUME;
	value.volume = volume;

	return value;
}

struct PropertyValue InitPropValue(void)
{
	const struct PropertyValue value = INIT_PROPERTYVALUE;
	return value;
}

const struct Property *PropFind(const struct Property *list, int type, const char *name)
{
	const struct Property *prop = list;
	const struct Property *found = NULL;

	while (prop_is_valid(prop)) {
		if (prop->type == type && strcmp(prop->name, name) == 0) {
			found = prop;
			break;
		}
		prop++;
	}
	return found;
}

/* for time variable properties */
void PropInitSampleList(struct PropertySampleList *list)
{
	const struct PropertySample initial_value = INIT_PROPERTYSAMPLE;
	int i;

	for (i = 0; i < MAX_PROPERTY_SAMPLES; i++) {
		list->samples[i] = initial_value;
	}

	list->sample_count = 1;
}

int PropPushSample(struct PropertySampleList *list, const struct PropertySample *sample)
{
	int i;

	if (list->sample_count >= MAX_PROPERTY_SAMPLES) {
		return -1;
	}

	for (i = 0; i < list->sample_count; i++) {
		if (list->samples[i].time == sample->time) {
			list->samples[i] = *sample;
			return 0;
		}
	}

	push_sample(list, sample);
	sort_by_sample_time(list);

	return 0;
}

void PropLerpSamples(const struct PropertySampleList *list, double time,
		struct PropertySample *dst)
{
	int i;

	if (list->samples[0].time >= time || list->sample_count == 1) {
		VEC4_COPY(dst->vector, list->samples[0].vector);
		return;
	}

	if (list->samples[list->sample_count-1].time <= time) {
		VEC4_COPY(dst->vector, list->samples[list->sample_count-1].vector);
		return;
	}

	for (i = 0; i < list->sample_count; i++) {
		if (list->samples[i].time == time) {
			VEC4_COPY(dst->vector, list->samples[i].vector);
			return;
		}
		if (list->samples[i].time > time) {
			const struct PropertySample *sample0 = &list->samples[i-1];
			const struct PropertySample *sample1 = &list->samples[i];
			const double t = Fit(time, sample0->time, sample1->time, 0, 1);
			VEC4_LERP(dst->vector, sample0->vector, sample1->vector, t);
			return;
		}
	}
}

static int prop_is_valid(const struct Property *prop)
{
	if (prop->type == PROP_NONE)
		return 0;

	if (prop->name == NULL)
		return 0;

	return 1;
}

static int compare_property_sample(const void *ptr0, const void *ptr1)
{
	const struct PropertySample *sample0 = (const struct PropertySample *) ptr0;
	const struct PropertySample *sample1 = (const struct PropertySample *) ptr1;
	const double x = sample0->time;
	const double y = sample1->time;

	if (x > y)
		return 1;
	else if (x < y)
		return -1;
	else
		return 0;
}

static void push_sample(struct PropertySampleList *list, const struct PropertySample *sample)
{
	const int next_index = list->sample_count;
	assert(list->sample_count < MAX_PROPERTY_SAMPLES);

	list->samples[next_index] = *sample;
	list->sample_count++;
}

static void sort_by_sample_time(struct PropertySampleList *list)
{
	qsort(list->samples, list->sample_count,
			sizeof(struct PropertySample),
			compare_property_sample);
}

