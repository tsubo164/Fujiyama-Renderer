/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "VolumeAccelerator.h"
#include "Intersection.h"
#include "Interval.h"
#include "Volume.h"
#include "Box.h"
#include "Ray.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>

#define EXPAND .0001
#define HALF_EXPAND (.5*EXPAND)

struct VolumeAccelerator {
	const char *name;
	double bounds[6];
	int has_built;

	/* TODO should make struct PrimitiveSet? */
	const void *volume_set;
	int num_volumes;
	double volume_set_bounds[6];
	VolumeIntersectFunction VolumeIntersect;
	VolumeBoundsFunction VolumeBounds;

	/* private */
	char *derived;
	void (*FreeDerived)(struct VolumeAccelerator *acc);
	int (*Build)(struct VolumeAccelerator *acc);
	int (*Intersect)(const struct VolumeAccelerator *acc, double time, const struct Ray *ray,
			struct IntervalList *intervals);
};

/* -------------------------------------------------------------------------- */
/* VolumeAccelerator */
static int ray_volume_intersect(const struct VolumeAccelerator *acc, int volume_id,
	double time, const struct Ray *ray, struct IntervalList *intervals);
#if 0
static void swap_isect_ptr(struct IntervalList **isect0, struct IntervalList **isect1);
#endif

/* -------------------------------------------------------------------------- */
/* VolumeBruteForceAccelerator */
struct VolumeBruteForceAccelerator {
	struct Volume **volume_list;
	int nvolumes;
};

static struct VolumeBruteForceAccelerator *new_bruteforce_accel(void);
static void free_bruteforce_accel(struct VolumeAccelerator *acc);
static int build_bruteforce_accel(struct VolumeAccelerator *acc);
static int intersect_bruteforce_accel(const struct VolumeAccelerator *acc, double time,
		const struct Ray *ray, struct IntervalList *intervals);

/* -------------------------------------------------------------------------- */
/* VolumeBVHAccelerator */
#if 0
enum { BVH_STACKSIZE = 64 };
enum {
	HIT_NONE = 0,
	HIT_LEFT = 1,
	HIT_RIGHT = 2,
	HIT_BOTH = 3
};
#endif

struct VolumePrimitive {
	double bounds[6];
	double centroid[3];
	int index;
};

struct VolumeBVHNode {
	struct VolumeBVHNode *left;
	struct VolumeBVHNode *right;
	double bounds[6];
	int volume_id;
};

#if 0
struct VolumeBVHNodeStack {
	int depth;
	const struct VolumeBVHNode *node[BVH_STACKSIZE];
};
#endif

struct VolumeBVHAccelerator {
	struct VolumeBVHNode *root;
};

static struct VolumeBVHAccelerator *new_bvh_accel(void);
static void free_bvh_accel(struct VolumeAccelerator *acc);
static int build_bvh_accel(struct VolumeAccelerator *acc);
static int intersect_bvh_accel(const struct VolumeAccelerator *acc, double time,
		const struct Ray *ray, struct IntervalList *intervals);

static int intersect_bvh_recursive(const struct VolumeAccelerator *acc,
		const struct VolumeBVHNode *node, double time,
		const struct Ray *ray, struct IntervalList *intervals);
#if 0
static int intersect_bvh_loop(const struct VolumeAccelerator *acc, const struct VolumeBVHNode *root,
		const struct Ray *ray, struct IntervalList *intervals);
#endif

static struct VolumeBVHNode *new_bvhnode(void);
static void free_bvhnode_recursive(struct VolumeBVHNode *node);
static struct VolumeBVHNode *build_bvh(struct VolumePrimitive **volumes, int begin, int end, int axis);
static int is_bvh_leaf(const struct VolumeBVHNode *node);
static int find_median(struct VolumePrimitive **volumes, int begin, int end, int axis);

static int compare_double(double x, double y);
static int volume_compare_x(const void *a, const void *b);
static int volume_compare_y(const void *a, const void *b);
static int volume_compare_z(const void *a, const void *b);

