/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_importance_sampling.h"
#include "fj_numeric.h"
#include "fj_texture.h"
#include "fj_memory.h"
#include "fj_random.h"
#include "fj_vector.h"

#include <stdio.h>
#include <float.h>

namespace fj {

struct SamplePoint {
  int x, y;
  int label;
};
static int compare_sample_point(const void *ptr0, const void *ptr1);

static void make_histgram(struct Texture *texture,
    int sample_xres, int sample_yres, double *histgram);
static int lookup_histgram(const double *histgram, int pixel_count, double key_value);
static void index_to_uv(int xres, int yres, int index, struct TexCoord *uv);
static void uv_to_dir(float u, float v, struct Vector *dir);
static void xy_to_uv(int xres, int yres, int x, int y, struct TexCoord *uv);

/* functions for structured importance sampling */
static void setup_structured_importance_sampling(struct Texture *texture,
    int sample_xres, int sample_yres,
    double *illum_values, double *whole_illum, double *mean_illum);
static double standard_deviation(const double *illum_values, int nvalues, double mean_value);
static void divide_into_layers(const double *illum_values, int nvalues,
    const double *thresholds, int nthreshholds,
    char *strata_id);
static void solve_connected_components(
    int sample_xres, int sample_yres, const char *strata_id,
    int *connected_label);
static void remap_connected_label(int npixels,
    int *connected_label, int *sorted_max_label);
static void compute_connected_sample_count(const double *illum_values, int nvalues,
    const int *connected_label, int label_count,
    int total_sample_count, double gamma_whole,
    int *connected_sample_count);
static void generate_dome_samples(int sample_xres, int sample_yres,
    const int *connected_sample_count,
    const int *connected_label, int label_count,
    struct DomeSample *dome_samples, int sample_count);

int ImportanceSampling(struct Texture *texture, int seed,
    int sample_xres, int sample_yres,
    struct DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  double *histgram = FJ_MEM_ALLOC_ARRAY(double, NPIXELS);
  char *picked = FJ_MEM_ALLOC_ARRAY(char, NPIXELS);

  struct XorShift xr;
  double sum = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    picked[i] = 0;
  }

  make_histgram(texture, sample_xres, sample_yres, histgram);
  sum = histgram[NPIXELS-1];

  XorInit(&xr);
  for (i = 0; i < sample_count; i++) {
    for (;;) {
      const double rand = sum * XorNextFloat01(&xr);
      const int index = lookup_histgram(histgram, NPIXELS, rand);
      struct DomeSample sample = {{0}};
      struct Color4 tex_rgba = {0, 0, 0, 0};

      if (picked[index] == 1) {
        /*
        continue;
        */
      }

      index_to_uv(sample_xres, sample_yres, index, &sample.uv);
      uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
      TexLookup(texture, sample.uv.u, sample.uv.v, &tex_rgba);
      sample.color.r = tex_rgba.r;
      sample.color.g = tex_rgba.g;
      sample.color.b = tex_rgba.b;

      dome_samples[i] = sample;
      picked[index] = 1;
      break;
    }
  }

  FJ_MEM_FREE(picked);
  FJ_MEM_FREE(histgram);
  return 0;
}

int StratifiedImportanceSampling(struct Texture *texture, int seed,
    int sample_xres, int sample_yres,
    struct DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  double *histgram = FJ_MEM_ALLOC_ARRAY(double, NPIXELS);
  char *picked = FJ_MEM_ALLOC_ARRAY(char, NPIXELS);

  struct XorShift xr;
  double sum = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    picked[i] = 0;
  }

  make_histgram(texture, sample_xres, sample_yres, histgram);
  sum = histgram[NPIXELS-1];

  XorInit(&xr);
  for (i = 0; i < seed; i++) {
    XorNextFloat01(&xr);
  }

  for (i = 0; i < sample_count; i++) {
    for (;;) {
      const double rand = sum * ((i + XorNextFloat01(&xr)) / sample_count);
      const int index = lookup_histgram(histgram, NPIXELS, rand);
      struct DomeSample sample = {{0}};
      struct Color4 tex_rgba = {0, 0, 0, 0};

      if (picked[index] == 1) {
        /*
        continue;
        */
      }

      index_to_uv(sample_xres, sample_yres, index, &sample.uv);
      uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
      TexLookup(texture, sample.uv.u, sample.uv.v, &tex_rgba);
      sample.color.r = tex_rgba.r;
      sample.color.g = tex_rgba.g;
      sample.color.b = tex_rgba.b;

      dome_samples[i] = sample;
      picked[index] = 1;
      break;
    }
  }

  FJ_MEM_FREE(picked);
  FJ_MEM_FREE(histgram);
  return 0;
}

