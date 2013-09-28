/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VOLUME_FILLING_H
#define VOLUME_FILLING_H

#include "fj_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Volume;

struct CloudControlPoint {
  struct Vector orig;
  struct Vector udir;
  struct Vector vdir;
  struct Vector wdir;
  struct Vector noise_space;

  double density;
  double radius;
  double noise_amplitude;
};

struct WispsControlPoint {
  struct Vector orig;
  struct Vector udir;
  struct Vector vdir;
  struct Vector wdir;
  struct Vector noise_space;

  double density;
  double radius;
  double noise_amplitude;

  double speck_count;
  double speck_radius;
};

extern void LerpWispConstrolPoint(struct WispsControlPoint *cp,
    const struct WispsControlPoint *cp0, const struct WispsControlPoint *cp1,
    double t);

extern void BilerpWispConstrolPoint(struct WispsControlPoint *cp,
    const struct WispsControlPoint *cp00, const struct WispsControlPoint *cp10,
    const struct WispsControlPoint *cp01, const struct WispsControlPoint *cp11,
    double s, double t);

extern void FillWithSphere(struct Volume *volume,
    const struct Vector *center, double radius, float density);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

