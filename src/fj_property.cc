// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

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
static void push_sample(PropertySampleList *list, const PropertySample *sample);
static void sort_by_sample_time(PropertySampleList *list);

PropertyValue PropNull()
{
  return PropertyValue();
}

PropertyValue PropScalar(double v0)
{
  PropertyValue value;

  value.type = PROP_SCALAR;
  value.vector[0] = v0;

  return value;
}

PropertyValue PropVector2(double v0, double v1)
{
  PropertyValue value;

  value.type = PROP_VECTOR2;
  value.vector[0] = v0;
  value.vector[1] = v1;

  return value;
}

PropertyValue PropVector3(double v0, double v1, double v2)
{
  PropertyValue value;

  value.type = PROP_VECTOR3;
  value.vector[0] = v0;
  value.vector[1] = v1;
  value.vector[2] = v2;

  return value;
}

PropertyValue PropVector4(double v0, double v1, double v2, double v3)
{
  PropertyValue value;

  value.type = PROP_VECTOR4;
  value.vector[0] = v0;
  value.vector[1] = v1;
  value.vector[2] = v2;
  value.vector[3] = v3;

  return value;
}

PropertyValue PropString(const char *string)
{
  PropertyValue value;

  value.type = PROP_STRING;
  value.string = string;

  return value;
}

PropertyValue PropObjectGroup(ObjectGroup *group)
{
  PropertyValue value;

  value.type = PROP_OBJECTGROUP;
  value.object_group = group;

  return value;
}

PropertyValue PropTurbulence(Turbulence *turbulence)
{
  PropertyValue value;

  value.type = PROP_TURBULENCE;
  value.turbulence = turbulence;

  return value;
}

PropertyValue PropTexture(Texture *texture)
{
  PropertyValue value;

  value.type = PROP_TEXTURE;
  value.texture = texture;

  return value;
}

PropertyValue PropVolume(Volume *volume)
{
  PropertyValue value;

  value.type = PROP_VOLUME;
  value.volume = volume;

  return value;
}

PropertyValue PropMesh(Mesh *mesh)
{
  PropertyValue value;

  value.type = PROP_MESH;
  value.mesh = mesh;

  return value;
}

Property::Property()
  : type_(PROP_NONE), name_(NULL), default_value_(), set_value_fn_(NULL)
{
}

Property::Property(const char *name, const PropertyValue &value, SetValueFn set_value_fn)
{
  type_ = value.type;
  name_ = name;
  default_value_ = value.vector;
  set_value_fn_ = set_value_fn;
}

Property::~Property()
{
}

bool Property::IsValid() const
{
  if (GetType() == PROP_NONE)
    return false;
  if (GetName() == NULL)
    return false;

  return true;
}

int Property::GetType() const
{
  return type_;
}

const char *Property::GetName() const
{
  return name_;
}

const Vector4 &Property::GetDefaultValue() const
{
  return default_value_;
}

const char *Property::GetTypeString() const
{
  switch (GetType()) {
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
  default:               return "(null)";
  }
}

int Property::SetValue(void *self, const PropertyValue &value) const
{
  if (set_value_fn_ == NULL)
    return -1;
  return set_value_fn_(self, value);
}

const Property *PropFind(const Property *list, int type, const char *name)
{
  const Property *prop = list;
  const Property *found = NULL;

  while (prop->IsValid()) {
    if (prop->GetType() == type && strcmp(prop->GetName(), name) == 0) {
      found = prop;
      break;
    }
    prop++;
  }
  return found;
}

int PropSetAllDefaultValues(void *self, const Property *list)
{
  int err_count = 0;

  for (const Property *prop = list; prop->IsValid(); prop++) {
    PropertyValue value;

    switch (prop->GetType()) {
      case PROP_SCALAR:
        value.vector[0] = prop->GetDefaultValue()[0];
        break;
      case PROP_VECTOR2:
        value.vector[0] = prop->GetDefaultValue()[0];
        value.vector[1] = prop->GetDefaultValue()[1];
        break;
      case PROP_VECTOR3:
        value.vector[0] = prop->GetDefaultValue()[0];
        value.vector[1] = prop->GetDefaultValue()[1];
        value.vector[2] = prop->GetDefaultValue()[2];
        break;
      case PROP_VECTOR4:
        value.vector[0] = prop->GetDefaultValue()[0];
        value.vector[1] = prop->GetDefaultValue()[1];
        value.vector[2] = prop->GetDefaultValue()[2];
        value.vector[3] = prop->GetDefaultValue()[3];
        break;
      default:
        break;
    }

    const int err = prop->SetValue(self, value);
    if (err) {
      err_count++;
    }
  }
  return err_count;
}

/* for time variable properties */
void PropInitSampleList(PropertySampleList *list)
{
  const PropertySample initial_value;
  int i;

  for (i = 0; i < MAX_PROPERTY_SAMPLES; i++) {
    list->samples[i] = initial_value;
  }

  list->sample_count = 1;
}

int PropPushSample(PropertySampleList *list, const PropertySample *sample)
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

void PropLerpSamples(const PropertySampleList *list, double time,
    PropertySample *dst)
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
      const PropertySample *sample0 = &list->samples[i-1];
      const PropertySample *sample1 = &list->samples[i];
      const double t = Fit(time, sample0->time, sample1->time, 0, 1);
      VEC4_LERP(dst->vector, sample0->vector, sample1->vector, t);
      return;
    }
  }
}

static int compare_property_sample(const void *ptr0, const void *ptr1)
{
  const PropertySample *sample0 = (const PropertySample *) ptr0;
  const PropertySample *sample1 = (const PropertySample *) ptr1;
  const double x = sample0->time;
  const double y = sample1->time;

  if (x > y)
    return 1;
  else if (x < y)
    return -1;
  else
    return 0;
}

static void push_sample(PropertySampleList *list, const PropertySample *sample)
{
  const int next_index = list->sample_count;
  assert(list->sample_count < MAX_PROPERTY_SAMPLES);

  list->samples[next_index] = *sample;
  list->sample_count++;
}

static void sort_by_sample_time(PropertySampleList *list)
{
  qsort(list->samples, list->sample_count,
      sizeof(PropertySample),
      compare_property_sample);
}

} // namespace xxx
