/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

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

struct ObjectGroup;
struct Turbulence;
struct Texture;
struct Shader;
struct Volume;
struct Mesh;

struct PropertyValue {
	int type;

	double vector[4];
	const char *string;

	struct ObjectGroup *object_group;
	struct Turbulence *turbulence;
	struct Texture *texture;
	struct Shader *shader;
	struct Volume *volume;
	struct Mesh *mesh;

	double time;
};

#define INIT_PROPERTYVALUE { \
	PROP_NONE, \
	{0, 0, 0, 0}, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	0}

struct Property {
	int type;
	const char *name;
	int (*SetProperty)(void *self, const struct PropertyValue *value);
};

extern const char *PropTypeString(int property_type);

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

extern struct PropertyValue InitPropValue(void);
extern const struct Property *PropFind(const struct Property *list, int type, const char *name);

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

