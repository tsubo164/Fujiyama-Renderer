/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

struct Texture;
struct Shader;
struct Volume;

struct PropertyValue {
	double vector[4];
	struct Texture *texture;
	struct Shader *shader;
	struct Volume *volume;
	const void *pointer;
};

#define INIT_PROPERTYVALUE { \
	{0, 0, 0, 0}, \
	NULL, \
	NULL, \
	NULL, \
	NULL}

struct Property {
	const char *name;
	int (*SetProperty)(void *self, const struct PropertyValue *value);
};

extern struct PropertyValue InitPropValue(void);
extern const struct Property *PropFind(const struct Property *list, const char *name);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

