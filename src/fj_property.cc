/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_property.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define VEC4_COPY(dst,a) do { \
  (dst)[0] = (a)[0]; \
  (dst)[1] = (a)[1]; \
  (dst)[2] = (a)[2]; \
  (dst)[3] = (a)[3]; \
  } while(0)

#define VEC4_LERP(dst,a,b,t) do { \
  (dst)[0] = (1-(t)) * (a)[0] + (t) * (b)[0]; \
  (dst)[1] = (1-(t)) * (a)[1] + (t) * (b)[1]; \
  (dst)[2] = (1-(t)) * (a)[2] + (t) * (b)[2]; \
  (dst)[3] = (1-(t)) * (a)[3] + (t) * (b)[3]; \
  } while(0)

namespace fj {

static int compare_property_sample(const void *ptr0, const void *ptr1);
static void push_sample(struct PropertySampleList *list, const struct PropertySample *sample);
static void sort_by_sample_time(struct PropertySampleList *list);

struct PropertyValue PropScalar(double v0)
{
  struct PropertyValue value;

  value.type = PROP_SCALAR;
  value.vector[0] = v0;

  return value;
}

struct PropertyValue PropVector2(double v0, double v1)
{
  struct PropertyValue value;

  value.type = PROP_VECTOR2;
  value.vector[0] = v0;
  value.vector[1] = v1;

  return value;
}

struct PropertyValue PropVector3(double v0, double v1, double v2)
{
  struct PropertyValue value;

  value.type = PROP_VECTOR3;
  value.vector[0] = v0;
  value.vector[1] = v1;
  value.vector[2] = v2;

  return value;
}

struct PropertyValue PropVector4(double v0, double v1, double v2, double v3)
{
  struct PropertyValue value;

  value.type = PROP_VECTOR4;
  value.vector[0] = v0;
  value.vector[1] = v1;
  value.vector[2] = v2;
  value.vector[3] = v3;

  return value;
}

struct PropertyValue PropString(const char *string)
{
  struct PropertyValue value;

  value.type = PROP_STRING;
  value.string = string;

  return value;
}

struct PropertyValue PropObjectGroup(struct ObjectGroup *group)
{
  struct PropertyValue value;

  value.type = PROP_OBJECTGROUP;
  value.object_group = group;

  return value;
}

struct PropertyValue PropTurbulence(struct Turbulence *turbulence)
{
  struct PropertyValue value;

  value.type = PROP_TURBULENCE;
  value.turbulence = turbulence;

  return value;
}

struct PropertyValue PropTexture(struct Texture *texture)
{
  struct PropertyValue value;

  value.type = PROP_TEXTURE;
  value.texture = texture;

  return value;
}

struct PropertyValue PropVolume(struct Volume *volume)
{
  struct PropertyValue value;

  value.type = PROP_VOLUME;
  value.volume = volume;

  return value;
}

struct PropertyValue PropMesh(struct Mesh *mesh)
{
  struct PropertyValue value;

  value.type = PROP_MESH;
  value.mesh = mesh;

  return value;
}

int PropIsValid(const struct Property *prop)
{
  if (PropType(prop) == PROP_NONE)
    return 0;

  if (PropName(prop) == NULL)
    return 0;

  return 1;
}

const char *PropName(const struct Property *prop)
{
  return prop->name;
}

const int PropType(const struct Property *prop)
{
  return prop->type;
}

const double PropDefaultValue(const struct Property *prop, int index)
{
  if (index < 0 || index > 3)
    return 0;

  return prop->default_value[index];
}

const char *PropTypeString(const struct Property *prop)
{
  switch (PropType(prop)) {
  case PROP_SCALAR:      return "Scalar";
  case PROP_VECTOR2:     return "Vector2";
  case PROP_VECTOR3:     return "Vector3";
  case PROP_VECTOR4:     return "Vector4";
  case PROP_STRING:      return "String";
  case PROP_OBJECTGROUP: return "ObjectGroup";
  case PROP_TURBULENCE:  return "Turbulence";
  case PROP_TEXTURE:     return "Texture";
  case PROP_SHADER:      return "Shader";
  case PROP_VOLUME:      return "Volume";
  case PROP_MESH:        return "Mesh";
  default:               return NULL;
  }
}

const struct Property *PropFind(const struct Property *list, int type, const char *name)
{
  const struct Property *prop = list;
  const struct Property *found = NULL;

  while (PropIsValid(prop)) {
    if (PropType(prop) == type && strcmp(PropName(prop), name) == 0) {
      found = prop;
      break;
    }
    prop++;
  }
  return found;
}

int PropSetAllDefaultValues(void *self, const struct Property *list)
{
  const struct Property *prop = NULL;
  int err_count = 0;

  for (prop = list; PropIsValid(prop); prop++) {
    struct PropertyValue value;
    int err = 0;

    switch (PropType(prop)) {
      case PROP_SCALAR:
        value.vector[0] = PropDefaultValue(prop, 0);
        break;
      case PROP_VECTOR2:
        value.vector[0] = PropDefaultValue(prop, 0);
        value.vector[1] = PropDefaultValue(prop, 1);
        break;
      case PROP_VECTOR3:
        value.vector[0] = PropDefaultValue(prop, 0);
        value.vector[1] = PropDefaultValue(prop, 1);
        value.vector[2] = PropDefaultValue(prop, 2);
        break;
      case PROP_VECTOR4:
        value.vector[0] = PropDefaultValue(prop, 0);
        value.vector[1] = PropDefaultValue(prop, 1);
        value.vector[2] = PropDefaultValue(prop, 2);
        value.vector[3] = PropDefaultValue(prop, 3);
        break;
      default:
        break;
    }
    err = prop->SetProperty(self, &value);
    if (err) {
      err_count++;
    }
  }

  return err_count;
}

/* for time variable properties */
void PropInitSampleList(struct PropertySampleList *list)
{
  const struct PropertySample initial_value;
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

} // namespace xxx
