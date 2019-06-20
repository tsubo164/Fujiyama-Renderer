// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_importance_sampling.h"
#include "fj_numeric.h"
#include "fj_texture.h"
#include "fj_random.h"
#include "fj_vector.h"

#include <vector>
#include <cstdlib>
#include <cfloat>

namespace fj {

class SamplePoint {
public:
  SamplePoint() : x(0), y(0), label(0) {}
  ~SamplePoint() {}

public:
  int x, y;
  int label;
};
static int compare_sample_point(const void *ptr0, const void *ptr1);

static void make_histgram(Texture *texture,
    int sample_xres, int sample_yres, double *histgram);
static int lookup_histgram(const double *histgram, int pixel_count, double key_value);
static void index_to_uv(int xres, int yres, int index, TexCoord *uv);
static void uv_to_dir(float u, float v, Vector *dir);
static void xy_to_uv(int xres, int yres, int x, int y, TexCoord *uv);

// functions for structured importance sampling
static void setup_structured_importance_sampling(Texture *texture,
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
    DomeSample *dome_samples, int sample_count);

int ImportanceSampling(Texture *texture, int seed,
    int sample_xres, int sample_yres,
    DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  std::vector<double> histgram(NPIXELS);
  std::vector<char> picked(NPIXELS);

  XorShift rng;
  double sum = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    picked[i] = 0;
  }

  make_histgram(texture, sample_xres, sample_yres, &histgram[0]);
  sum = histgram[NPIXELS-1];

  for (i = 0; i < sample_count; i++) {
    for (;;) {
      const double rand = sum * rng.NextFloat01();
      const int index = lookup_histgram(&histgram[0], NPIXELS, rand);
      DomeSample sample;
      Color4 tex_rgba;

      if (picked[index] == 1) {
        // continue;
      }

      index_to_uv(sample_xres, sample_yres, index, &sample.uv);
      uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
      tex_rgba = texture->Lookup(sample.uv.u, sample.uv.v);
      sample.color.r = tex_rgba.r;
      sample.color.g = tex_rgba.g;
      sample.color.b = tex_rgba.b;

      dome_samples[i] = sample;
      picked[index] = 1;
      break;
    }
  }

  return 0;
}

int StratifiedImportanceSampling(Texture *texture, int seed,
    int sample_xres, int sample_yres,
    DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  std::vector<double> histgram(NPIXELS);
  std::vector<char> picked(NPIXELS);

  XorShift rng;
  double sum = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    picked[i] = 0;
  }

  make_histgram(texture, sample_xres, sample_yres, &histgram[0]);
  sum = histgram[NPIXELS-1];

  for (i = 0; i < seed; i++) {
    rng.NextFloat01();
  }

  for (i = 0; i < sample_count; i++) {
    for (;;) {
      const double rand = sum * ((i + rng.NextFloat01()) / sample_count);
      const int index = lookup_histgram(&histgram[0], NPIXELS, rand);
      DomeSample sample;
      Color4 tex_rgba;

      if (picked[index] == 1) {
        // continue;
      }

      index_to_uv(sample_xres, sample_yres, index, &sample.uv);
      uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
      tex_rgba = texture->Lookup(sample.uv.u, sample.uv.v);
      sample.color.r = tex_rgba.r;
      sample.color.g = tex_rgba.g;
      sample.color.b = tex_rgba.b;

      dome_samples[i] = sample;
      picked[index] = 1;
      break;
    }
  }

  return 0;
}

