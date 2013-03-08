/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Accelerator.h"
#include "Intersection.h"
#include "PrimitiveSet.h"
#include "Numeric.h"
#include "Vector.h"
#include "Box.h"
#include "Ray.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>

#define EXPAND .0001
#define HALF_EXPAND (.5*EXPAND)

#define COPY3(dst,a) do { \
	(dst)[0] = (a)[0]; \
	(dst)[1] = (a)[1]; \
	(dst)[2] = (a)[2]; \
	} while(0)

/* -------------------------------------------------------------------------- */
/* GridAccelerator */
enum { GRID_MAXCELLS = 512 };

struct Cell {
	int prim_id;
	struct Cell *next;
};

struct GridAccelerator {
	struct Cell **cells;
	int ncells[3];
	double cellsize[3];
	struct Box bounds;
};

static struct GridAccelerator *new_grid_accel(void);
static void free_grid_accel(struct Accelerator *acc);
static int build_grid_accel(struct Accelerator *acc);
static int intersect_grid_accel(const struct Accelerator *acc, double time,
		const struct Ray *ray, struct Intersection *isect);
static void compute_grid_cellsizes(int nprimitives,
		double xwidth, double ywidth, double zwidth,
		int *xncells, int *yncells, int *zncells);

/* -------------------------------------------------------------------------- */
/* BVHAccelerator */
enum { BVH_STACKSIZE = 64 };
enum {
	HIT_NONE = 0,
	HIT_LEFT = 1,
	HIT_RIGHT = 2,
	HIT_BOTH = 3
};

struct Primitive {
	struct Box bounds;
	double centroid[3];
	int index;
};

struct BVHNode {
	struct BVHNode *left;
	struct BVHNode *right;
	struct Box bounds;
	int prim_id;
};

struct BVHNodeStack {
	int depth;
	const struct BVHNode *node[BVH_STACKSIZE];
};

struct BVHAccelerator {
	struct BVHNode *root;
};

static struct BVHAccelerator *new_bvh_accel(void);
static void free_bvh_accel(struct Accelerator *acc);
static int build_bvh_accel(struct Accelerator *acc);
static int intersect_bvh_accel(const struct Accelerator *acc, double time,
		const struct Ray *ray, struct Intersection *isect);
static int intersect_bvh_recursive(const struct Accelerator *acc, const struct BVHNode *node,
		double time, const struct Ray *ray, struct Intersection *isect);
static int intersect_bvh_loop(const struct Accelerator *acc, const struct BVHNode *root,
		double time, const struct Ray *ray, struct Intersection *isect);

static struct BVHNode *new_bvhnode(void);
static void free_bvhnode_recursive(struct BVHNode *node);
static struct BVHNode *build_bvh(struct Primitive **prims, int begin, int end, int axis);
static int is_bvh_leaf(const struct BVHNode *node);
static int find_median(struct Primitive **prims, int begin, int end, int axis);

static int compare_double(double x, double y);
static int primitive_compare_x(const void *a, const void *b);
static int primitive_compare_y(const void *a, const void *b);
static int primitive_compare_z(const void *a, const void *b);

static int is_empty(const struct BVHNodeStack *stack);
static void push_node(struct BVHNodeStack *stack, const struct BVHNode *node);
static const struct BVHNode *pop_node(struct BVHNodeStack *stack);

/* -------------------------------------------------------------------------- */
/* Accelerator */
static int prim_ray_intersect(const struct Accelerator *acc, int prim_id, double time,
	const struct Ray *ray, struct Intersection *isect);
static void get_prim_bounds(const struct Accelerator *acc, int prim_id, struct Box *bounds);
static void swap_isect_ptr(struct Intersection **isect0, struct Intersection **isect1);

struct Accelerator {
	const char *name;
	struct Box bounds;
	int has_built;

	struct PrimitiveSet primset;

	/* private */
	char *derived;
	void (*FreeDerived)(struct Accelerator *acc);
	int (*Build)(struct Accelerator *acc);
	int (*Intersect)(const struct Accelerator *acc, double time,
			const struct Ray *ray, struct Intersection *isect);
};