int StructuredImportanceSampling(struct Texture *texture, int seed,
    int sample_xres, int sample_yres,
    struct DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  const double delta_omega0 = .01;
  double thresholds[8] = {0};
  const int depth = 6;

  char *strata_id = NULL;
  int *connected_label = NULL;
  int *connected_sample_count = NULL;
  int sorted_label_count = 0;

  double *L = NULL;
  double L_whole = 0;
  double L_mean = 0;
  double L_sigma = 0;
  double gamma_whole = 0;
  int i;

  /* prepare */
  L = FJ_MEM_ALLOC_ARRAY(double, NPIXELS);
  setup_structured_importance_sampling(texture,
      sample_xres, sample_yres,
      L, &L_whole, &L_mean);
  L_sigma = standard_deviation(L, NPIXELS, L_mean);

  /* thresholds */
  for (i = 0; i < depth; i++) {
    thresholds[i] = i * L_sigma;
  }

  /* Gamma_{4 Pi} = L * dOmega_{0}^{1/4} */
  gamma_whole = L_whole * pow(delta_omega0, 1./4);

  /* layers */
  strata_id = FJ_MEM_ALLOC_ARRAY(char, NPIXELS);
  divide_into_layers(L, NPIXELS,
      thresholds, depth,
      strata_id);

  /* connections */
  connected_label = FJ_MEM_ALLOC_ARRAY(int, NPIXELS);
  solve_connected_components(
      sample_xres, sample_yres, strata_id,
      connected_label);

  /* re-map labels like 0, 1, ... */
  remap_connected_label(NPIXELS, connected_label, &sorted_label_count);

  /* sample count for each connection */
  connected_sample_count = FJ_MEM_ALLOC_ARRAY(int, sorted_label_count);
  compute_connected_sample_count(L, NPIXELS,
      connected_label, sorted_label_count,
      sample_count, gamma_whole,
      connected_sample_count);

  generate_dome_samples(sample_xres, sample_yres,
      connected_sample_count,
      connected_label, sorted_label_count,
      dome_samples, sample_count);

  for (i = 0; i < sample_count; i++) {
    struct DomeSample *sample = &dome_samples[i];
    struct Color4 tex_rgba = {0, 0, 0, 0};
    TexLookup(texture, sample->uv.u, sample->uv.v, &tex_rgba);
    sample->color.r = tex_rgba.r;
    sample->color.g = tex_rgba.g;
    sample->color.b = tex_rgba.b;
  }

  FJ_MEM_FREE(connected_sample_count);
  FJ_MEM_FREE(connected_label);
  FJ_MEM_FREE(strata_id);
  FJ_MEM_FREE(L);

  return 0;
}

static void make_histgram(struct Texture *texture,
    int sample_xres, int sample_yres, double *histgram)
{
  const int NPIXELS = sample_xres * sample_yres;
  double sum = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    struct Color4 tex_rgba = {0, 0, 0, 0};
    struct TexCoord uv = {0, 0};

    index_to_uv(sample_xres, sample_yres, i, &uv);
    TexLookup(texture, uv.u, uv.v, &tex_rgba);

    sum += .2989 * tex_rgba.r + .5866 * tex_rgba.g + .1145 * tex_rgba.b;
    histgram[i] = sum;
  }
}

static int lookup_histgram(const double *histgram, int pixel_count, double key_value)
{
  int i;

  for (i = 0; i < pixel_count; i++) {
    if (key_value < histgram[i]) {
      return i;
    }
  }
  return -1;
}

static void index_to_uv(int xres, int yres, int index, struct TexCoord *uv)
{
  const int x = (int) (index % xres);
  const int y = (int) (index / xres);

  uv->u = (.5 + x) / xres;
  uv->v = 1. - ((.5 + y) / yres);
}

static void xy_to_uv(int xres, int yres, int x, int y, struct TexCoord *uv)
{
  uv->u = (.5 + x) / xres;
  uv->v = 1. - ((.5 + y) / yres);
}

static void uv_to_dir(float u, float v, struct Vector *dir)
{
  const double phi = 2 * N_PI * u;
  const double theta = N_PI * (v - .5);
  const double r = cos(theta);

  dir->x = r * sin(phi);
  dir->y = sin(theta);
  dir->z = r * cos(phi);
}

