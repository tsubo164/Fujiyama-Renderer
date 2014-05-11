/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_IMPORTANCESAMPLING_H
#define FJ_IMPORTANCESAMPLING_H

#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"

namespace fj {

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

} // namespace xxx

#endif /* FJ_XXX_H */
