/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Light.h"
#include "Vector.h"
#include <stdlib.h>

struct Light {
	double position[3];
	float color[3];
	float intensity;
};

struct Light *LgtNew(const char *type)
{
	struct Light *light;

	light = (struct Light *) malloc(sizeof(struct Light));
	if (light == NULL)
		return NULL;

	VEC3_SET(light->position, 0, 0, 0);
	VEC3_SET(light->color, 1, 1, 1);
	light->intensity = 1;

	return light;
}

void LgtFree(struct Light *light)
{
	if (light == NULL)
		return;
	free(light);
}

void LgtSetPosition(struct Light *light, double xpos, double ypos, double zpos)
{
	light->position[0] = xpos;
	light->position[1] = ypos;
	light->position[2] = zpos;
}

void LgtSetColor(struct Light *light, float r, float g, float b)
{
	light->color[0] = r;
	light->color[1] = g;
	light->color[2] = b;
}

void LgtSetIntensity(struct Light *light, double intensity)
{
	light->intensity = intensity;
}

const double *LgtGetPosition(const struct Light *light)
{
	return light->position;
}

void LgtIlluminate(const struct Light *light, const double *Ps, float *Cl)
{
	Cl[0] = light->intensity * light->color[0];
	Cl[1] = light->intensity * light->color[1];
	Cl[2] = light->intensity * light->color[2];
}

