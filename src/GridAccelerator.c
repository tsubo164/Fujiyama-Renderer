/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "GridAccelerator.h"
#include "Intersection.h"
#include "PrimitiveSet.h"
#include "Accelerator.h"
#include "Numeric.h"
#include "Memory.h"
#include "Box.h"
#include "Ray.h"

#include <float.h>

static const char ACCELERATOR_NAME[] = "Uniform-Grid";
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

static DerivedAccelerator new_grid_accel(void);
static void free_grid_accel(DerivedAccelerator derived);
static int build_grid_accel(DerivedAccelerator derived,
		const struct PrimitiveSet *primset);
static int intersect_grid_accel(DerivedAccelerator derived,
		const struct PrimitiveSet *primset, double time, const struct Ray *ray,
		struct Intersection *isect);
static void compute_grid_cellsizes(int nprimitives,
		double xwidth, double ywidth, double zwidth,
		int *xncells, int *yncells, int *zncells);
static const char *get_grid_accel_name(void);

/* TODO move this somewhere */
static int prim_ray_intersect(const struct PrimitiveSet *primset, int prim_id,
		double time, const struct Ray *ray, struct Intersection *isect);
static void swap_isect_ptr(struct Intersection **isect0, struct Intersection **isect1);

void GetGridAcceleratorFunction(struct Accelerator *acc)
{
	AccSetDerivedFunctions(acc,
			new_grid_accel,
			free_grid_accel,
			build_grid_accel,
			intersect_grid_accel,
			get_grid_accel_name);
}

static DerivedAccelerator new_grid_accel(void)
{
	struct GridAccelerator *grid = MEM_ALLOC(struct GridAccelerator);

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

	return (DerivedAccelerator) grid;
}

static void free_grid_accel(DerivedAccelerator derived)
{
	int x, y, z;
	struct GridAccelerator *grid = (struct GridAccelerator *) derived;
	const int XNCELLS = grid->ncells[0];
	const int YNCELLS = grid->ncells[1];
	const int ZNCELLS = grid->ncells[2];

	for (z = 0; z < ZNCELLS; z++) {
		for (y = 0; y < YNCELLS; y++) {
			for (x = 0; x < XNCELLS; x++) {
				int cell_id = z * YNCELLS * XNCELLS + y * XNCELLS + x;
				struct Cell *cell = grid->cells[cell_id];

				while (cell != NULL) {
					struct Cell *kill = cell;
					struct Cell *next = cell->next;
					MEM_FREE(kill);
					cell = next;
				}
			}
		}
	}
	MEM_FREE(grid->cells);
	MEM_FREE(grid);
}

