/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_grid_accelerator.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_numeric.h"
#include "fj_types.h"
#include "fj_ray.h"

#include <utility>
#include <cstddef>

namespace fj {

static const char ACCELERATOR_NAME[] = "Uniform-Grid";
static const int GRID_MAXCELLS = 512;

class Cell {
public:
  Cell() : prim_id(0), next(NULL) {}
  ~Cell() {}

  int prim_id;
  Cell *next;
};

static Cell *new_cell() { return new Cell(); }
static void free_cell(Cell *cell) { delete cell; }

static void compute_grid_cellsizes(int nprimitives,
    Real xwidth, Real ywidth, Real zwidth,
    int *xncells, int *yncells, int *zncells);

/* TODO move this somewhere */
static bool prim_ray_intersect(const PrimitiveSet *primset, int prim_id,
    Real time, const Ray &ray, Intersection *isect);

GridAccelerator::GridAccelerator() : cells_(), cellsize_(), bounds_()
{
  cellsize_[0] = 0;
  cellsize_[1] = 0;
  cellsize_[2] = 0;
}

GridAccelerator::~GridAccelerator()
{
  const int XNCELLS = ncells_[0];
  const int YNCELLS = ncells_[1];
  const int ZNCELLS = ncells_[2];

  for (int z = 0; z < ZNCELLS; z++) {
    for (int y = 0; y < YNCELLS; y++) {
      for (int x = 0; x < XNCELLS; x++) {
        const int cell_id = z * YNCELLS * XNCELLS + y * XNCELLS + x;
        Cell *cell = cells_[cell_id];

        while (cell != NULL) {
          Cell *kill = cell;
          Cell *next = cell->next;
          free_cell(kill);
          cell = next;
        }
      }
    }
  }
}

int GridAccelerator::build()
{
  int NPRIMS = 0;
  int XNCELLS = 0;
  int YNCELLS = 0;
  int ZNCELLS = 0;
  Box bounds_tmp;
  Vector cellsize_tmp;
  std::vector<Cell*> cells_tmp;

  const PrimitiveSet *primset = GetPrimitiveSet();

  const Real PADDING = GetBoundsPadding();
  const Real HALF_PADDING = .5 * PADDING;

  primset->GetBounds(&bounds_tmp);
  BoxExpand(&bounds_tmp, PADDING);

  NPRIMS = primset->GetPrimitiveCount();
  compute_grid_cellsizes(NPRIMS, 
      bounds_tmp.max.x - bounds_tmp.min.x,
      bounds_tmp.max.y - bounds_tmp.min.y,
      bounds_tmp.max.z - bounds_tmp.min.z,
      &XNCELLS, &YNCELLS, &ZNCELLS);

  cells_tmp.resize(XNCELLS * YNCELLS * ZNCELLS, NULL);

  cellsize_tmp.x = (bounds_tmp.max.x - bounds_tmp.min.x) / XNCELLS;
  cellsize_tmp.y = (bounds_tmp.max.y - bounds_tmp.min.y) / YNCELLS;
  cellsize_tmp.z = (bounds_tmp.max.z - bounds_tmp.min.z) / ZNCELLS;

  for (int i = 0; i < NPRIMS; i++) {
    const int primid = i;
    Box primbbox;

    primset->GetPrimitiveBounds(primid, &primbbox);
    BoxExpand(&primbbox, HALF_PADDING);

    /* compute the ranges of cell indices. e.g. [X0 .. X1) */
    int X0 = (int) floor((primbbox.min.x - bounds_tmp.min.x) / cellsize_tmp.x);
    int X1 = (int) floor((primbbox.max.x - bounds_tmp.min.x) / cellsize_tmp.x) + 1;
    int Y0 = (int) floor((primbbox.min.y - bounds_tmp.min.y) / cellsize_tmp.y);
    int Y1 = (int) floor((primbbox.max.y - bounds_tmp.min.y) / cellsize_tmp.y) + 1;
    int Z0 = (int) floor((primbbox.min.z - bounds_tmp.min.z) / cellsize_tmp.z);
    int Z1 = (int) floor((primbbox.max.z - bounds_tmp.min.z) / cellsize_tmp.z) + 1;
    X0 = Clamp(X0, 0, XNCELLS);
    X1 = Clamp(X1, 0, XNCELLS);
    Y0 = Clamp(Y0, 0, YNCELLS);
    Y1 = Clamp(Y1, 0, YNCELLS);
    Z0 = Clamp(Z0, 0, ZNCELLS);
    Z1 = Clamp(Z1, 0, ZNCELLS);

    /* add cell list which holds face id inside the cell */
    for (int z = Z0; z < Z1; z++) {
      for (int y = Y0; y < Y1; y++) {
        for (int x = X0; x < X1; x++) {
          const int cell_id = z * YNCELLS * XNCELLS + y * XNCELLS + x;
          Cell *newcell = new_cell();

          if (newcell == NULL) {
            /* TODO error handling */
            return -1;
          }
          newcell->prim_id = primid;
          newcell->next = NULL;

          if (cells_tmp[cell_id] == NULL) {
            cells_tmp[cell_id] = newcell;
          } else {
            Cell *oldcell = cells_tmp[cell_id];
            cells_tmp[cell_id] = newcell;
            cells_tmp[cell_id]->next = oldcell;
          }
        }
      }
    }
  }

  // commit
  cells_.swap(cells_tmp);
  ncells_[0] = XNCELLS;
  ncells_[1] = YNCELLS;
  ncells_[2] = ZNCELLS;
  cellsize_ = cellsize_tmp;
  bounds_ = bounds_tmp;

  return 0;
}

bool GridAccelerator::intersect(const Ray &ray, Real time, Intersection *isect) const
{
  int NCELLS[3];
  int cell_id[3];
  int cell_step[3];
  int cell_end[3];
  const PrimitiveSet *primset = GetPrimitiveSet();

  // check intersection with overall bounds
  // to get boxhit_tmin and boxhit_tmax
  Real boxhit_tmin;
  Real boxhit_tmax;
  if (!BoxRayIntersect(bounds_, ray.orig, ray.dir, ray.tmin, ray.tmax,
        &boxhit_tmin, &boxhit_tmax)) {
    return 0;
  }

  // check if the ray shot from inside bounds
  Vector start;
  Real t_start = REAL_MAX;
  Real t_end = REAL_MAX;

  if (BoxContainsPoint(bounds_, ray.orig)) {
    start = ray.orig;
    t_start = 0;
  }
  else {
    t_start = boxhit_tmin;
    t_end = boxhit_tmax;
    start = RayPointAt(ray, t_start);
  }
  t_end = Min(t_end, ray.tmax);

  NCELLS[0] = ncells_[0];
  NCELLS[1] = ncells_[1];
  NCELLS[2] = ncells_[2];

  // TODO grid_min should be a member?
  const Vector &grid_min = bounds_.min;
  const Vector &dir = ray.dir;
  Real t_next[3];
  Real t_delta[3];

  // setup 3D DDA
  for (int i = 0; i < 3; i++) {
    cell_id[i] = (int) floor((start[i] - grid_min[i]) / cellsize_[i]);
    cell_id[i] = Clamp(cell_id[i], 0, NCELLS[i]-1);

    if (dir[i] > 0) {
      t_next[i] = t_start +
        (((cell_id[i]+1) * cellsize_[i] + grid_min[i]) - start[i]) / dir[i];

      t_delta[i] = cellsize_[i] / dir[i];
      cell_step[i] = +1;
      cell_end[i] = NCELLS[i];
    }
    else if (dir[i] <0) {
      t_next[i] = t_start +
        ((cell_id[i] * cellsize_[i] + grid_min[i]) - start[i]) / dir[i];

      t_delta[i] = -1 * cellsize_[i] / dir[i];
      cell_step[i] = -1;
      cell_end[i] = -1;
    }
    else {
      t_next[i] = REAL_MAX;
      t_delta[i] = 0;
      cell_step[i] = 0;
      cell_end[i] = -1;
    }
  }

  // traverse voxels
  bool hit = false;
  for (;;) {
    Intersection isect_candidates[2];
    Intersection *isect_min, *isect_tmp;

    isect_min = &isect_candidates[0];
    isect_tmp = &isect_candidates[1];
    isect_min->t_hit = REAL_MAX;

    const int id = NCELLS[0] * NCELLS[1] * cell_id[2] + NCELLS[0] * cell_id[1] + cell_id[0];

    // loop over face list that associated in current cell
    for (Cell *cell = cells_[id]; cell != NULL; cell = cell->next) {
      const bool hittmp = prim_ray_intersect(primset, cell->prim_id, time, ray, isect_tmp);
      if (!hittmp)
        continue;

      // check if the hit point is inside the cell
      Box cellbox;
      cellbox.min.x = bounds_.min.x + cell_id[0] * cellsize_[0];
      cellbox.min.y = bounds_.min.y + cell_id[1] * cellsize_[1];
      cellbox.min.z = bounds_.min.z + cell_id[2] * cellsize_[2];
      cellbox.max.x = cellbox.min.x + cellsize_[0];
      cellbox.max.y = cellbox.min.y + cellsize_[1];
      cellbox.max.z = cellbox.min.z + cellsize_[2];
      const Vector P_hit = RayPointAt(ray, isect_tmp->t_hit);
      const bool inside_cell = BoxContainsPoint(cellbox, P_hit);

      if (!inside_cell)
        continue;

      // update info ONLY if isect->t_hit renewed
      if (isect_tmp->t_hit < isect_min->t_hit) {
        std::swap(isect_min, isect_tmp);
        hit = hittmp;
      }
    }
    if (hit) {
      *isect = *isect_min;
      break;
    }

    // advance to the next cell
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

const char *GridAccelerator::get_name() const
{
  return ACCELERATOR_NAME;
}

static void compute_grid_cellsizes(int nprimitives,
    Real xwidth, Real ywidth, Real zwidth,
    int *xncells, int *yncells, int *zncells)
{
  Real max_width = 0;
  if (xwidth > ywidth && xwidth > zwidth) {
    max_width = xwidth;
  }
  else if (ywidth > zwidth) {
    max_width = ywidth;
  }
  else {
    max_width = zwidth;
  }

  const Real cube_root = 3 * pow(nprimitives, 1./3);
  const Real ncells_per_unit_dist = cube_root / max_width;

  int XNCELLS = (int) floor(xwidth * ncells_per_unit_dist + .5);
  int YNCELLS = (int) floor(ywidth * ncells_per_unit_dist + .5);
  int ZNCELLS = (int) floor(zwidth * ncells_per_unit_dist + .5);
  XNCELLS = Clamp(XNCELLS, 1, GRID_MAXCELLS);
  YNCELLS = Clamp(YNCELLS, 1, GRID_MAXCELLS);
  ZNCELLS = Clamp(ZNCELLS, 1, GRID_MAXCELLS);

  *xncells = XNCELLS;
  *yncells = YNCELLS;
  *zncells = ZNCELLS;
}

static bool prim_ray_intersect(const PrimitiveSet *primset, int prim_id,
    Real time, const Ray &ray, Intersection *isect)
{
  const bool hit = primset->RayIntersect(prim_id, time, ray, isect);

  if (!hit) {
    isect->t_hit = REAL_MAX;
    return false;
  }

  if (isect->t_hit < ray.tmin || ray.tmax < isect->t_hit) {
    isect->t_hit = REAL_MAX;
    return false;
  }

  return true;
}

} // namespace xxx
