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
	int has_built;
	double bounds[6];
	const char *name;

	/* TODO should make struct PrimitiveSet? */
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
/* GridAccelerator */
enum { GRID_MAXCELLS = 512 };

struct Cell {
	int faceid;
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
struct Primitive {
	double bounds[6];
	double centroid[3];
	int index;
};

struct BVHNode {
	struct BVHNode *left;
	struct BVHNode *right;
	double bounds[6];
	int index;
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
static struct BVHNode *new_bvhnode(void);
static void free_bvhnode_recursive(struct BVHNode *node);
static struct BVHNode *build_bvh(struct Primitive **prims, int begin, int end, int axis);
static int is_bvh_leaf(const struct BVHNode *node);
static int find_median(struct Primitive **prims, int begin, int end, int axis);
static int compare_double(double x, double y);
static int primitive_compare_x(const void *a, const void *b);
static int primitive_compare_y(const void *a, const void *b);
static int primitive_compare_z(const void *a, const void *b);

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
	double stepdir[3];
	double tnext[3];
	double tdelt[3];

	/* check intersection with overall bounds */
	if (!BoxRayIntersect(acc->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

#if 0
	boxhit_tmin = ray->tmin;
	boxhit_tmax = ray->tmax;
	if (!acc->has_built) {
		/* dynamic build */
		printf("\nbuilding grid accelerator...\n");
		AccBuild((struct Accelerator *) acc);
		fflush(stdout);
	}
#endif

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

	VEC3_COPY(stepdir, ray->dir);
	VEC3_COPY(NCELLS, grid->ncells);

	/* setup 3D DDA */
	for (i = 0; i < 3; i++) {
		cellid[i] = (int) floor((start[i] - grid->bounds[i]) / grid->cellsize[i]);
		cellid[i] = CLAMP(cellid[i], 0, grid->ncells[i]-1);

		if (stepdir[i] > 0) {
			tnext[i] = tstart +
				(((cellid[i]+1) * grid->cellsize[i] + grid->bounds[i]) - start[i]) / stepdir[i];

			tdelt[i] = grid->cellsize[i] / stepdir[i];
			cellstep[i] = +1;
			cellend[i] = NCELLS[i];
		}
		else if (stepdir[i] <0) {
			tnext[i] = tstart +
				((cellid[i] * grid->cellsize[i] + grid->bounds[i]) - start[i]) / stepdir[i];

			tdelt[i] = -1 * grid->cellsize[i] / stepdir[i];
			cellstep[i] = -1;
			cellend[i] = -1;
		}
		else {
			tnext[i] = FLT_MAX;
			tdelt[i] = 0;
			cellstep[i] = -1;
			cellend[i] = -1;
		}
	}

	/* traversal voxels */
	hit = 0;
	for(;;) {
		int id;
		int xid, yid, zid;
		double tmin = FLT_MAX;
		int faceid;
		struct Cell *c;
		struct LocalGeometry loctmp;

		VEC3_GET(xid, yid, zid, cellid);
		id = zid * NCELLS[0] * NCELLS[1] + yid * NCELLS[0] + xid;

		/* loop over face list that associated in current cell */
		for (c = grid->cells[id]; c != NULL; c = c->next) {
			int hittmp;
			double ttmp = FLT_MAX;
			double cellbox[6];
			double hitpt[3];
			int inside_cell;
			faceid = c->faceid;

			hittmp = acc->PrimIntersect(acc->primset, faceid, ray, &loctmp, &ttmp);
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
			if (ttmp < tmin && inside_cell &&
			    ttmp > ray->tmin && ttmp < ray->tmax) {
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
					newcell->faceid = primid;
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
#if 0
	{
		int x, y, z;
		printf("bounds: (%g, %g, %g), (%g, %g, %g)\n",
				bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
		for (z = 0; z < ZNCELLS; z++) {
			for (y = 0; y < YNCELLS; y++) {
				for (x = 0; x < XNCELLS; x++) {
					int cellid = z * YNCELLS * XNCELLS + y * XNCELLS + x;
					struct Cell *c;
					if (cells[cellid] == NULL)
						continue;

					printf("cellid %d: ", cellid);
					printf("%d %d %d => ", x, y, z);
					for (c = cells[cellid]; c != NULL; c = c->next) {
						printf("%d ", c->faceid);
					}
					printf("\n");
				}
			}
		}
	}
#endif
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
				struct Cell *c = grid->cells[cellid];

				while (c != NULL) {
					struct Cell *kill = c;
					struct Cell *next = c->next;
					free(kill);
					c = next;
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
	default:
		assert(!"invalid accelerator type");
		break;
	}

	acc->has_built = 0;
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
	/*
	struct Ray newray;
	*/

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
#if 0
	newray = *ray;
	newray.tmin = boxhit_tmin;
	newray.tmax = boxhit_tmax;

	return acc->Intersect(acc, &newray, isect, t_hit);
#endif
	return acc->Intersect(acc, ray, isect, t_hit);
}

void AccSetTargetGeometry(struct Accelerator *acc,
	const void *primset, int nprims, const double *primset_bounds,
	PrimIntersectFunction prim_intersect_function,
	PrimBoundsFunction prim_bounds_function)
{
	acc->primset = primset;
	acc->nprims = nprims;
	acc->PrimIntersect = prim_intersect_function;
	acc->PrimBounds = prim_bounds_function;
	BOX3_COPY(acc->primset_bounds, primset_bounds);

	/* accelerator's bounds */
	BOX3_COPY(acc->bounds, primset_bounds);
	BOX3_EXPAND(acc->bounds, EXPAND);
}

int AccGetPrimitiveCount(const struct Accelerator *acc)
{
	return acc->nprims;
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

	NPRIMS = AccGetPrimitiveCount(acc);

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
		/*
		BOX3_EXPAND(prims[i].bounds, HALF_EXPAND);
		*/
		prims[i].centroid[0] = (prims[i].bounds[3] + prims[i].bounds[0]) / 2;
		prims[i].centroid[1] = (prims[i].bounds[4] + prims[i].bounds[1]) / 2;
		prims[i].centroid[2] = (prims[i].bounds[5] + prims[i].bounds[2]) / 2;
		prims[i].index = i;
		/*
		PrintBox3d(prims[i].bounds);
		printf("%g, %g, %g\n", prims[i].centroid[0], prims[i].centroid[1], prims[i].centroid[2]);
		printf("%d: %g\n", i, prims[i].centroid[0]);
		*/

		primptrs[i] = &prims[i];
	}
	/*
	puts("===============================");

	qsort(primptrs, NPRIMS, sizeof(struct Primitive *), primitive_compare_x);
	for (i = 0; i < NPRIMS; i++) {
		printf("%d: %d: %g\n", i, primptrs[i]->index, primptrs[i]->centroid[0]);
	}
	{
		int median = find_median(primptrs, 0, NPRIMS);
		printf("median: %d: %d: %g\n", median, primptrs[median]->index, primptrs[median]->centroid[0]);
	}
	*/

	bvh->root = build_bvh(primptrs, 0, NPRIMS, 0);
	if (bvh->root == NULL) {
		free(primptrs);
		free(prims);
		return -1;
	}
	/*
	*/

	free(primptrs);
	free(prims);
	return 0;
}

static int IntersectBVH(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct BVHAccelerator *bvh = (const struct BVHAccelerator *) acc->derived;

	return intersect_bvh_recursive(acc, bvh->root, ray, isect,  t_hit);
#if 0
	double tmin_left, tmin_right;
	struct LocalGeometry local_left, local_right;
	double boxhit_tmin;
	double boxhit_tmax;

	/* check intersection with overall bounds */
	if (!BoxRayIntersect(acc->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	if (!acc->has_built) {
		/* dynamic build */
		printf("\nbuilding BVH accelerator...\n");
		AccBuild((struct Accelerator *) acc);
		fflush(stdout);
	}

	return 0;
#endif
}

static int intersect_bvh_recursive(const struct Accelerator *acc, const struct BVHNode *node,
		const struct Ray *ray, struct LocalGeometry *isect, double *t_hit)
{
	/*
	const struct BVHAccelerator *bvh = (const struct BVHAccelerator *) acc->derived;
	*/
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
		int hitprim;
		hitprim = acc->PrimIntersect(acc->primset, node->index, ray, isect, t_hit);
			/*
		if (hitprim) {
			printf("hit index: %d: t_hit: %g: node offset: %ul\n", node->index, *t_hit,
				node - bvh->root);
			PrintBox3d(node->bounds);
		}
			*/

		if (hitprim)
			return 1;
		else
			return 0;
	}

	t_hit_left = FLT_MAX;
	hit_left  = intersect_bvh_recursive(acc, node->left,  ray, &local_left,  &t_hit_left);
	t_hit_right = FLT_MAX;
	hit_right = intersect_bvh_recursive(acc, node->right, ray, &local_right, &t_hit_right);

	if (t_hit_left < t_hit_right) {
		*t_hit = t_hit_left;
		*isect = local_left;
	} else if (t_hit_right < t_hit_left) {
		*t_hit = t_hit_right;
		*isect = local_right;
	}
#if 0
#endif
	/*
	if (hit_left || hit_right) {
		if (*t_hit > 1000) {
			printf("t_hit: %g\n", *t_hit);
			printf("\tt_hit_left:  %g\n", t_hit_left);
			printf("\tt_hit_right: %g\n", t_hit_right);
			printf("\thit_left:  %d\n", hit_left);
			printf("\thit_right: %d\n", hit_right);
		}
	}
	*/

	return (hit_left || hit_right);
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
		node->index = primptrs[begin]->index;
		BOX3_COPY(node->bounds, primptrs[begin]->bounds);
		/*
		printf("*** leaf: %d\n", node->index);
		*/
		/*
		printf("*** leaf: ");
		PrintBox3d(node->bounds);
		*/
		/*
		if (node->index == 7941) {
			static int n = 0;
			n++;
			printf("*** leaf: %d was built %d times\n", node->index, n);
			printf("\t begin: %d end %d\n", begin, end);
		}
		*/
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

	/* XXX QSORT */
	qsort(primptrs + begin, NPRIMS, sizeof(struct Primitive *), primitive_compare);
	median = find_median(primptrs, begin, end, axis);

	/*
	new_axis = (axis + 1) % 3;
	*/

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
	node->index = -1;
	BOX3_SET(node->bounds, 0, 0, 0, 0, 0, 0);

	return node;
}

static void free_bvhnode_recursive(struct BVHNode *node)
{
	if (node == NULL)
		return;

	if (is_bvh_leaf(node)) {
		/*
		printf("freeing index: %d\n", node->index);
		*/
		free(node);
		return;
	}

	assert(node->left != NULL);
	assert(node->right != NULL);
	assert(node->index == -1);

	free_bvhnode_recursive(node->left);
	free_bvhnode_recursive(node->right);

	free(node);
}

static int is_bvh_leaf(const struct BVHNode *node)
{
	return (
		node->left == NULL &&
		node->right == NULL &&
		node->index != -1);
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

