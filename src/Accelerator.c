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

	if (!acc->has_built) {
		/* dynamic build */
		printf("\nbuilding grid accelerator...\n");
		AccBuild((struct Accelerator *) acc);
		fflush(stdout);
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

	if (acc->has_built)
		return -1;

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

	acc->has_built = 1;
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
	return acc->Build(acc);
}

int AccIntersect(const struct Accelerator *acc, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
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

