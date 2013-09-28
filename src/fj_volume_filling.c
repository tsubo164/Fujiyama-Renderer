/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_volume_filling.h"
#include "fj_numeric.h"
#include "fj_vector.h"
#include "fj_volume.h"

#define VEC3_BILERP(dst,v00,v10,v01,v11,s,t) do { \
  (dst)->x = Bilerp((v00)->x, (v10)->x, (v01)->x, (v11)->x, (s), (t)); \
  (dst)->y = Bilerp((v00)->y, (v10)->y, (v01)->y, (v11)->y, (s), (t)); \
  (dst)->z = Bilerp((v00)->z, (v10)->z, (v01)->z, (v11)->z, (s), (t)); \
  } while(0)

void LerpWispConstrolPoint(struct WispsControlPoint *cp,
    const struct WispsControlPoint *cp0, const struct WispsControlPoint *cp1,
    double t)
{
  VEC3_LERP(&cp->orig, &cp0->orig, &cp1->orig, t);
  VEC3_LERP(&cp->udir, &cp0->udir, &cp1->udir, t);
  VEC3_LERP(&cp->vdir, &cp0->vdir, &cp1->vdir, t);
  VEC3_LERP(&cp->wdir, &cp0->wdir, &cp1->wdir, t);
  VEC3_LERP(&cp->noise_space, &cp0->noise_space, &cp1->noise_space, t);
  VEC3_NORMALIZE(&cp->udir);
  VEC3_NORMALIZE(&cp->vdir);
  VEC3_NORMALIZE(&cp->wdir);

  cp->density = LERP(cp0->density, cp1->density, t);
  cp->radius = LERP(cp0->radius, cp1->radius, t);
  cp->noise_amplitude = LERP(cp0->noise_amplitude, cp1->noise_amplitude, t);
  /* TODO should not point attribute? */
  cp->speck_count = LERP(cp0->speck_count, cp1->speck_count, t);
  cp->speck_radius = LERP(cp0->speck_radius, cp1->speck_radius, t);
}

extern void BilerpWispConstrolPoint(struct WispsControlPoint *cp,
    const struct WispsControlPoint *cp00, const struct WispsControlPoint *cp10,
    const struct WispsControlPoint *cp01, const struct WispsControlPoint *cp11,
    double s, double t)
{
  VEC3_BILERP(&cp->orig, &cp00->orig, &cp10->orig, &cp01->orig, &cp11->orig, s, t);
  VEC3_BILERP(&cp->udir, &cp00->udir, &cp10->udir, &cp01->udir, &cp11->udir, s, t);
  VEC3_BILERP(&cp->vdir, &cp00->vdir, &cp10->vdir, &cp01->vdir, &cp11->vdir, s, t);
  VEC3_BILERP(&cp->wdir, &cp00->wdir, &cp10->wdir, &cp01->wdir, &cp11->wdir, s, t);
  VEC3_BILERP(&cp->noise_space, &cp00->noise_space, &cp10->noise_space,
      &cp01->noise_space, &cp11->noise_space, s, t);
  VEC3_NORMALIZE(&cp->udir);
  VEC3_NORMALIZE(&cp->vdir);
  VEC3_NORMALIZE(&cp->wdir);

  cp->density = Bilerp(cp00->density, cp10->density, cp01->density, cp11->density, s, t);
  cp->radius = Bilerp(cp00->radius, cp10->radius, cp01->radius, cp11->radius, s, t);
  cp->noise_amplitude = Bilerp(cp00->noise_amplitude, cp10->noise_amplitude,
      cp01->noise_amplitude, cp11->noise_amplitude, s, t);
  /* TODO should not point attribute? */
  cp->speck_count = Bilerp(cp00->speck_count, cp10->speck_count,
      cp01->speck_count, cp11->speck_count, s, t);
  cp->speck_radius = Bilerp(cp00->speck_radius, cp10->speck_radius,
      cp01->speck_radius, cp11->speck_radius, s, t);
}

void FillWithSphere(struct Volume *volume,
    const struct Vector *center, double radius, float density)
{
  int i, j, k;
  int xmin, ymin, zmin;
  int xmax, ymax, zmax;
  const double thresholdwidth = .5 * VolGetFilterSize(volume);

  VolGetIndexRange(volume, center, radius,
      &xmin, &ymin, &zmin,
      &xmax, &ymax, &zmax);

  for (k = zmin; k <= zmax; k++) {
    for (j = ymin; j <= ymax; j++) {
      for (i = xmin; i <= xmax; i++) {
        struct Vector P = {0, 0, 0};
        float value = 0;

        VolIndexToPoint(volume, i, j, k, &P);

        P.x -= center->x;
        P.y -= center->y;
        P.z -= center->z;

        value = VolGetValue(volume, i, j, k);
        value += density * Fit(VEC3_LEN(&P) - radius,
            -thresholdwidth, thresholdwidth, 1, 0);
        VolSetValue(volume, i, j, k, value);
      }
    }
  }
}

