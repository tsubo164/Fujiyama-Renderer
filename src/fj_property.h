/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PROPERTY_H
#define FJ_PROPERTY_H

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

class PropertyValue {
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

  Real vector[4];
  const char *string;

  ObjectGroup *object_group;
  Turbulence *turbulence;
  Texture *texture;
  Shader *shader;
  Volume *volume;
  Mesh *mesh;

  Real time;
};

class Property {
public:
  int type;
  const char *name;
  Real default_value[4];
  int (*SetProperty)(void *self, const PropertyValue *value);
};

extern PropertyValue PropScalar(Real v0);
extern PropertyValue PropVector2(Real v0, Real v1);
extern PropertyValue PropVector3(Real v0, Real v1, Real v2);
extern PropertyValue PropVector4(Real v0, Real v1, Real v2, Real v3);
extern PropertyValue PropString(const char *string);

extern PropertyValue PropObjectGroup(ObjectGroup *group);
extern PropertyValue PropTurbulence(Turbulence *turbulence);
extern PropertyValue PropTexture(Texture *texture);
extern PropertyValue PropVolume(Volume *volume);
extern PropertyValue PropMesh(Mesh *mesh);

extern int PropIsValid(const Property *prop);

extern const char *PropName(const Property *prop);
extern const int PropType(const Property *prop);
extern const Real PropDefaultValue(const Property *prop, int index);
extern const char *PropTypeString(const Property *prop);

extern const Property *PropFind(const Property *list, int type, const char *name);
extern int PropSetAllDefaultValues(void *self, const Property *list);

/* for time variable properties */
enum { MAX_PROPERTY_SAMPLES = 8 };

struct PropertySample {
  PropertySample() : vector(), time(0) {}
  ~PropertySample() {}

  Real vector[4];
  Real time;
};

struct PropertySampleList {
  PropertySample samples[MAX_PROPERTY_SAMPLES];
  int sample_count;
};

extern void PropInitSampleList(PropertySampleList *list);
extern int PropPushSample(PropertySampleList *list, const PropertySample *sample);
extern void PropLerpSamples(const PropertySampleList *list, Real time,
    PropertySample *dst);

} // namespace xxx

#endif // FJ_XXX_H
