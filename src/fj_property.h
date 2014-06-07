/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_PROPERTY_H
#define FJ_PROPERTY_H

#include "fj_types.h"
#include <algorithm>

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
  PropertyValue()
  {
    type = PROP_NONE;
    std::fill(vector, vector + 4, 0);
    string       = NULL;
    object_group = NULL;
    turbulence   = NULL;
    texture      = NULL;
    shader       = NULL;
    volume       = NULL;
    mesh         = NULL;
    time         = 0;
  }
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

extern struct PropertyValue PropScalar(double v0);
extern struct PropertyValue PropVector2(double v0, double v1);
extern struct PropertyValue PropVector3(double v0, double v1, double v2);
extern struct PropertyValue PropVector4(double v0, double v1, double v2, double v3);
extern struct PropertyValue PropString(const char *string);

extern struct PropertyValue PropObjectGroup(struct ObjectGroup *group);
extern struct PropertyValue PropTurbulence(struct Turbulence *turbulence);
extern struct PropertyValue PropTexture(struct Texture *texture);
extern struct PropertyValue PropVolume(struct Volume *volume);
extern struct PropertyValue PropMesh(struct Mesh *mesh);

extern int PropIsValid(const struct Property *prop);

extern const char *PropName(const struct Property *prop);
extern const int PropType(const struct Property *prop);
extern const double PropDefaultValue(const struct Property *prop, int index);
extern const char *PropTypeString(const struct Property *prop);

extern const struct Property *PropFind(const struct Property *list, int type, const char *name);
extern int PropSetAllDefaultValues(void *self, const struct Property *list);

/* for time variable properties */
enum { MAX_PROPERTY_SAMPLES = 8 };

struct PropertySample {
  double vector[4];
  double time;
};

#define INIT_PROPERTYSAMPLE { \
  {0, 0, 0, 0}, \
  0}

struct PropertySampleList {
  struct PropertySample samples[MAX_PROPERTY_SAMPLES];
  int sample_count;
};

extern void PropInitSampleList(struct PropertySampleList *list);
extern int PropPushSample(struct PropertySampleList *list, const struct PropertySample *sample);
extern void PropLerpSamples(const struct PropertySampleList *list, double time,
    struct PropertySample *dst);

} // namespace xxx

#endif /* FJ_XXX_H */
