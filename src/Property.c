/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Property.h"
#include <string.h>
#include <assert.h>

static int prop_is_valid(const struct Property *prop);

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

static int prop_is_valid(const struct Property *prop)
{
	if (prop->type == PROP_NONE)
		return 0;

	if (prop->name == NULL)
		return 0;

	return 1;
}

