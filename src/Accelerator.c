/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Accelerator.h"
#include "LocalGeometry.h"
#include "Triangle.h"
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

struct Accelerator {
	const char *name;
	double bounds[6];
	int has_built;

	/* TODO should make struct PrimitiveSet? */
	int primtype;
	const void *primset;
	int nprims;
	double primset_bounds[6];
	PrimIntersectFunction PrimIntersect;
	PrimBoundsFunction PrimBounds;

	/* private */
	char *derived;
	void (*FreeDerived)(struct Accelerator *acc);
	int (*Build)(struct Accelerator *acc);
	int (*Intersect)(const struct Accelerator *acc, const struct Ray *ray,
			struct LocalGeometry *isect, double *t_hit);
};

/* -------------------------------------------------------------------------- */
/* Accelerator */
static int ray_prim_intersect (const struct Accelerator *acc, int prim_id,
	const struct Ray *ray, struct LocalGeometry *isect, double *t_hit);

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
	double bounds[6];
};

static struct GridAccelerator *NewGrid(void);
static void FreeGrid(struct Accelerator *acc);
static int BuildGrid(struct Accelerator *acc);
static int IntersectGrid(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
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
	double bounds[6];
	double centroid[3];
	int index;
};

struct BVHNode {
	struct BVHNode *left;
	struct BVHNode *right;
	double bounds[6];
	int prim_id;
};

struct BVHNodeStack {
	int depth;
	const struct BVHNode *node[BVH_STACKSIZE];
};

struct BVHAccelerator {
	struct BVHNode *root;
};