#if 0
static int is_empty(const struct VolumeBVHNodeStack *stack);
static void push_node(struct VolumeBVHNodeStack *stack, const struct VolumeBVHNode *node);
static const struct VolumeBVHNode *pop_node(struct VolumeBVHNodeStack *stack);
#endif

struct VolumeAccelerator *VolumeAccNew(int accelerator_type)
{
	struct VolumeAccelerator *acc = (struct VolumeAccelerator *) malloc(sizeof(struct VolumeAccelerator));
	if (acc == NULL)
		return NULL;

	switch (accelerator_type) {
	case VOLACC_BRUTEFORCE:
		acc->derived = (char *) new_bruteforce_accel();
		if (acc->derived == NULL) {
			VolumeAccFree(acc);
			return NULL;
		}
		acc->FreeDerived = free_bruteforce_accel;
		acc->Build = build_bruteforce_accel;
		acc->Intersect = intersect_bruteforce_accel;
		acc->name = "Brute Force";
		break;
	case VOLACC_BVH:
		acc->derived = (char *) new_bvh_accel();
		if (acc->derived == NULL) {
			VolumeAccFree(acc);
			return NULL;
		}
		acc->FreeDerived = free_bvh_accel;
		acc->Build = build_bvh_accel;
		acc->Intersect = intersect_bvh_accel;
		acc->name = "Volume BVH";
		break;
	default:
		assert(!"invalid accelerator type");
		break;
	}

	acc->has_built = 0;

	acc->volume_set = NULL;
	acc->num_volumes = 0;
	acc->VolumeIntersect = NULL;
	acc->VolumeBounds = NULL;

