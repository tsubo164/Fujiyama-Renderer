/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PROP_NONE = 0,
	PROP_SCALAR,
	PROP_VECTOR2,
	PROP_VECTOR3,
	PROP_VECTOR4,
	PROP_TEXTURE,
	PROP_SHADER,
	PROP_VOLUME
};

struct Texture;
struct Shader;
struct Volume;

struct PropertyValue {
	int type;
	double vector[4];
	struct Texture *texture;
	struct Shader *shader;
	struct Volume *volume;
};

#define INIT_PROPERTYVALUE { \
	PROP_NONE, \
	{0, 0, 0, 0}, \
	NULL, \
	NULL, \
	NULL}

struct Property {
	int type;
	const char *name;
	int (*SetProperty)(void *self, const struct PropertyValue *value);
};

extern struct PropertyValue PropScalar(double v0);
extern struct PropertyValue PropVector2(double v0, double v1);
extern struct PropertyValue PropVector3(double v0, double v1, double v2);
extern struct PropertyValue PropVector4(double v0, double v1, double v2, double v3);

extern struct PropertyValue PropTexture(struct Texture *texture);
extern struct PropertyValue PropVolume(struct Volume *volume);

extern struct PropertyValue InitPropValue(void);
extern const struct Property *PropFind(const struct Property *list, int type, const char *name);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