static struct BVHAccelerator *NewBVH(void);
static void FreeBVH(struct Accelerator *acc);
static int BuildBVH(struct Accelerator *acc);
static int IntersectBVH(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static int intersect_bvh_recursive(const struct Accelerator *acc, const struct BVHNode *node,
		const struct Ray *ray, struct LocalGeometry *isect, double *t_hit);
static int intersect_bvh_loop(const struct Accelerator *acc, const struct BVHNode *root,
		const struct Ray *ray, struct LocalGeometry *isect, double *t_hit);

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
/* VolumeAccelerator */
struct VolumeAccelerator {
	double bounds[6];
};

static struct VolumeAccelerator *new_volume_accelerator(void);
static void free_volume_accelerator(struct Accelerator *acc);
static int build_volume_accelerator(struct Accelerator *acc);
static int intersect_volume_accelerator(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);

/* -------------------------------------------------------------------------- */
static int IntersectGrid(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct GridAccelerator *grid = (const struct GridAccelerator *) acc->derived;
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

	/* check intersection with overall bounds */
	/* to get boxhit_tmin and boxhit_tmax */
	if (!BoxRayIntersect(acc->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	/* check if the ray shot from inside bounds */
	if (BoxContainsPoint(grid->bounds, ray->orig)) {
		VEC3_COPY(start, ray->orig);
		tstart = 0;
	}
	else {
		tstart = boxhit_tmin;
		tend = boxhit_tmax;
		POINT_ON_RAY(start, ray->orig, ray->dir, tstart);
	}

	VEC3_COPY(NCELLS, grid->ncells);

	/* setup 3D DDA */
	for (i = 0; i < 3; i++) {
		cellid[i] = (int) floor((start[i] - grid->bounds[i]) / grid->cellsize[i]);
		cellid[i] = CLAMP(cellid[i], 0, NCELLS[i]-1);

		if (ray->dir[i] > 0) {
			tnext[i] = tstart +
				(((cellid[i]+1) * grid->cellsize[i] + grid->bounds[i]) - start[i]) / ray->dir[i];

			tdelt[i] = grid->cellsize[i] / ray->dir[i];
			cellstep[i] = +1;
			cellend[i] = NCELLS[i];
		}
		else if (ray->dir[i] <0) {
			tnext[i] = tstart +
				((cellid[i] * grid->cellsize[i] + grid->bounds[i]) - start[i]) / ray->dir[i];

			tdelt[i] = -1 * grid->cellsize[i] / ray->dir[i];
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
		double tmin = FLT_MAX;
		struct Cell *cell;
		struct LocalGeometry loctmp;

		id = cellid[2] * NCELLS[0] * NCELLS[1] + cellid[1] * NCELLS[0] + cellid[0];

		/* loop over face list that associated in current cell */
		for (cell = grid->cells[id]; cell != NULL; cell = cell->next) {
			int hittmp;
			int inside_cell;
			double ttmp = FLT_MAX;
			double hitpt[3];
			double cellbox[6];

			hittmp = ray_prim_intersect(acc, cell->prim_id, ray, &loctmp, &ttmp);
			if (!hittmp)
				continue;

			/* check if the hit point is inside the cell */
			cellbox[0] = grid->bounds[0] + cellid[0] * grid->cellsize[0];
			cellbox[1] = grid->bounds[1] + cellid[1] * grid->cellsize[1];
			cellbox[2] = grid->bounds[2] + cellid[2] * grid->cellsize[2];
			cellbox[3] = cellbox[0] + grid->cellsize[0];
			cellbox[4] = cellbox[1] + grid->cellsize[1];
			cellbox[5] = cellbox[2] + grid->cellsize[2];
			POINT_ON_RAY(hitpt, ray->orig, ray->dir, ttmp);
			inside_cell = BoxContainsPoint(cellbox, hitpt);

			/* update info ONLY if tmin renewed */
			if (ttmp < tmin && inside_cell) {
				tmin = ttmp;
				hit = hittmp;
				*isect = loctmp;
			}
		}
		if (hit) {
			*t_hit = tmin;
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

static int BuildGrid(struct Accelerator *acc)
{
	int i;
	int XNCELLS;
	int YNCELLS;
	int ZNCELLS;
	double bounds[6] = {0};
	double cellsize[3] = {0};
	struct Cell **cells = NULL;

	struct GridAccelerator *grid = (struct GridAccelerator *) acc->derived;

	compute_grid_cellsizes(acc->nprims, 
			acc->bounds[3] - acc->bounds[0],
			acc->bounds[4] - acc->bounds[1],
			acc->bounds[5] - acc->bounds[2],
			&XNCELLS, &YNCELLS, &ZNCELLS);

	cells = (struct Cell **) malloc(sizeof(struct Cell *) * XNCELLS * YNCELLS * ZNCELLS);
	if (cells == NULL)
		return -1;

	for (i = 0; i < XNCELLS * YNCELLS * ZNCELLS; i++)
		cells[i] = NULL;

	BOX3_COPY(bounds, acc->bounds);
	cellsize[0] = (bounds[3] - bounds[0]) / XNCELLS;
	cellsize[1] = (bounds[4] - bounds[1]) / YNCELLS;
	cellsize[2] = (bounds[5] - bounds[2]) / ZNCELLS;

	for (i = 0; i < acc->nprims; i++) {
		int X0, X1, Y0, Y1, Z0, Z1;
		int x, y, z;
		int primid = i;
		double primbbox[6];

		acc->PrimBounds(acc->primset, primid, primbbox);
		BOX3_EXPAND(primbbox, HALF_EXPAND);

		/* compute the ranges of cell indices. e.g. [X0 .. X1) */
		X0 = (int) floor((primbbox[0] - bounds[0]) / cellsize[0]);
		X1 = (int) floor((primbbox[3] - bounds[0]) / cellsize[0]) + 1;
		Y0 = (int) floor((primbbox[1] - bounds[1]) / cellsize[1]);
		Y1 = (int) floor((primbbox[4] - bounds[1]) / cellsize[1]) + 1;
		Z0 = (int) floor((primbbox[2] - bounds[2]) / cellsize[2]);
		Z1 = (int) floor((primbbox[5] - bounds[2]) / cellsize[2]) + 1;
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
	VEC3_SET(grid->ncells, XNCELLS, YNCELLS, ZNCELLS);
	VEC3_COPY(grid->cellsize, cellsize);
	BOX3_COPY(grid->bounds, bounds);

	return 0;
}

static struct GridAccelerator *NewGrid(void)
{
	struct GridAccelerator *grid;

	grid = (struct GridAccelerator *) malloc(sizeof(struct GridAccelerator));
	if (grid == NULL)
		return NULL;

	grid->cells = NULL;
	VEC3_SET(grid->ncells, 0, 0, 0);
	VEC3_SET(grid->cellsize, 0, 0, 0);
	BOX3_SET(grid->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	return grid;
}

static void FreeGrid(struct Accelerator *acc)
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
	free(grid);
}

struct Accelerator *AccNew(int accelerator_type)
{
	struct Accelerator *acc = (struct Accelerator *) malloc(sizeof(struct Accelerator));
	if (acc == NULL)
		return NULL;

	switch (accelerator_type) {
	case ACC_GRID:
		acc->derived = (char *) NewGrid();
		if (acc->derived == NULL) {
			AccFree(acc);
			return NULL;
		}
		acc->FreeDerived = FreeGrid;
		acc->Build = BuildGrid;
		acc->Intersect = IntersectGrid;
		acc->name = "Uniform-Grid";
		break;
	case ACC_BVH:
		acc->derived = (char *) NewBVH();
		if (acc->derived == NULL) {
			AccFree(acc);
			return NULL;
		}
		acc->FreeDerived = FreeBVH;
		acc->Build = BuildBVH;
		acc->Intersect = IntersectBVH;
		acc->name = "BVH";
		break;
	case ACC_VOLUME:
		acc->derived = (char *) new_volume_accelerator();
		if (acc->derived == NULL) {
			AccFree(acc);
			return NULL;
		}
		acc->FreeDerived = free_volume_accelerator;
		acc->Build = build_volume_accelerator;
		acc->Intersect = intersect_volume_accelerator;
		acc->name = "Volume";
		break;
	default:
		assert(!"invalid accelerator type");
		break;
	}

	acc->has_built = 0;

	acc->primtype = ACC_PRIM_SURFACE;
	acc->primset = NULL;
	acc->nprims = 0;
	acc->PrimIntersect = NULL;
	acc->PrimBounds = NULL;

	BOX3_SET(acc->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	BOX3_SET(acc->primset_bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

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

int AccGetPrimitiveType(const struct Accelerator *acc)
{
	return acc->primtype;
}

void AccGetBounds(const struct Accelerator *acc, double *bounds)
{
	BOX3_COPY(bounds, acc->bounds);
}

int AccBuild(struct Accelerator *acc)
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

int AccIntersect(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
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
		AccBuild((struct Accelerator *) acc);
		fflush(stdout);
	}

	return acc->Intersect(acc, ray, isect, t_hit);
}

void AccSetTargetGeometry(struct Accelerator *acc,
	int primtype,
	const void *primset, int nprims, const double *primset_bounds,
	PrimIntersectFunction prim_intersect_function,
	PrimBoundsFunction prim_bounds_function)
{
	assert(ACC_PRIM_SURFACE <= primtype && primtype <= ACC_PRIM_VOLUME );

	acc->primtype = primtype;
	acc->primset = primset;
	acc->nprims = nprims;
	acc->PrimIntersect = prim_intersect_function;
	acc->PrimBounds = prim_bounds_function;
	BOX3_COPY(acc->primset_bounds, primset_bounds);

	/* accelerator's bounds */
	BOX3_COPY(acc->bounds, primset_bounds);
	BOX3_EXPAND(acc->bounds, EXPAND);
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
static struct BVHAccelerator *NewBVH(void)
{
	struct BVHAccelerator *bvh;

	bvh = (struct BVHAccelerator *) malloc(sizeof(struct BVHAccelerator));
	if (bvh == NULL)
		return NULL;

	bvh->root = NULL;

	return bvh;
}

static void FreeBVH(struct Accelerator *acc)
{
	struct BVHAccelerator *bvh = (struct BVHAccelerator *) acc->derived;

	free_bvhnode_recursive(bvh->root);

	free(bvh);
}

static int BuildBVH(struct Accelerator *acc)
{
	struct BVHAccelerator *bvh = (struct BVHAccelerator *) acc->derived;
	struct Primitive *prims;
	struct Primitive **primptrs;
	int NPRIMS;
	int i;

	NPRIMS = acc->nprims;

	prims = (struct Primitive *) malloc(sizeof(struct Primitive) * NPRIMS);
	if (prims == NULL)
		return -1;

	primptrs = (struct Primitive **) malloc(sizeof(struct Primitive *) * NPRIMS);
	if (primptrs == NULL) {
		free(prims);
		return -1;
	}

	for (i = 0; i < NPRIMS; i++) {
		acc->PrimBounds(acc->primset, i, prims[i].bounds);
		prims[i].centroid[0] = (prims[i].bounds[3] + prims[i].bounds[0]) / 2;
		prims[i].centroid[1] = (prims[i].bounds[4] + prims[i].bounds[1]) / 2;
		prims[i].centroid[2] = (prims[i].bounds[5] + prims[i].bounds[2]) / 2;
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

static int IntersectBVH(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct BVHAccelerator *bvh = (const struct BVHAccelerator *) acc->derived;

	if (1)
		return intersect_bvh_loop(acc, bvh->root, ray, isect,  t_hit);
	else
		return intersect_bvh_recursive(acc, bvh->root, ray, isect,  t_hit);
}

static int intersect_bvh_recursive(const struct Accelerator *acc, const struct BVHNode *node,
		const struct Ray *ray, struct LocalGeometry *isect, double *t_hit)
{
	struct LocalGeometry local_left, local_right;
	double boxhit_tmin;
	double boxhit_tmax;
	double t_hit_left, t_hit_right;
	int hit_left, hit_right;
	int hit;

	hit = BoxRayIntersect(node->bounds,
			ray->orig, ray->dir, ray->tmin, ray->tmax,
			&boxhit_tmin, &boxhit_tmax);
	if (!hit) {
		return 0;
	}

	if (is_bvh_leaf(node)) {
		return ray_prim_intersect(acc, node->prim_id, ray, isect, t_hit);
	}

	t_hit_left = FLT_MAX;
	hit_left  = intersect_bvh_recursive(acc, node->left,  ray, &local_left,  &t_hit_left);
	t_hit_right = FLT_MAX;
	hit_right = intersect_bvh_recursive(acc, node->right, ray, &local_right, &t_hit_right);

	if (t_hit_left < ray->tmin)
		t_hit_left = FLT_MAX;
	if (t_hit_right < ray->tmin)
		t_hit_right = FLT_MAX;

	if (t_hit_left < t_hit_right) {
		*t_hit = t_hit_left;
		*isect = local_left;
	} else if (t_hit_right < t_hit_left) {
		*t_hit = t_hit_right;
		*isect = local_right;
	}

	return (hit_left || hit_right);
}

static int intersect_bvh_loop(const struct Accelerator *acc, const struct BVHNode *root,
		const struct Ray *ray, struct LocalGeometry *isect, double *t_hit)
{
	int hit, hittmp;
	int whichhit;
	int hit_left, hit_right;
	double boxhit_tmin, boxhit_tmax;
	double ttmp, tmin;
	struct LocalGeometry localgeo[2];
	struct LocalGeometry *localmin, *localtmp;

	const struct BVHNode *node;
	struct BVHNodeStack stack = {0, {NULL}};

	node = root;
	hit = 0;
	tmin = FLT_MAX;

	localmin = &localgeo[0];
	localtmp = &localgeo[1];

	for (;;) {
		if (is_bvh_leaf(node)) {
			hittmp = ray_prim_intersect(acc, node->prim_id, ray, localtmp, &ttmp);
			if (hittmp && ttmp < tmin) {
				/* swap local geometry */
				struct LocalGeometry *localswp = localtmp;
				localtmp = localmin;
				localmin = localswp;
				/* update tmin */
				tmin = ttmp;
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
		*t_hit = tmin;
		*isect = *localmin;
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
		BOX3_COPY(node->bounds, primptrs[begin]->bounds);
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

	BOX3_COPY(node->bounds, node->left->bounds);
	BoxAddBox(node->bounds, node->right->bounds);

	return node;
}

static struct BVHNode *new_bvhnode(void)
{
	struct BVHNode *node;

	node = (struct BVHNode *) malloc(sizeof(struct BVHNode));
	if (node == NULL)
		return NULL;

	node->left = NULL;
	node->right = NULL;
	node->prim_id = -1;
	BOX3_SET(node->bounds, 0, 0, 0, 0, 0, 0);

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

static int ray_prim_intersect (const struct Accelerator *acc, int prim_id,
	const struct Ray *ray, struct LocalGeometry *isect, double *t_hit)
{
	int hit;

	hit = acc->PrimIntersect(acc->primset, prim_id, ray, isect, t_hit);
	if (!hit)
		return 0;

	if (*t_hit < ray->tmin || ray->tmax < *t_hit)
		return 0;

	return 1;
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

/* -------------------------------------------------------------------------- */
static struct VolumeAccelerator *new_volume_accelerator(void)
{
	struct VolumeAccelerator *vol;

	vol = (struct VolumeAccelerator *) malloc(sizeof(struct VolumeAccelerator));
	if (vol == NULL)
		return NULL;

	return vol;
}

static void free_volume_accelerator(struct Accelerator *acc)
{
	struct VolumeAccelerator *vol = (struct VolumeAccelerator *) acc->derived;
	free(vol);
}

static int build_volume_accelerator(struct Accelerator *acc)
{
	return 0;
}

static int intersect_volume_accelerator(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	return acc->PrimIntersect(acc->primset, 0, ray, isect, t_hit);
}

/* XXX TEST */
struct Volume *AccGetVolume(const struct Accelerator *acc, int index)
{
	assert(acc->primtype == ACC_PRIM_VOLUME);
	return (struct Volume *) acc->primset;
}