int StructuredImportanceSampling(Texture *texture, int seed,
    int sample_xres, int sample_yres,
    DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  const double delta_omega0 = .01;
  double thresholds[8] = {0};
  const int depth = 6;

  int sorted_label_count = 0;

  double L_whole = 0;
  double L_mean = 0;
  double L_sigma = 0;
  double gamma_whole = 0;

  // prepare
  std::vector<double> L(NPIXELS);
  setup_structured_importance_sampling(texture,
      sample_xres, sample_yres,
      &L[0], &L_whole, &L_mean);
  L_sigma = standard_deviation(&L[0], NPIXELS, L_mean);

  // thresholds
  for (int i = 0; i < depth; i++) {
    thresholds[i] = i * L_sigma;
  }

  // Gamma_{4 Pi} = L * dOmega_{0}^{1/4}
  gamma_whole = L_whole * pow(delta_omega0, 1./4);

  // layers
  std::vector<char> strata_id(NPIXELS);
  divide_into_layers(&L[0], NPIXELS,
      thresholds, depth,
      &strata_id[0]);

  // connections
  std::vector<int> connected_label(NPIXELS);
  solve_connected_components(
      sample_xres, sample_yres, &strata_id[0],
      &connected_label[0]);

  // re-map labels like 0, 1, ...
  remap_connected_label(NPIXELS, &connected_label[0], &sorted_label_count);

  // sample count for each connection
  std::vector<int> connected_sample_count(sorted_label_count);
  compute_connected_sample_count(&L[0], NPIXELS,
      &connected_label[0], sorted_label_count,
      sample_count, gamma_whole,
      &connected_sample_count[0]);

  generate_dome_samples(sample_xres, sample_yres,
      &connected_sample_count[0],
      &connected_label[0], sorted_label_count,
      dome_samples, sample_count);

  for (int i = 0; i < sample_count; i++) {
    DomeSample *sample = &dome_samples[i];
    Color4 tex_rgba;
    tex_rgba = texture->Lookup(sample->uv.u, sample->uv.v);
    sample->color.r = tex_rgba.r;
    sample->color.g = tex_rgba.g;
    sample->color.b = tex_rgba.b;
  }

  return 0;
}