static int build_grid_accel(DerivedAccelerator derived,
		const struct PrimitiveSet *primset)
{
	int i;
	int NPRIMS = 0;
	int XNCELLS = 0;
	int YNCELLS = 0;
	int ZNCELLS = 0;
	struct Box bounds = {{0}};
	double cellsize[3] = {0};
	struct Cell **cells = NULL;

	struct GridAccelerator *grid = (struct GridAccelerator *) derived;

	const double PADDING = AccGetBoundsPadding();
	const double HALF_PADDING = .5 * PADDING;

	PrmGetBounds(primset, &bounds);
	BOX3_EXPAND(&bounds, PADDING);

	NPRIMS = PrmGetPrimitiveCount(primset);
	compute_grid_cellsizes(NPRIMS, 
			bounds.max.x - bounds.min.x,
			bounds.max.y - bounds.min.y,
			bounds.max.z - bounds.min.z,
			&XNCELLS, &YNCELLS, &ZNCELLS);

	cells = MEM_ALLOC_ARRAY(struct Cell *, XNCELLS * YNCELLS * ZNCELLS);
	if (cells == NULL)
		return -1;

	for (i = 0; i < XNCELLS * YNCELLS * ZNCELLS; i++)
		cells[i] = NULL;

	cellsize[0] = (bounds.max.x - bounds.min.x) / XNCELLS;
	cellsize[1] = (bounds.max.y - bounds.min.y) / YNCELLS;
	cellsize[2] = (bounds.max.z - bounds.min.z) / ZNCELLS;

	for (i = 0; i < NPRIMS; i++) {
		int X0, X1, Y0, Y1, Z0, Z1;
		int x, y, z;
		int primid = i;
		struct Box primbbox;

		PrmGetPrimitiveBounds(primset, primid, &primbbox);
		BOX3_EXPAND(&primbbox, HALF_PADDING);

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
					int cell_id = z * YNCELLS * XNCELLS + y * XNCELLS + x;
					struct Cell *newcell = MEM_ALLOC(struct Cell);

					if (newcell == NULL) {
						/* TODO error handling */
						return -1;
					}
					newcell->prim_id = primid;
					newcell->next = NULL;

					if (cells[cell_id] == NULL) {
						cells[cell_id] = newcell;
					} else {
						struct Cell *oldcell = cells[cell_id];
						cells[cell_id] = newcell;
						cells[cell_id]->next = oldcell;
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

static int intersect_grid_accel(DerivedAccelerator derived,
		const struct PrimitiveSet *primset, double time, const struct Ray *ray,
		struct Intersection *isect)
{
	const struct GridAccelerator *grid = (const struct GridAccelerator *) derived;
	double grid_min[3];
	double start[3];
	double dir[3];

	double t_start = FLT_MAX;
	double t_end = FLT_MAX;
	double boxhit_tmin;
	double boxhit_tmax;

	int i;
	int hit;
	int NCELLS[3];
	int cell_id[3];
	int cell_step[3];
	int cell_end[3];
	double t_next[3];
	double t_delta[3];

	/* check intersection with overall bounds */
	/* to get boxhit_tmin and boxhit_tmax */
	if (!BoxRayIntersect(&grid->bounds, &ray->orig, &ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax)) {
		return 0;
	}

	/* check if the ray shot from inside bounds */
	if (BoxContainsPoint(&grid->bounds, &ray->orig)) {
		start[0] = ray->orig.x;
		start[1] = ray->orig.y;
		start[2] = ray->orig.z;
		t_start = 0;
	}
	else {
		t_start = boxhit_tmin;
		t_end = boxhit_tmax;
		start[0] = ray->orig.x + ray->dir.x * t_start;
		start[1] = ray->orig.y + ray->dir.y * t_start;
		start[2] = ray->orig.z + ray->dir.z * t_start;
	}
	t_end = MIN(t_end, ray->tmax);

	NCELLS[0] = grid->ncells[0];
	NCELLS[1] = grid->ncells[1];
	NCELLS[2] = grid->ncells[2];

	/* TODO grid_min is static? */
	grid_min[0] = grid->bounds.min.x;
	grid_min[1] = grid->bounds.min.y;
	grid_min[2] = grid->bounds.min.z;
	dir[0] = ray->dir.x;
	dir[1] = ray->dir.y;
	dir[2] = ray->dir.z;

	/* setup 3D DDA */
	for (i = 0; i < 3; i++) {
		cell_id[i] = (int) floor((start[i] - grid_min[i]) / grid->cellsize[i]);
		cell_id[i] = CLAMP(cell_id[i], 0, NCELLS[i]-1);

		if (dir[i] > 0) {
			t_next[i] = t_start +
				(((cell_id[i]+1) * grid->cellsize[i] + grid_min[i]) - start[i]) / dir[i];

			t_delta[i] = grid->cellsize[i] / dir[i];
			cell_step[i] = +1;
			cell_end[i] = NCELLS[i];
		}
		else if (dir[i] <0) {
			t_next[i] = t_start +
				((cell_id[i] * grid->cellsize[i] + grid_min[i]) - start[i]) / dir[i];

			t_delta[i] = -1 * grid->cellsize[i] / dir[i];
			cell_step[i] = -1;
			cell_end[i] = -1;
		}
		else {
			t_next[i] = FLT_MAX;
			t_delta[i] = 0;
			cell_step[i] = 0;
			cell_end[i] = -1;
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

		id = cell_id[2] * NCELLS[0] * NCELLS[1] + cell_id[1] * NCELLS[0] + cell_id[0];

		/* loop over face list that associated in current cell */
		for (cell = grid->cells[id]; cell != NULL; cell = cell->next) {
			int hittmp;
			int inside_cell;
			struct Vector P_hit;
			struct Box cellbox;

			hittmp = prim_ray_intersect(primset, cell->prim_id, time, ray, isect_tmp);
			if (!hittmp)
				continue;

			/* check if the hit point is inside the cell */
			cellbox.min.x = grid->bounds.min.x + cell_id[0] * grid->cellsize[0];
			cellbox.min.y = grid->bounds.min.y + cell_id[1] * grid->cellsize[1];
			cellbox.min.z = grid->bounds.min.z + cell_id[2] * grid->cellsize[2];
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
		if ((t_next[0] < t_next[1]) && (t_next[0] < t_next[2])) {
			if (t_end < t_next[0])
				break;
			cell_id[0] += cell_step[0];
			if (cell_id[0] == cell_end[0])
				break;
			t_next[0] += t_delta[0];
		}
		else if ((t_next[2] < t_next[1])) {
			if (t_end < t_next[2])
				break;
			cell_id[2] += cell_step[2];
			if (cell_id[2] == cell_end[2])
				break;
			t_next[2] += t_delta[2];
		}
		else {
			if (t_end < t_next[1])
				break;
			cell_id[1] += cell_step[1];
			if (cell_id[1] == cell_end[1])
				break;
			t_next[1] += t_delta[1];
		}
	}
	return hit;
}

static void compute_grid_cellsizes(int nprimitives,
		double xwidth, double ywidth, double zwidth,
		int *xncells, int *yncells, int *zncells)
{
	double max_width = 0;
	double ncells_per_unit_dist = 0;
	double cube_root = 0;
	int XNCELLS = 0;
	int YNCELLS = 0;
	int ZNCELLS = 0;

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

static const char *get_grid_accel_name(void)
{
	return ACCELERATOR_NAME;
}

static int prim_ray_intersect(const struct PrimitiveSet *primset, int prim_id,
		double time, const struct Ray *ray, struct Intersection *isect)
{
	const int hit = PrmRayIntersect(primset, prim_id, time, ray, isect);

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

static void swap_isect_ptr(struct Intersection **isect0, struct Intersection **isect1)
{
	struct Intersection *isect_swp = *isect0;
	*isect0 = *isect1;
	*isect1 = isect_swp;
}

