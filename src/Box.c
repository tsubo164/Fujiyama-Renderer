/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Box.h"
#include "Numeric.h"
#include <stdio.h>
#include <math.h>

int BoxContainsPoint(const struct Box *box, const struct Vector *point)
{
  if ((point->x < box->min.x) || (point->x > box->max.x)) return 0;
  if ((point->y < box->min.y) || (point->y > box->max.y)) return 0;
  if ((point->z < box->min.z) || (point->z > box->max.z)) return 0;

  return 1;
}

void BoxAddPoint(struct Box *box, const struct Vector *point)
{
  box->min.x = MIN(box->min.x, point->x);
  box->min.y = MIN(box->min.y, point->y);
  box->min.z = MIN(box->min.z, point->z);
  box->max.x = MAX(box->max.x, point->x);
  box->max.y = MAX(box->max.y, point->y);
  box->max.z = MAX(box->max.z, point->z);
}

void BoxAddBox(struct Box *box, const struct Box *otherbox)
{
  box->min.x = MIN(box->min.x, otherbox->min.x);
  box->min.y = MIN(box->min.y, otherbox->min.y);
  box->min.z = MIN(box->min.z, otherbox->min.z);
  box->max.x = MAX(box->max.x, otherbox->max.x);
  box->max.y = MAX(box->max.y, otherbox->max.y);
  box->max.z = MAX(box->max.z, otherbox->max.z);
}

int BoxRayIntersect(const struct Box *box,
    const struct Vector *rayorig, const struct Vector *raydir,
    double ray_tmin, double ray_tmax,
    double *hit_tmin, double *hit_tmax)
{
  int hit;
  double tmin, tmax, tymin, tymax, tzmin, tzmax;

  if (raydir->x >= 0) {
    tmin = (box->min.x - rayorig->x) / raydir->x;
    tmax = (box->max.x - rayorig->x) / raydir->x;
  } else {
    tmin = (box->max.x - rayorig->x) / raydir->x;
    tmax = (box->min.x - rayorig->x) / raydir->x;
  }

  if (raydir->y >= 0) {
    tymin = (box->min.y - rayorig->y) / raydir->y;
    tymax = (box->max.y - rayorig->y) / raydir->y;
  } else {
    tymin = (box->max.y - rayorig->y) / raydir->y;
    tymax = (box->min.y - rayorig->y) / raydir->y;
  }

  if ((tmin > tymax) || (tymin > tmax))
    return 0;

  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;

  if (raydir->z >= 0) {
    tzmin = (box->min.z - rayorig->z) / raydir->z;
    tzmax = (box->max.z - rayorig->z) / raydir->z;
  } else {
    tzmin = (box->max.z - rayorig->z) / raydir->z;
    tzmax = (box->min.z - rayorig->z) / raydir->z;
  }

  if ((tmin > tzmax) || (tzmin > tmax))
    return 0;

  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  hit = ((tmin < ray_tmax) && (tmax > ray_tmin));
  if (hit) {
    *hit_tmin = tmin;
    *hit_tmax = tmax;
  }

  return hit;
}

void BoxCentroid(const struct Box *box, struct Vector *centroid)
{
  centroid->x = .5 * (box->min.x + box->max.x);
  centroid->y = .5 * (box->min.y + box->max.y);
  centroid->z = .5 * (box->min.z + box->max.z);
}

double BoxDiagonal(const struct Box *box)
{
  const double xsize = BOX3_XSIZE(box);
  const double ysize = BOX3_YSIZE(box);
  const double zsize = BOX3_ZSIZE(box);

  return .5 * sqrt(xsize * xsize + ysize * ysize + zsize * zsize);
}

void BoxPrint(const struct Box *box)
{
  printf("(%g, %g, %g) (%g, %g, %g)\n",
      box->min.x, box->min.y, box->min.z,
      box->max.x, box->max.y, box->max.z);
}