static void setup_structured_importance_sampling(struct Texture *texture,
    int sample_xres, int sample_yres,
    double *illum_values, double *whole_illum, double *mean_illum)
{
  const int NPIXELS = sample_xres * sample_yres;
  double *L = illum_values;
  double L_whole = 0;
  double L_mean = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    struct Color4 tex_rgba = {0, 0, 0, 0};
    struct TexCoord uv = {0, 0};

    index_to_uv(sample_xres, sample_yres, i, &uv);
    TexLookup(texture, uv.u, uv.v, &tex_rgba);

    L[i] = .2989 * tex_rgba.r + .5866 * tex_rgba.g + .1145 * tex_rgba.b;
    L_whole += L[i];
  }
  L_mean = L_whole / NPIXELS;

  *whole_illum = L_whole;
  *mean_illum = L_mean;
}

static double standard_deviation(const double *illum_values, int nvalues, double mean_value)
{
  double sum = 0;
  int i;

  for (i = 0; i < nvalues; i++) {
    const double x = illum_values[i] - mean_value;
    sum += x * x;
  }
  sum /= nvalues;

  return sqrt(sum);
}

static void divide_into_layers(const double *illum_values, int nvalues,
    const double *thresholds, int nthreshholds,
    char *strata_id)
{
  int i;

  for (i = 0; i < nvalues; i++) {
    const double L = illum_values[i];
    int layer;

    for (layer = nthreshholds - 1; layer >= 0; layer--) {
      if (L > thresholds[layer]) {
        strata_id[i] = layer;
        break;
      }
    }
  }
}

/* Based on Fast Connect Components on Images */
static void solve_connected_components(
    int sample_xres, int sample_yres, const char *strata_id,
    int *connected_label)
{
  int next_label = 0;
  int x, y;

  for (y = 0; y < sample_yres; y++) {
    for (x = 0; x < sample_xres; x++) {
      const int top = (y-1) * sample_xres + x;
      const int curr = y * sample_xres + x;
      const int left = curr - 1;

      const int top_id  = y == 0 ? -1 : strata_id[top];
      const int left_id = x == 0 ? -1 : strata_id[left];
      const int curr_id = strata_id[curr];

      if (curr_id == left_id && curr_id == top_id) {
        if (connected_label[left] != connected_label[top]) {
          /* merge */
          const int left_label = connected_label[left];
          const int top_label = connected_label[top];
          int k;
          for (k = 0; k < curr; k++) {
            if (connected_label[k] == left_label) {
              connected_label[k] = top_label;
            }
          }
        }
        connected_label[curr] = connected_label[top];
      }
      else if (curr_id == top_id) {
        connected_label[curr] = connected_label[top];
      }
      else if (curr_id == left_id) {
        connected_label[curr] = connected_label[left];
      }
      else {
        connected_label[curr] = next_label;
        next_label++;
      }
    }
  }
}

static void remap_connected_label(int npixels,
    int *connected_label, int *sorted_max_label)
{
  const int NPIXELS = npixels;
  int *label_map = NULL;
  int max_label = 0;
  int label_count = 0;
  int new_label = 0;
  int i;

  /* find max label */
  for (i = 0; i < NPIXELS; i++) {
    if (max_label < connected_label[i]) {
      max_label = connected_label[i];
    }
  }
  label_count = max_label + 1; /* for 0 */
  label_map = FJ_MEM_ALLOC_ARRAY(int, label_count);

  /* initialize */
  for (i = 0; i < label_count; i++) {
    label_map[i] = -1;
  }

  /* re-map id from 0 */
  for (i = 0; i < NPIXELS; i++) {
    const int old_label = connected_label[i];
    if (label_map[old_label] == -1) {
      label_map[old_label] = new_label;
      new_label++;
    }
    connected_label[i] = label_map[old_label];
  }
  FJ_MEM_FREE(label_map);
  *sorted_max_label = new_label;
}