struct Accelerator *AccNew(int accelerator_type)
{
	struct Accelerator *acc = (struct Accelerator *) malloc(sizeof(struct Accelerator));
	if (acc == NULL)
		return NULL;

	switch (accelerator_type) {
	case ACC_GRID:
		acc->derived = (char *) new_grid_accel();
		if (acc->derived == NULL) {
			AccFree(acc);
			return NULL;
		}
		acc->FreeDerived = free_grid_accel;
		acc->Build = build_grid_accel;
		acc->Intersect = intersect_grid_accel;
		acc->name = "Uniform-Grid";
		break;
	case ACC_BVH:
		acc->derived = (char *) new_bvh_accel();
		if (acc->derived == NULL) {
			AccFree(acc);
			return NULL;
		}
		acc->FreeDerived = free_bvh_accel;
		acc->Build = build_bvh_accel;
		acc->Intersect = intersect_bvh_accel;
		acc->name = "BVH";
		break;
	default:
		assert(!"invalid accelerator type");
		break;
	}

	acc->has_built = 0;

	InitPrimitiveSet(&acc->primset);
	BOX3_SET(&acc->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return acc;
}

void AccFree(struct Accelerator *acc)
{
	if (acc == NULL)
		return;
	if (acc->derived == NULL)
		return;

	acc->FreeDerived(acc);
	free(acc);
}

void AccGetBounds(const struct Accelerator *acc, struct Box *bounds)
{
	*bounds = acc->bounds;
}

void AccComputeBounds(struct Accelerator *acc)
{
	PrmGetBounds(&acc->primset, &acc->bounds);
	BOX3_EXPAND(&acc->bounds, EXPAND);
}

int AccBuild(struct Accelerator *acc)
{
	int err = 0;

	if (acc->has_built)
		return -1;

	err = acc->Build(acc);
	if (err)
		return -1;

	acc->has_built = 1;
	return 0;
}

int AccIntersect(const struct Accelerator *acc, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	double boxhit_tmin;
	double boxhit_tmax;

	/* check intersection with overall bounds */
	if (!BoxRayIntersect(&acc->bounds, &ray->orig, &ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	if (!acc->has_built) {
		/* dynamic build */
		printf("\nbuilding %s accelerator ...\n", acc->name);
		AccBuild((struct Accelerator *) acc);
		fflush(stdout);
	}

	return acc->Intersect(acc, time, ray, isect);
}

void AccSetPrimitiveSet(struct Accelerator *acc, const struct PrimitiveSet *primset)
{
	acc->primset = *primset;

	/* accelerator's bounds */
	PrmGetBounds(&acc->primset, &acc->bounds);
	BOX3_EXPAND(&acc->bounds, EXPAND);
}

/* -------------------------------------------------------------------------- */
static int intersect_grid_accel(const struct Accelerator *acc, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const struct GridAccelerator *grid = (const struct GridAccelerator *) acc->derived;
	double grid_min[3];
	double start[3];
	double tstart = FLT_MAX;
	double tend = FLT_MAX;
	double boxhit_tmin;
	double boxhit_tmax;

	int i;
	int hit;
	int NCELLS[3];
	int cellid[3];
	int cellstep[3];
	int cellend[3];
	double tnext[3];
	double tdelt[3];
	double dir[3];

	/* check intersection with overall bounds */
	/* to get boxhit_tmin and boxhit_tmax */
	if (!BoxRayIntersect(&acc->bounds, &ray->orig, &ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	/* check if the ray shot from inside bounds */
	if (BoxContainsPoint(&grid->bounds, &ray->orig)) {
		start[0] = ray->orig.x;
		start[1] = ray->orig.y;
		start[2] = ray->orig.z;
		tstart = 0;
	}
	else {
		tstart = boxhit_tmin;
		tend = boxhit_tmax;
		start[0] = ray->orig.x + ray->dir.x * tstart;
		start[1] = ray->orig.y + ray->dir.y * tstart;
		start[2] = ray->orig.z + ray->dir.z * tstart;
	}
	tend = MIN(tend, ray->tmax);

	COPY3(NCELLS, grid->ncells);

	/* TODO grid_min is static? */
	grid_min[0] = grid->bounds.min.x;
	grid_min[1] = grid->bounds.min.y;
	grid_min[2] = grid->bounds.min.z;
	dir[0] = ray->dir.x;
	dir[1] = ray->dir.y;
	dir[2] = ray->dir.z;
	/* setup 3D DDA */
	for (i = 0; i < 3; i++) {
		cellid[i] = (int) floor((start[i] - grid_min[i]) / grid->cellsize[i]);
		cellid[i] = CLAMP(cellid[i], 0, NCELLS[i]-1);

		if (dir[i] > 0) {
			tnext[i] = tstart +
				(((cellid[i]+1) * grid->cellsize[i] + grid_min[i]) - start[i]) / dir[i];

			tdelt[i] = grid->cellsize[i] / dir[i];
			cellstep[i] = +1;
			cellend[i] = NCELLS[i];
		}
		else if (dir[i] <0) {
			tnext[i] = tstart +
				((cellid[i] * grid->cellsize[i] + grid_min[i]) - start[i]) / dir[i];

			tdelt[i] = -1 * grid->cellsize[i] / dir[i];
			cellstep[i] = -1;
			cellend[i] = -1;
		}
		else {
			tnext[i] = FLT_MAX;
			tdelt[i] = 0;
			cellstep[i] = 0;
			cellend[i] = -1;
		}
	}

	/* traverse voxels */
	hit = 0;
	for (;;) {
		int id;
		struct Cell *cell;
		struct Intersection isect_candidates[2];
		struct Intersection *isect_min, *isect_tmp;

		isect_min = &isect_candidates[0];
		isect_tmp = &isect_candidates[1];
		isect_min->t_hit = FLT_MAX;

		id = cellid[2] * NCELLS[0] * NCELLS[1] + cellid[1] * NCELLS[0] + cellid[0];

		/* loop over face list that associated in current cell */
		for (cell = grid->cells[id]; cell != NULL; cell = cell->next) {
			int hittmp;
			int inside_cell;
			struct Vector P_hit;
			struct Box cellbox;

			hittmp = prim_ray_intersect(acc, cell->prim_id, time, ray, isect_tmp);
			if (!hittmp)
				continue;

			/* check if the hit point is inside the cell */
			cellbox.min.x = grid->bounds.min.x + cellid[0] * grid->cellsize[0];
			cellbox.min.y = grid->bounds.min.y + cellid[1] * grid->cellsize[1];
			cellbox.min.z = grid->bounds.min.z + cellid[2] * grid->cellsize[2];
			cellbox.max.x = cellbox.min.x + grid->cellsize[0];
			cellbox.max.y = cellbox.min.y + grid->cellsize[1];
			cellbox.max.z = cellbox.min.z + grid->cellsize[2];
			POINT_ON_RAY(&P_hit, &ray->orig, &ray->dir, isect_tmp->t_hit);
			inside_cell = BoxContainsPoint(&cellbox, &P_hit);

			if (!inside_cell)
				continue;

			/* update info ONLY if isect->t_hit renewed */
			if (isect_tmp->t_hit < isect_min->t_hit) {
				swap_isect_ptr(&isect_min, &isect_tmp);
				hit = hittmp;
			}
		}
		if (hit) {
			*isect = *isect_min;
			break;
		}

		/* advance to the next cell */
		if ((tnext[0] < tnext[1]) && (tnext[0] < tnext[2])) {
			if (tend < tnext[0])
				break;
			cellid[0] += cellstep[0];
			if (cellid[0] == cellend[0])
				break;
			tnext[0] += tdelt[0];
		}
		else if ((tnext[2] < tnext[1])) {
			if (tend < tnext[2])
				break;
			cellid[2] += cellstep[2];
			if (cellid[2] == cellend[2])
				break;
			tnext[2] += tdelt[2];
		}
		else {
			if (tend < tnext[1])
				break;
			cellid[1] += cellstep[1];
			if (cellid[1] == cellend[1])
				break;
			tnext[1] += tdelt[1];
		}
	}
	return hit;
}

static int build_grid_accel(struct Accelerator *acc)
{
	int i;
	int NPRIMS;
	int XNCELLS;
	int YNCELLS;
	int ZNCELLS;
	struct Box bounds = {{0}};
	double cellsize[3] = {0};
	struct Cell **cells = NULL;

	struct GridAccelerator *grid = (struct GridAccelerator *) acc->derived;

	NPRIMS = PrmGetPrimitiveCount(&acc->primset);
	compute_grid_cellsizes(NPRIMS, 
			acc->bounds.max.x - acc->bounds.min.x,
			acc->bounds.max.y - acc->bounds.min.y,
			acc->bounds.max.z - acc->bounds.min.z,
			&XNCELLS, &YNCELLS, &ZNCELLS);

	cells = (struct Cell **) malloc(sizeof(struct Cell *) * XNCELLS * YNCELLS * ZNCELLS);
	if (cells == NULL)
		return -1;

	for (i = 0; i < XNCELLS * YNCELLS * ZNCELLS; i++)
		cells[i] = NULL;

	bounds = acc->bounds;
	cellsize[0] = (bounds.max.x - bounds.min.x) / XNCELLS;
	cellsize[1] = (bounds.max.y - bounds.min.y) / YNCELLS;
	cellsize[2] = (bounds.max.z - bounds.min.z) / ZNCELLS;

	for (i = 0; i < NPRIMS; i++) {
		int X0, X1, Y0, Y1, Z0, Z1;
		int x, y, z;
		int primid = i;
		struct Box primbbox;

		get_prim_bounds(acc, primid, &primbbox);
		BOX3_EXPAND(&primbbox, HALF_EXPAND);

		/* compute the ranges of cell indices. e.g. [X0 .. X1) */
		X0 = (int) floor((primbbox.min.x - bounds.min.x) / cellsize[0]);
		X1 = (int) floor((primbbox.max.x - bounds.min.x) / cellsize[0]) + 1;
		Y0 = (int) floor((primbbox.min.y - bounds.min.y) / cellsize[1]);
		Y1 = (int) floor((primbbox.max.y - bounds.min.y) / cellsize[1]) + 1;
		Z0 = (int) floor((primbbox.min.z - bounds.min.z) / cellsize[2]);
		Z1 = (int) floor((primbbox.max.z - bounds.min.z) / cellsize[2]) + 1;
		X0 = CLAMP(X0, 0, XNCELLS);
		X1 = CLAMP(X1, 0, XNCELLS);
		Y0 = CLAMP(Y0, 0, YNCELLS);
		Y1 = CLAMP(Y1, 0, YNCELLS);
		Z0 = CLAMP(Z0, 0, ZNCELLS);
		Z1 = CLAMP(Z1, 0, ZNCELLS);

		/* add cell list which holds face id inside the cell */
		for (z = Z0; z < Z1; z++) {
			for (y = Y0; y < Y1; y++) {
				for (x = X0; x < X1; x++) {
					int cellid = z * YNCELLS * XNCELLS + y * XNCELLS + x;
					struct Cell *newcell = (struct Cell *) malloc(sizeof(struct Cell));

					if (newcell == NULL) {
						/* TODO error handling */
						return -1;
#if 0
						continue;
#endif
					}
					newcell->prim_id = primid;
					newcell->next = NULL;

					if (cells[cellid] == NULL) {
						cells[cellid] = newcell;
					} else {
						struct Cell *oldcell = cells[cellid];
						cells[cellid] = newcell;
						cells[cellid]->next = oldcell;
					}
				}
			}
		}
	}

	/* commit */
	grid->cells = cells;
	grid->ncells[0] = XNCELLS;
	grid->ncells[1] = YNCELLS;
	grid->ncells[2] = ZNCELLS;
	grid->cellsize[0] = cellsize[0];
	grid->cellsize[1] = cellsize[1];
	grid->cellsize[2] = cellsize[2];
	grid->bounds = bounds;

	return 0;
}

static struct GridAccelerator *new_grid_accel(void)
{
	struct GridAccelerator *grid =
			(struct GridAccelerator *) malloc(sizeof(struct GridAccelerator));

	if (grid == NULL)
		return NULL;

	grid->cells = NULL;
	grid->ncells[0] = 0;
	grid->ncells[1] = 0;
	grid->ncells[2] = 0;
	grid->cellsize[0] = 0;
	grid->cellsize[1] = 0;
	grid->cellsize[2] = 0;
	BOX3_SET(&grid->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return grid;
}

static void free_grid_accel(struct Accelerator *acc)
{
	int x, y, z;
	struct GridAccelerator *grid = (struct GridAccelerator *) acc->derived;
	const int XNCELLS = grid->ncells[0];
	const int YNCELLS = grid->ncells[1];
	const int ZNCELLS = grid->ncells[2];

	for (z = 0; z < ZNCELLS; z++) {
		for (y = 0; y < YNCELLS; y++) {
			for (x = 0; x < XNCELLS; x++) {
				int cellid = z * YNCELLS * XNCELLS + y * XNCELLS + x;
				struct Cell *cell = grid->cells[cellid];

				while (cell != NULL) {
					struct Cell *kill = cell;
					struct Cell *next = cell->next;
					free(kill);
					cell = next;
				}
			}
		}
	}
	free(grid->cells);
	free(grid);
}

static void compute_grid_cellsizes(int nprimitives,
		double xwidth, double ywidth, double zwidth,
		int *xncells, int *yncells, int *zncells)
{
	double max_width = 0;
	double ncells_per_unit_dist;
	double cube_root;
	int XNCELLS;
	int YNCELLS;
	int ZNCELLS;

	if (xwidth > ywidth && xwidth > zwidth) {
		max_width = xwidth;
	}
	else if (ywidth > zwidth) {
		max_width = ywidth;
	}
	else {
		max_width = zwidth;
	}

	cube_root = 3 * pow(nprimitives, 1./3);
	ncells_per_unit_dist = cube_root / max_width;

	XNCELLS = (int) floor(xwidth * ncells_per_unit_dist + .5);
	YNCELLS = (int) floor(ywidth * ncells_per_unit_dist + .5);
	ZNCELLS = (int) floor(zwidth * ncells_per_unit_dist + .5);
	XNCELLS = CLAMP(XNCELLS, 1, GRID_MAXCELLS);
	YNCELLS = CLAMP(YNCELLS, 1, GRID_MAXCELLS);
	ZNCELLS = CLAMP(ZNCELLS, 1, GRID_MAXCELLS);

	*xncells = XNCELLS;
	*yncells = YNCELLS;
	*zncells = ZNCELLS;
}

/* -------------------------------------------------------------------------- */
static struct BVHAccelerator *new_bvh_accel(void)
{
	struct BVHAccelerator *bvh =
			(struct BVHAccelerator *) malloc(sizeof(struct BVHAccelerator));

	if (bvh == NULL)
		return NULL;

	bvh->root = NULL;

	return bvh;
}

static void free_bvh_accel(struct Accelerator *acc)
{
	struct BVHAccelerator *bvh = (struct BVHAccelerator *) acc->derived;

	free_bvhnode_recursive(bvh->root);

	free(bvh);
}

static int build_bvh_accel(struct Accelerator *acc)
{
	struct BVHAccelerator *bvh = (struct BVHAccelerator *) acc->derived;
	struct Primitive *prims;
	struct Primitive **primptrs;
	int NPRIMS;
	int i;

	NPRIMS = PrmGetPrimitiveCount(&acc->primset);

	prims = (struct Primitive *) malloc(sizeof(struct Primitive) * NPRIMS);
	if (prims == NULL)
		return -1;

	primptrs = (struct Primitive **) malloc(sizeof(struct Primitive *) * NPRIMS);
	if (primptrs == NULL) {
		free(prims);
		return -1;
	}

	for (i = 0; i < NPRIMS; i++) {
		get_prim_bounds(acc, i, &prims[i].bounds);
		prims[i].centroid[0] = (prims[i].bounds.max.x + prims[i].bounds.min.x) / 2;
		prims[i].centroid[1] = (prims[i].bounds.max.y + prims[i].bounds.min.y) / 2;
		prims[i].centroid[2] = (prims[i].bounds.max.z + prims[i].bounds.min.z) / 2;
		prims[i].index = i;

		primptrs[i] = &prims[i];
	}

	bvh->root = build_bvh(primptrs, 0, NPRIMS, 0);
	if (bvh->root == NULL) {
		free(primptrs);
		free(prims);
		return -1;
	}

	free(primptrs);
	free(prims);
	return 0;
}

static int intersect_bvh_accel(const struct Accelerator *acc, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const struct BVHAccelerator *bvh = (const struct BVHAccelerator *) acc->derived;

	if (1)
		return intersect_bvh_loop(acc, bvh->root, time, ray, isect);
	else
		return intersect_bvh_recursive(acc, bvh->root, time, ray, isect);
}

static int intersect_bvh_recursive(const struct Accelerator *acc, const struct BVHNode *node,
		double time, const struct Ray *ray, struct Intersection *isect)
{
	struct Intersection isect_left, isect_right;
	double boxhit_tmin;
	double boxhit_tmax;
	int hit_left, hit_right;

	const int hit = BoxRayIntersect(&node->bounds,
			&ray->orig, &ray->dir, ray->tmin, ray->tmax,
			&boxhit_tmin, &boxhit_tmax);

	if (!hit) {
		return 0;
	}

	if (is_bvh_leaf(node)) {
		return prim_ray_intersect(acc, node->prim_id, time, ray, isect);
	}

	isect_left.t_hit = FLT_MAX;
	hit_left  = intersect_bvh_recursive(acc, node->left,  time, ray, &isect_left);
	isect_right.t_hit = FLT_MAX;
	hit_right = intersect_bvh_recursive(acc, node->right, time, ray, &isect_right);

	if (isect_left.t_hit < ray->tmin)
		isect_left.t_hit = FLT_MAX;
	if (isect_right.t_hit < ray->tmin)
		isect_right.t_hit = FLT_MAX;

	if (isect_left.t_hit < isect_right.t_hit) {
		*isect = isect_left;
	} else if (isect_right.t_hit < isect_left.t_hit) {
		*isect = isect_right;
	}

	return (hit_left || hit_right);
}

static int intersect_bvh_loop(const struct Accelerator *acc, const struct BVHNode *root,
		double time, const struct Ray *ray, struct Intersection *isect)
{
	int hit, hittmp;
	int whichhit;
	int hit_left, hit_right;
	double boxhit_tmin, boxhit_tmax;
	struct Intersection isect_candidates[2];
	struct Intersection *isect_min, *isect_tmp;

	const struct BVHNode *node;
	struct BVHNodeStack stack = {0, {NULL}};

	node = root;
	hit = 0;

	isect_min = &isect_candidates[0];
	isect_tmp = &isect_candidates[1];
	isect_min->t_hit = FLT_MAX;

	for (;;) {
		if (is_bvh_leaf(node)) {
			hittmp = prim_ray_intersect(acc, node->prim_id, time, ray, isect_tmp);
			if (hittmp && isect_tmp->t_hit < isect_min->t_hit) {
				swap_isect_ptr(&isect_min, &isect_tmp);
				hit = hittmp;
			}

			if (is_empty(&stack))
				goto loop_exit;
			node = pop_node(&stack);
			continue;
		}

		hit_left = BoxRayIntersect(&node->left->bounds,
				&ray->orig, &ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax);

		hit_right = BoxRayIntersect(&node->right->bounds,
				&ray->orig, &ray->dir, ray->tmin, ray->tmax,
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
		*isect = *isect_min;
	}

	return hit;
}

static struct BVHNode *build_bvh(struct Primitive **primptrs, int begin, int end, int axis)
{
	struct BVHNode *node = NULL;
	const int NPRIMS = end - begin;
	int median;
	int new_axis;
	int (*primitive_compare)(const void *, const void *);

	node = new_bvhnode();
	if (node == NULL)
		return NULL;

	if (NPRIMS == 1) {
		node->prim_id = primptrs[begin]->index;
		node->bounds = primptrs[begin]->bounds;
		return node;
	}

	switch (axis) {
		case 0:
			primitive_compare = primitive_compare_x;
			new_axis = 1;
			break;
		case 1:
			primitive_compare = primitive_compare_y;
			new_axis = 2;
			break;
		case 2:
			primitive_compare = primitive_compare_z;
			new_axis = 0;
			break;
		default:
			assert(!"invalid axis");
			break;
	}

	qsort(primptrs + begin, NPRIMS, sizeof(struct Primitive *), primitive_compare);
	median = find_median(primptrs, begin, end, axis);

	node->left  = build_bvh(primptrs, begin, median, new_axis);
	if (node->left == NULL)
		return NULL;

	node->right = build_bvh(primptrs, median, end, new_axis);
	if (node->right == NULL)
		return NULL;

	node->bounds = node->left->bounds;
	BoxAddBox(&node->bounds, &node->right->bounds);

	return node;
}

static struct BVHNode *new_bvhnode(void)
{
	struct BVHNode *node = (struct BVHNode *) malloc(sizeof(struct BVHNode));

	if (node == NULL)
		return NULL;

	node->left = NULL;
	node->right = NULL;
	node->prim_id = -1;
	BOX3_SET(&node->bounds, 0, 0, 0, 0, 0, 0);

	return node;
}

static void free_bvhnode_recursive(struct BVHNode *node)
{
	if (node == NULL)
		return;

	if (is_bvh_leaf(node)) {
		free(node);
		return;
	}

	assert(node->left != NULL);
	assert(node->right != NULL);
	assert(node->prim_id == -1);

	free_bvhnode_recursive(node->left);
	free_bvhnode_recursive(node->right);

	free(node);
}

static int is_bvh_leaf(const struct BVHNode *node)
{
	return (
		node->left == NULL &&
		node->right == NULL &&
		node->prim_id != -1);
}

static int find_median(struct Primitive **prims, int begin, int end, int axis)
{
	int low, high, mid;
	double key;

	assert(axis >= 0 && axis <= 2);

	low = begin;
	high = end - 1;
	mid = -1;
	key = (prims[low]->centroid[axis] + prims[high]->centroid[axis]) / 2;

	while (low != mid) {
		mid = (low + high) / 2;
		if (key < prims[mid]->centroid[axis])
			high = mid;
		else if (prims[mid]->centroid[axis] < key)
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

static int primitive_compare_x(const void *a, const void *b)
{
	struct Primitive **A = (struct Primitive **) a;
	struct Primitive **B = (struct Primitive **) b;
	return compare_double((*A)->centroid[0], (*B)->centroid[0]);
}

static int primitive_compare_y(const void *a, const void *b)
{
	struct Primitive **A = (struct Primitive **) a;
	struct Primitive **B = (struct Primitive **) b;
	return compare_double((*A)->centroid[1], (*B)->centroid[1]);
}

static int primitive_compare_z(const void *a, const void *b)
{
	struct Primitive **A = (struct Primitive **) a;
	struct Primitive **B = (struct Primitive **) b;
	return compare_double((*A)->centroid[2], (*B)->centroid[2]);
}

static int prim_ray_intersect(const struct Accelerator *acc, int prim_id, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const int hit = PrmRayIntersect(&acc->primset, prim_id, time, ray, isect);

	if (!hit) {
		isect->t_hit = FLT_MAX;
		return 0;
	}

	if (isect->t_hit < ray->tmin || ray->tmax < isect->t_hit) {
		isect->t_hit = FLT_MAX;
		return 0;
	}

	return 1;
}

static void get_prim_bounds(const struct Accelerator *acc, int prim_id, struct Box *bounds)
{
	PrmGetPrimitiveBounds(&acc->primset, prim_id, bounds);
}

static void swap_isect_ptr(struct Intersection **isect0, struct Intersection **isect1)
{
	struct Intersection *isect_swp = *isect0;
	*isect0 = *isect1;
	*isect1 = isect_swp;
}

static int is_empty(const struct BVHNodeStack *stack)
{
	return (stack->depth == 0);
}

static void push_node(struct BVHNodeStack *stack, const struct BVHNode *node)
{
	stack->node[stack->depth++] = node;
	assert(stack->depth < BVH_STACKSIZE);
}

static const struct BVHNode *pop_node(struct BVHNodeStack *stack)
{
	assert(!is_empty(stack));
	return stack->node[--stack->depth];
}

