// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_IMPORTANCE_SAMPLING_H
#define FJ_IMPORTANCE_SAMPLING_H

#include "fj_tex_coord.h"
#include "fj_vector.h"
#include "fj_color.h"

namespace fj {

class Texture;

class DomeSample {
public:
  DomeSample() {}
  ~DomeSample() {}

public:
  Color color;
  TexCoord uv;
  Vector dir;
};

extern int ImportanceSampling(Texture *texture, int seed,
    int sample_xres, int sample_yres,
    DomeSample *dome_samples, int sample_count);

extern int StratifiedImportanceSampling(Texture *texture, int seed,
    int sample_xres, int sample_yres,
    DomeSample *dome_samples, int sample_count);

extern int StructuredImportanceSampling(Texture *texture, int seed,
    int sample_xres, int sample_yres,
    DomeSample *dome_samples, int sample_count);

} // namespace xxx

#endif // FJ_XXX_H