static void make_histgram(Texture *texture,
    int sample_xres, int sample_yres, double *histgram)
{
  const int NPIXELS = sample_xres * sample_yres;
  double sum = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    Color4 tex_rgba;
    TexCoord uv;

    index_to_uv(sample_xres, sample_yres, i, &uv);
    tex_rgba = texture->Lookup(uv.u, uv.v);

    sum += Luminance4(tex_rgba);
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

static void index_to_uv(int xres, int yres, int index, TexCoord *uv)
{
  const int x = (int) (index % xres);
  const int y = (int) (index / xres);

  uv->u = (.5 + x) / xres;
  uv->v = 1. - ((.5 + y) / yres);
}

static void xy_to_uv(int xres, int yres, int x, int y, TexCoord *uv)
{
  uv->u = (.5 + x) / xres;
  uv->v = 1. - ((.5 + y) / yres);
}

static void uv_to_dir(float u, float v, Vector *dir)
{
  const double phi = 2 * PI * u;
  const double theta = PI * (v - .5);
  const double r = cos(theta);

  dir->x = r * sin(phi);
  dir->y = sin(theta);
  dir->z = r * cos(phi);
}

static void setup_structured_importance_sampling(Texture *texture,
    int sample_xres, int sample_yres,
    double *illum_values, double *whole_illum, double *mean_illum)
{
  const int NPIXELS = sample_xres * sample_yres;
  double *L = illum_values;
  double L_whole = 0;
  double L_mean = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    Color4 tex_rgba;
    TexCoord uv;

    index_to_uv(sample_xres, sample_yres, i, &uv);
    tex_rgba = texture->Lookup(uv.u, uv.v);

    L[i] = Luminance4(tex_rgba);
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

// Based on Fast Connect Components on Images
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
          // merge
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
  int max_label = 0;
  int label_count = 0;
  int new_label = 0;

  // find max label
  for (int i = 0; i < NPIXELS; i++) {
    if (max_label < connected_label[i]) {
      max_label = connected_label[i];
    }
  }
  label_count = max_label + 1; // for 0
  std::vector<int> label_map(label_count, -1);

  // re-map id from 0
  for (int i = 0; i < NPIXELS; i++) {
    const int old_label = connected_label[i];
    if (label_map[old_label] == -1) {
      label_map[old_label] = new_label;
      new_label++;
    }
    connected_label[i] = label_map[old_label];
  }
  *sorted_max_label = new_label;
}

static void compute_connected_sample_count(const double *illum_values, int nvalues,
    const int *connected_label, int label_count,
    int total_sample_count, double gamma_whole,
    int *connected_sample_count)
{
  std::vector<double> connected_domega(label_count, 0);
  std::vector<double> connected_illumi(label_count, 0);
  std::vector<int>    connected_area(label_count, 0);
  const double *L = illum_values;
  double dOmega0 = 0;
  int generated_count = 0;

  // Gamma_{4 Pi} = L * dOmega_{0}^{1/4}
  dOmega0 = 1 * pow(4 * PI / nvalues, 1./4);
  for (int i = 0; i < nvalues; i++) {
    const int label = connected_label[i];
    connected_domega[label] = dOmega0;
    connected_illumi[label] += L[i];

    connected_area[label]++;
  }

  for (int i = 0; i < label_count; i++) {
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
    for (int i = 0; i < label_count; i++) {
      if (max_area < connected_area[i]) {
        max_area = connected_area[i];
        max_area_index = i;
      }
    }
    connected_sample_count[max_area_index] +=
        total_sample_count - generated_count;
  }
}

// Hochbaum-Shmoys algorithm
static void generate_dome_samples(int sample_xres, int sample_yres,
    const int *connected_sample_count,
    const int *connected_label, int label_count,
    DomeSample *dome_samples, int sample_count)
{
  const int NPIXELS = sample_xres * sample_yres;
  std::vector<SamplePoint> samples(NPIXELS);
  std::vector<int>         nsamples(label_count);
  std::vector<int>         offsets(label_count);
  int next_dome_sample = 0;
  int i;

  for (i = 0; i < NPIXELS; i++) {
    samples[i].x = (int) (i % sample_xres);
    samples[i].y = (int) (i / sample_xres);
    samples[i].label = connected_label[i];
  }
  // TODO use std::sort
  qsort(&samples[0], NPIXELS,
      sizeof(SamplePoint),
      compare_sample_point);

  {
    int curr = 0;
    for (i = 0; i < label_count; i++) {
      const int begin = curr;
      offsets[i] = begin;
      while (curr < NPIXELS && samples[curr].label == i) {
        curr++;
      }
      nsamples[i] = curr - begin;
    }
  }

  {
    std::vector<SamplePoint> X(sample_count);
    XorShift rng;

    for (i = 0; i < label_count; i++) {
      const int ngen = connected_sample_count[i];
      const SamplePoint *Y = &samples[0] + offsets[i];
      const int nY = nsamples[i];
      const int x0 = (int) (nY * rng.NextFloat01());
      int nX = 0;
      int j;

      X[0] = Y[x0];
      nX++;

      if (ngen == 0) {
        continue;
      }
      {
        DomeSample sample;
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
          const SamplePoint *Yp = &Y[p];
          double d_min = FLT_MAX;
          int q;
          for (q = 0; q < nX; q++) {
            const SamplePoint *Xq = &X[q];
            const double xx = Xq->x - Yp->x;
            const double yy = Xq->y - Yp->y;
            const double d = xx * xx + yy * yy;
            d_min = Min(d_min, d);
          }
          if (d_max < d_min) {
            d_max = d_min;
            p_max = p;
          }
        }

        X[nX] = Y[p_max];
        nX++;

        {
          DomeSample sample;
          xy_to_uv(sample_xres, sample_yres, Y[p_max].x, Y[p_max].y, &sample.uv);
          uv_to_dir(sample.uv.u, sample.uv.v, &sample.dir);
          dome_samples[next_dome_sample] = sample;
          next_dome_sample++;
        }
      }
    }
  }
}

static int compare_sample_point(const void *ptr0, const void *ptr1)
{
  const SamplePoint *sample0 = (const SamplePoint *) ptr0;
  const SamplePoint *sample1 = (const SamplePoint *) ptr1;
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
