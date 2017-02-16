// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROPERTY_H
#define FJ_PROPERTY_H

#include "fj_vector.h"
#include "fj_types.h"
#include <cstddef>

namespace fj {

enum PropertyType {
  PROP_NONE = 0,
  PROP_SCALAR,
  PROP_VECTOR2,
  PROP_VECTOR3,
  PROP_VECTOR4,
  PROP_STRING,
  PROP_OBJECTGROUP,
  PROP_TURBULENCE,
  PROP_TEXTURE,
  PROP_SHADER,
  PROP_VOLUME,
  PROP_MESH
};

class ObjectGroup;
class Turbulence;
class Texture;
class Shader;
class Volume;
class Mesh;

class FJ_API PropertyValue {
public:
  PropertyValue() :
    type(PROP_NONE),
    vector(),
    string      (NULL),
    object_group(NULL),
    turbulence  (NULL),
    texture     (NULL),
    shader      (NULL),
    volume      (NULL),
    mesh        (NULL),
    time        (0) {}
  ~PropertyValue() {}

public:
  int type;

  Vector4 vector;
  const char *string;

  ObjectGroup *object_group;
  Turbulence *turbulence;
  Texture *texture;
  Shader *shader;
  Volume *volume;
  Mesh *mesh;

  Real time;
};

FJ_API PropertyValue PropNull();
FJ_API PropertyValue PropScalar(Real v0);
FJ_API PropertyValue PropVector2(Real v0, Real v1);
FJ_API PropertyValue PropVector3(Real v0, Real v1, Real v2);
FJ_API PropertyValue PropVector4(Real v0, Real v1, Real v2, Real v3);
FJ_API PropertyValue PropString(const char *string);

FJ_API PropertyValue PropObjectGroup(ObjectGroup *group);
FJ_API PropertyValue PropTurbulence(Turbulence *turbulence);
FJ_API PropertyValue PropTexture(Texture *texture);
FJ_API PropertyValue PropVolume(Volume *volume);
FJ_API PropertyValue PropMesh(Mesh *mesh);

class FJ_API Property {
public:
  typedef int (*SetValueFn)(void *self, const PropertyValue &value);

public:
  Property();
  Property(const char *name, const PropertyValue &value, SetValueFn set_value_fn);
  ~Property();

  bool IsValid() const;
  int GetType() const;
  const char *GetName() const;
  const Vector4 &GetDefaultValue() const;
  const char *GetTypeString() const;

  int SetValue(void *self, const PropertyValue &value) const;

private:
  int type_;
  const char *name_;
  Vector4 default_value_;
  SetValueFn set_value_fn_;
};

FJ_API const Property *PropFind(const Property *list, int type, const char *name);
FJ_API int PropSetAllDefaultValues(void *self, const Property *list);

/* for time variable properties */
enum { MAX_PROPERTY_SAMPLES = 8 };

class FJ_API PropertySample {
public:
  PropertySample() : vector(), time(0) {}
  ~PropertySample() {}

public:
  Real vector[4];
  Real time;
};

class FJ_API PropertySampleList {
public:
  PropertySampleList() : samples(), sample_count(0) {}
  ~PropertySampleList() {}

public:
  PropertySample samples[MAX_PROPERTY_SAMPLES];
  int sample_count;
};

FJ_API void PropInitSampleList(PropertySampleList *list);
FJ_API int PropPushSample(PropertySampleList *list, const PropertySample *sample);
FJ_API void PropLerpSamples(const PropertySampleList *list, Real time,
    PropertySample *dst);

} // namespace xxx

#endif // FJ_XXX_H
