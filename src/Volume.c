/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Volume.h"
#include "LocalGeometry.h"
#include "Accelerator.h"
#include "Array.h"
#include "Box.h"
#include "Ray.h"
/*
#include "Transform.h"
#include "Numeric.h"
#include "Matrix.h"
#include "Vector.h"
*/
#include <stdlib.h>
#include <float.h>
/*
#include <string.h>
#include <assert.h>
#include <math.h>
*/
#include <stdio.h>

/* volume types */
struct VoxelBuffer {
	float *data;
	int size[3];
};

struct Volume {
	double bounds[6];
#if 0
	double bounds_expand[6];
#endif
	int nbuffers;
};

struct VolumeList {
	struct Array *volumes;
};

/* volume interfaces */
static int volume_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void volume_bounds(const void *prim_set, int prim_id, double *bounds);

struct Volume *VolNew(void)
{
	struct Volume *volume;

	volume = (struct Volume *) malloc(sizeof(struct Volume));
	if (volume == NULL)
		return NULL;

	BOX3_SET(volume->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	volume->nbuffers = 0;

	/* XXX TEST */
	BOX3_SET(volume->bounds, -1, -1, -1, 1, 1, 1);
#if 0
	BOX3_COPY(volume->bounds_expand, volume->bounds);
	BOX3_EXPAND(volume->bounds_expand, .000001);
#endif
	volume->nbuffers = 1;

	return volume;
}

void VolFree(struct Volume *volume)
{
	if (volume == NULL)
		return;

	free(volume);
}

void VolSetupAccelerator(const struct Volume *volume, struct Accelerator *acc)
{
	AccSetTargetGeometry(acc,
			ACC_PRIM_VOLUME,
			volume,
			volume->nbuffers,
			volume->bounds,
			volume_ray_intersect,
			volume_bounds);
}

/*
struct VolumeList *VolumeListNew(void)
{
	struct VolumeList *list;

	list = (struct VolumeList *) malloc(sizeof(struct VolumeList));
	if (list == NULL)
		return NULL;

	list->volumes = ArrNew(sizeof(struct ObjectInstance *));
	if (list->volumes == NULL) {
		VolumeListFree(list);
		return NULL;
	}

	return list;
}

void VolumeListFree(struct VolumeList *list)
{
	if (list == NULL)
		return;

	if (list->volumes != NULL)
		ArrFree(list->volumes);

	free(list);
}

void VolumeListAdd(struct VolumeList *list, const struct Volume *vol)
{
	ArrPushPointer(list->volumes, vol);
}
*/

static int volume_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct Volume *volume = (const struct Volume *) prim_set;
	int hit;
	double boxhit_tmin, boxhit_tmax;
	double bounds_expand[6];

	BOX3_COPY(bounds_expand, volume->bounds);
	BOX3_EXPAND(bounds_expand, .000001);
	if (BoxContainsPoint(bounds_expand, ray->orig)) {
		*t_hit = .05;
		return 1;
	}
#if 0
	if (BoxContainsPoint(volume->bounds_expand, ray->orig)) {
		*t_hit = .05;
		return 1;
	}
#endif

	hit = BoxRayIntersect(volume->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax);

	if (!hit) {
		/*
		printf("MISS!!\n");
		*/
		return 0;
	}

	*t_hit = boxhit_tmin;
	return hit;
}

static void volume_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Volume *volume = (const struct Volume *) prim_set;

	BOX3_COPY(bounds, volume->bounds);
}

/* XXX TEST */
int VolSample(const struct Volume *volume, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	int hit;
	double boxhit_tmin, boxhit_tmax;
	double bounds_expand[6];

	BOX3_COPY(bounds_expand, volume->bounds);
	BOX3_EXPAND(bounds_expand, .000001);
	if (BoxContainsPoint(bounds_expand, ray->orig)) {
		*t_hit = .05;
		return 1;
	}
#if 0
	if (BoxContainsPoint(volume->bounds_expand, ray->orig)) {
		*t_hit = .05;
		return 1;
	}
#endif

	hit = BoxRayIntersect(volume->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax);

	if (!hit) {
		return 0;
	}

	if (hit) {
		*t_hit = boxhit_tmin;
	}

	return hit;
}

