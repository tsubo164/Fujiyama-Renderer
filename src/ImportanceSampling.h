/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef IMPORTANCESAMPLING_H
#define IMPORTANCESAMPLING_H

#include "TexCoord.h"
#include "Vector.h"
#include "Color.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Texture;

struct DomeSample {
  struct Color color;
  struct TexCoord uv;
  struct Vector dir;
};

extern int ImportanceSampling(struct Texture *texture, int seed,
    int sample_xres, int sample_yres,
    struct DomeSample *dome_samples, int sample_count);

extern int StratifiedImportanceSampling(struct Texture *texture, int seed,
    int sample_xres, int sample_yres,
    struct DomeSample *dome_samples, int sample_count);

extern int StructuredImportanceSampling(struct Texture *texture, int seed,
    int sample_xres, int sample_yres,
    struct DomeSample *dome_samples, int sample_count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