static void compute_connected_sample_count(const double *illum_values, int nvalues,
    const int *connected_label, int label_count,
    int total_sample_count, double gamma_whole,
    int *connected_sample_count)
{
  double *connected_domega = FJ_MEM_ALLOC_ARRAY(double, label_count);
  double *connected_illumi = FJ_MEM_ALLOC_ARRAY(double, label_count);
  int *connected_area = FJ_MEM_ALLOC_ARRAY(int, label_count);
  const double *L = illum_values;
  double dOmega0 = 0;
  int generated_count = 0;
  int i;

  for (i = 0; i < label_count; i++) {
    connected_domega[i] = 0;
    connected_illumi[i] = 0;

    connected_area[i] = 0;
  }

  /* Gamma_{4 Pi} = L * dOmega_{0}^{1/4} */
  dOmega0 = 1 * pow(4 * N_PI / nvalues, 1./4);
  for (i = 0; i < nvalues; i++) {
    const int label = connected_label[i];
    connected_domega[label] = dOmega0;
    connected_illumi[label] += L[i];

    connected_area[label]++;
  }

  for (i = 0; i < label_count; i++) {
    const double n = connected_illumi[i] * connected_domega[i] *
        total_sample_count / gamma_whole;

    connected_sample_count[i] = (int) (n + .5);
    generated_count += connected_sample_count[i];

    if (generated_count > total_sample_count) {
      const int over = generated_count - total_sample_count;
      connected_sample_count[i] -= over;
      generated_count -= over;
      break;
    }
  }

  if (generated_count < total_sample_count) {
    int max_area_index = 0;
    int max_area = -1;
    for (i = 0; i < label_count; i++) {
      if (max_area < connected_area[i]) {
        max_area = connected_area[i];
        max_area_index = i;
      }
    }
    connected_sample_count[max_area_index] +=
        total_sample_count - generated_count;
  }

  FJ_MEM_FREE(connected_domega);
  FJ_MEM_FREE(connected_illumi);
  FJ_MEM_FREE(connected_area);
}

/* Hochbaum-Shmoys algorithm */
static void generate_dome_samples(int sample_xres, int sample_yres,
    const int *connected_sample_count,
    const int *connected_label, int label_count,
    struct DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  struct SamplePoint *samples = FJ_MEM_ALLOC_ARRAY(struct SamplePoint, NPIXELS);
  int *nsamples = FJ_MEM_ALLOC_ARRAY(int, label_count);
  int *offsets = FJ_MEM_ALLOC_ARRAY(int, label_count);
  int next_dome_sample = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    samples[i].x = (int) (i % sample_xres);
    samples[i].y = (int) (i / sample_xres);
    samples[i].label = connected_label[i];
  }
  qsort(samples, NPIXELS,
      sizeof(struct SamplePoint),
      compare_sample_point);

  {
    int curr = 0;
    for (i = 0; i < label_count; i++) {
      const int begin = curr;
      offsets[i] = begin;
      while (samples[curr].label == i) {
        curr++;
      }
      nsamples[i] = curr - begin;
    }
  }

  {
    struct SamplePoint *X = FJ_MEM_ALLOC_ARRAY(struct SamplePoint, sample_count);
    struct XorShift xr;
    XorInit(&xr);

    for (i = 0; i < label_count; i++) {
      const int ngen = connected_sample_count[i];
      const struct SamplePoint *Y = samples + offsets[i];
      const int nY = nsamples[i];
      const int x0 = (int) (nY * XorNextFloat01(&xr));
      int nX = 0;
      int j;

      X[0] = Y[x0];
      nX++;

      if (ngen == 0) {
        continue;
      }
      {
        struct DomeSample sample = {{0}};
        xy_to_uv(sample_xres, sample_yres, Y[x0].x, Y[x0].y, &sample.uv);
        uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
        dome_samples[next_dome_sample] = sample;
        next_dome_sample++;
      }

      for (j = 1; j < ngen; j++) {
        double d_max = -FLT_MAX;
        int p_max = 0;
        int p;

        for (p = 0; p < nY; p++) {
          const struct SamplePoint *Yp = &Y[p];
          double d_min = FLT_MAX;
          int q;
          for (q = 0; q < nX; q++) {
            const struct SamplePoint *Xq = &X[q];
            const double xx = Xq->x - Yp->x;
            const double yy = Xq->y - Yp->y;
            const double d = xx * xx + yy * yy;
            d_min = MIN(d_min, d);
          }
          if (d_max < d_min) {
            d_max = d_min;
            p_max = p;
          }
        }

        X[nX] = Y[p_max];
        nX++;

        {
          struct DomeSample sample = {{0}};
          xy_to_uv(sample_xres, sample_yres, Y[p_max].x, Y[p_max].y, &sample.uv);
          uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
          dome_samples[next_dome_sample] = sample;
          next_dome_sample++;
        }
      }
    }
    FJ_MEM_FREE(X);
  }

  FJ_MEM_FREE(samples);
  FJ_MEM_FREE(offsets);
  FJ_MEM_FREE(nsamples);
}

static int compare_sample_point(const void *ptr0, const void *ptr1)
{
  const struct SamplePoint *sample0 = (const struct SamplePoint *) ptr0;
  const struct SamplePoint *sample1 = (const struct SamplePoint *) ptr1;
  const double x = sample0->label;
  const double y = sample1->label;

  if (x > y)
    return 1;
  else if (x < y)
    return -1;
  else
    return 0;
}

} // namespace xxx