	BOX3_SET(acc->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	BOX3_SET(acc->volume_set_bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return acc;
}

void VolumeAccFree(struct VolumeAccelerator *acc)
{
	if (acc == NULL)
		return;
	if (acc->derived == NULL)
		return;

	acc->FreeDerived(acc);
	free(acc);
}

void VolumeAccGetBounds(const struct VolumeAccelerator *acc, double *bounds)
{
	BOX3_COPY(bounds, acc->bounds);
}

int VolumeAccBuild(struct VolumeAccelerator *acc)
{
	int err;

	if (acc->has_built)
		return -1;

	err = acc->Build(acc);
	if (err)
		return -1;

	acc->has_built = 1;
	return 0;
}

int VolumeAccIntersect(const struct VolumeAccelerator *acc, double time,
		const struct Ray *ray, struct IntervalList *intervals)
{
	double boxhit_tmin;
	double boxhit_tmax;

	/* check intersection with overall bounds */
	if (!BoxRayIntersect(acc->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	if (!acc->has_built) {
		/* dynamic build */
		printf("\nbuilding %s accelerator ...\n", acc->name);
		VolumeAccBuild((struct VolumeAccelerator *) acc);
		fflush(stdout);
	}

	return acc->Intersect(acc, time, ray, intervals);
}

void VolumeAccSetTargetGeometry(struct VolumeAccelerator *acc,
	const void *volume_set, int num_volumes, const double *volume_set_bounds,
	VolumeIntersectFunction volume_intersect_function,
	VolumeBoundsFunction volume_bounds_function)
{
	acc->volume_set = volume_set;
	acc->num_volumes = num_volumes;
	acc->VolumeIntersect = volume_intersect_function;
	acc->VolumeBounds = volume_bounds_function;
	BOX3_COPY(acc->volume_set_bounds, volume_set_bounds);

	/* accelerator's bounds */
	BOX3_COPY(acc->bounds, volume_set_bounds);
	BOX3_EXPAND(acc->bounds, EXPAND);
}

/* -------------------------------------------------------------------------- */
/* VolumeBruteForceAccelerator */
static struct VolumeBruteForceAccelerator *new_bruteforce_accel(void)
{
	struct VolumeBruteForceAccelerator *bruteforce;

	bruteforce = (struct VolumeBruteForceAccelerator *) malloc(sizeof(struct VolumeBruteForceAccelerator));
	if (bruteforce == NULL)
		return NULL;

	bruteforce->volume_list = NULL;
	bruteforce->nvolumes = 0;

	return bruteforce;
}

static void free_bruteforce_accel(struct VolumeAccelerator *acc)
{
	struct VolumeBruteForceAccelerator *bruteforce = (struct VolumeBruteForceAccelerator *) acc->derived;
	free(bruteforce);
}

static int build_bruteforce_accel(struct VolumeAccelerator *acc)
{
	return 0;
}

static int intersect_bruteforce_accel(const struct VolumeAccelerator *acc, double time,
		const struct Ray *ray, struct IntervalList *intervals)
{
	int hit;
	int i;

	hit = 0;
	for (i = 0; i < acc->num_volumes; i++) {
		hit += ray_volume_intersect(acc, i, time, ray, intervals);
	}

	return hit > 0;
}

/* -------------------------------------------------------------------------- */
/* VolumeBVHAccelerator */
static struct VolumeBVHAccelerator *new_bvh_accel(void)
{
	struct VolumeBVHAccelerator *bvh;

	bvh = (struct VolumeBVHAccelerator *) malloc(sizeof(struct VolumeBVHAccelerator));
	if (bvh == NULL)
		return NULL;

	bvh->root = NULL;

	return bvh;
}

static void free_bvh_accel(struct VolumeAccelerator *acc)
{
	struct VolumeBVHAccelerator *bvh = (struct VolumeBVHAccelerator *) acc->derived;

	free_bvhnode_recursive(bvh->root);
	free(bvh);
}

static int build_bvh_accel(struct VolumeAccelerator *acc)
{
	struct VolumeBVHAccelerator *bvh = (struct VolumeBVHAccelerator *) acc->derived;
	struct VolumePrimitive *volumes;
	struct VolumePrimitive **volume_ptr;
	int NPRIMS;
	int i;

	NPRIMS = acc->num_volumes;

	volumes = (struct VolumePrimitive *) malloc(sizeof(struct VolumePrimitive) * NPRIMS);
	if (volumes == NULL)
		return -1;

	volume_ptr = (struct VolumePrimitive **) malloc(sizeof(struct VolumePrimitive *) * NPRIMS);
	if (volume_ptr == NULL) {
		free(volumes);
		return -1;
	}

	for (i = 0; i < NPRIMS; i++) {
		acc->VolumeBounds(acc->volume_set, i, volumes[i].bounds);
		volumes[i].centroid[0] = (volumes[i].bounds[3] + volumes[i].bounds[0]) / 2;
		volumes[i].centroid[1] = (volumes[i].bounds[4] + volumes[i].bounds[1]) / 2;
		volumes[i].centroid[2] = (volumes[i].bounds[5] + volumes[i].bounds[2]) / 2;
		volumes[i].index = i;

		volume_ptr[i] = &volumes[i];
	}

	bvh->root = build_bvh(volume_ptr, 0, NPRIMS, 0);
	if (bvh->root == NULL) {
		free(volume_ptr);
		free(volumes);
		return -1;
	}

	free(volume_ptr);
	free(volumes);
	return 0;
}

static int intersect_bvh_accel(const struct VolumeAccelerator *acc, double time,
		const struct Ray *ray, struct IntervalList *intervals)
{
	const struct VolumeBVHAccelerator *bvh = (const struct VolumeBVHAccelerator *) acc->derived;

#if 0
	if (1)
		return intersect_bvh_loop(acc, bvh->root, ray, intervals);
	else
#endif
		return intersect_bvh_recursive(acc, bvh->root, time, ray, intervals);
}

static int intersect_bvh_recursive(const struct VolumeAccelerator *acc,
		const struct VolumeBVHNode *node, double time,
		const struct Ray *ray, struct IntervalList *intervals)
{
	double boxhit_tmin;
	double boxhit_tmax;
	int hit_left, hit_right;
	int hit;

	hit = BoxRayIntersect(node->bounds,
			ray->orig, ray->dir, ray->tmin, ray->tmax,
			&boxhit_tmin, &boxhit_tmax);
	if (!hit) {
		return 0;
	}

	if (is_bvh_leaf(node)) {
		return ray_volume_intersect(acc, node->volume_id, time, ray, intervals);
	}

	hit_left  = intersect_bvh_recursive(acc, node->left,  time, ray, intervals);
	hit_right = intersect_bvh_recursive(acc, node->right, time, ray, intervals);

	return hit_left || hit_right;
}

#if 0
static int intersect_bvh_loop(const struct VolumeAccelerator *acc, const struct VolumeBVHNode *root,
		const struct Ray *ray, struct IntervalList *intervals)
{
	int hit, hittmp;
	int whichhit;
	int hit_left, hit_right;
	double boxhit_tmin, boxhit_tmax;
	struct IntervalList isect_candidates[2];
	struct IntervalList *isect_min, *isect_tmp;

	const struct VolumeBVHNode *node;
	struct VolumeBVHNodeStack stack = {0, {NULL}};

	node = root;
	hit = 0;

	isect_min = &isect_candidates[0];
	isect_tmp = &isect_candidates[1];
	isect_min->t_hit = FLT_MAX;

	for (;;) {
		if (is_bvh_leaf(node)) {
			hittmp = ray_volume_intersect(acc, node->volume_id, ray, isect_tmp);
			if (hittmp && isect_tmp->t_hit < isect_min->t_hit) {
				swap_isect_ptr(&isect_min, &isect_tmp);
				hit = hittmp;
			}

			if (is_empty(&stack))
				goto loop_exit;
			node = pop_node(&stack);
			continue;
		}

		hit_left = BoxRayIntersect(node->left->bounds,
				ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax);

		hit_right = BoxRayIntersect(node->right->bounds,
				ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax);

		whichhit = HIT_NONE;
		whichhit |= hit_left  ? HIT_LEFT :  HIT_NONE;
		whichhit |= hit_right ? HIT_RIGHT : HIT_NONE;

		switch (whichhit) {
		case HIT_NONE:
			if (is_empty(&stack))
				goto loop_exit;
			node = pop_node(&stack);
			break;

		case HIT_LEFT:
			node = node->left;
			break;

		case HIT_RIGHT:
			node = node->right;
			break;

		case HIT_BOTH:
			push_node(&stack, node->right);
			node = node->left;
			break;

		default:
			assert(!"invalid whichhit");
			break;
		}
	}
loop_exit:

	if (hit) {
		*intervals = *isect_min;
	}

	return hit;
}
#endif

static struct VolumeBVHNode *build_bvh(struct VolumePrimitive **volume_ptr, int begin, int end, int axis)
{
	struct VolumeBVHNode *node = NULL;
	const int NPRIMS = end - begin;
	int median;
	int new_axis;
	int (*volume_compare)(const void *, const void *);

	node = new_bvhnode();
	if (node == NULL)
		return NULL;

	if (NPRIMS == 1) {
		node->volume_id = volume_ptr[begin]->index;
		BOX3_COPY(node->bounds, volume_ptr[begin]->bounds);
		return node;
	}

	switch (axis) {
		case 0:
			volume_compare = volume_compare_x;
			new_axis = 1;
			break;
		case 1:
			volume_compare = volume_compare_y;
			new_axis = 2;
			break;
		case 2:
			volume_compare = volume_compare_z;
			new_axis = 0;
			break;
		default:
			assert(!"invalid axis");
			break;
	}

	qsort(volume_ptr + begin, NPRIMS, sizeof(struct VolumePrimitive *), volume_compare);
	median = find_median(volume_ptr, begin, end, axis);

	node->left  = build_bvh(volume_ptr, begin, median, new_axis);
	if (node->left == NULL)
		return NULL;

	node->right = build_bvh(volume_ptr, median, end, new_axis);
	if (node->right == NULL)
		return NULL;

	BOX3_COPY(node->bounds, node->left->bounds);
	BoxAddBox(node->bounds, node->right->bounds);

	return node;
}

static struct VolumeBVHNode *new_bvhnode(void)
{
	struct VolumeBVHNode *node;

	node = (struct VolumeBVHNode *) malloc(sizeof(struct VolumeBVHNode));
	if (node == NULL)
		return NULL;

	node->left = NULL;
	node->right = NULL;
	node->volume_id = -1;
	BOX3_SET(node->bounds, 0, 0, 0, 0, 0, 0);

	return node;
}

static void free_bvhnode_recursive(struct VolumeBVHNode *node)
{
	if (node == NULL)
		return;

	if (is_bvh_leaf(node)) {
		free(node);
		return;
	}

	assert(node->left != NULL);
	assert(node->right != NULL);
	assert(node->volume_id == -1);

	free_bvhnode_recursive(node->left);
	free_bvhnode_recursive(node->right);

	free(node);
}

static int is_bvh_leaf(const struct VolumeBVHNode *node)
{
	return (
		node->left == NULL &&
		node->right == NULL &&
		node->volume_id != -1);
}

static int find_median(struct VolumePrimitive **volumes, int begin, int end, int axis)
{
	int low, high, mid;
	double key;

	assert(axis >= 0 && axis <= 2);

	low = begin;
	high = end - 1;
	mid = -1;
	key = (volumes[low]->centroid[axis] + volumes[high]->centroid[axis]) / 2;

	while (low != mid) {
		mid = (low + high) / 2;
		if (key < volumes[mid]->centroid[axis])
			high = mid;
		else if (volumes[mid]->centroid[axis] < key)
			low = mid;
		else
			break;
	}

	return mid + 1;
}

static int compare_double(double x, double y)
{
	if (x > y)
		return 1;
	else if (x < y)
		return -1;
	else
		return 0;
}

static int volume_compare_x(const void *a, const void *b)
{
	struct VolumePrimitive **A = (struct VolumePrimitive **) a;
	struct VolumePrimitive **B = (struct VolumePrimitive **) b;
	return compare_double((*A)->centroid[0], (*B)->centroid[0]);
}

static int volume_compare_y(const void *a, const void *b)
{
	struct VolumePrimitive **A = (struct VolumePrimitive **) a;
	struct VolumePrimitive **B = (struct VolumePrimitive **) b;
	return compare_double((*A)->centroid[1], (*B)->centroid[1]);
}

static int volume_compare_z(const void *a, const void *b)
{
	struct VolumePrimitive **A = (struct VolumePrimitive **) a;
	struct VolumePrimitive **B = (struct VolumePrimitive **) b;
	return compare_double((*A)->centroid[2], (*B)->centroid[2]);
}

static int ray_volume_intersect (const struct VolumeAccelerator *acc, int volume_id,
	double time, const struct Ray *ray, struct IntervalList *intervals)
{
	struct Interval interval;
	int hit;

	hit = acc->VolumeIntersect(acc->volume_set, volume_id, time, ray, &interval);
	if (!hit) {
		return 0;
	}

	if (interval.tmax < ray->tmin || ray->tmax < interval.tmin) {
		return 0;
	}

	IntervalListPush(intervals, &interval);
	return 1;
}

#if 0
static void swap_isect_ptr(struct IntervalList **isect0, struct IntervalList **isect1)
{
	struct IntervalList *isect_swp = *isect0;
	*isect0 = *isect1;
	*isect1 = isect_swp;
}

static int is_empty(const struct VolumeBVHNodeStack *stack)
{
	return (stack->depth == 0);
}

static void push_node(struct VolumeBVHNodeStack *stack, const struct VolumeBVHNode *node)
{
	stack->node[stack->depth++] = node;
	assert(stack->depth < BVH_STACKSIZE);
}

static const struct VolumeBVHNode *pop_node(struct VolumeBVHNodeStack *stack)
{
	assert(!is_empty(stack));
	return stack->node[--stack->depth];
}
#endif

