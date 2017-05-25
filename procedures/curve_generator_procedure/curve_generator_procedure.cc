// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_triangle.h"
#include "fj_random.h"
#include "fj_noise.h"
#include <iostream>
#include <cstdlib>

using namespace fj;

class CurveGeneratorProcedure : Procedure {
public:
  CurveGeneratorProcedure() : mesh(NULL), is_hair(false) {}
  virtual ~CurveGeneratorProcedure() {}

public:
  Mesh *mesh;
  Curve *curve;
  bool is_hair;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "CurveGeneratorProcedure";

static int set_mesh(void *self, const PropertyValue &value);
static int set_curve(void *self, const PropertyValue &value);
static int set_is_hair(void *self, const PropertyValue &value);

static int generate_curve(const Mesh &mesh, Curve &curve);
static int generate_hair(const Mesh &mesh, Curve &curve);

static const Property MyPropertyList[] = {
  Property("mesh",    PropMesh(NULL),  set_mesh),
  Property("curve",   PropCurve(NULL), set_curve),
  Property("is_hair", PropScalar(0),   set_is_hair),
  Property()
};

static const MetaInfo MyMetainfo[] = {
  {"help", "Curve Generator procedure."},
  {"plugin_type", "Procedure"},
  {NULL, NULL}
};

extern "C" {
FJ_PLUGIN_API int Initialize(PluginInfo *info)
{
  return PlgSetupInfo(info,
      PLUGIN_API_VERSION,
      PROCEDURE_PLUGIN_TYPE,
      MyPluginName,
      MyCreateFunction,
      MyDeleteFunction,
      MyPropertyList,
      MyMetainfo);
}
} // extern "C"

static void *MyCreateFunction(void)
{
  CurveGeneratorProcedure *crvgen = new CurveGeneratorProcedure();

  return crvgen;
}

static void MyDeleteFunction(void *self)
{
  CurveGeneratorProcedure *crvgen = (CurveGeneratorProcedure *) self;
  if (crvgen == NULL)
    return;
  delete crvgen;
}

int CurveGeneratorProcedure::run() const
{
  if (mesh == NULL) {
    return -1;
  }
  if (curve == NULL) {
    return -1;
  }

  int err;

  if (is_hair) {
    err = generate_hair(*mesh, *curve);
  } else {
    err = generate_curve(*mesh, *curve);
  }

  return err;
}

static int set_mesh(void *self, const PropertyValue &value)
{
  CurveGeneratorProcedure *crvgen = (CurveGeneratorProcedure *) self;

  if (value.mesh == NULL)
    return -1;

  crvgen->mesh = value.mesh;

  return 0;
}

static int set_curve(void *self, const PropertyValue &value)
{
  CurveGeneratorProcedure *crvgen = (CurveGeneratorProcedure *) self;

  if (value.curve == NULL)
    return -1;

  crvgen->curve = value.curve;

  return 0;
}

static int set_is_hair(void *self, const PropertyValue &value)
{
  CurveGeneratorProcedure *crvgen = (CurveGeneratorProcedure *) self;

  crvgen->is_hair = value.vector[0] > 0;

  return 0;
}

static int generate_curve(const Mesh &mesh, Curve &curve)
{
  curve.Clear();

  const int FACE_COUNT = mesh.GetFaceCount();
  std::cout << "face count: " << FACE_COUNT << "\n";

  // count total_ncurves
  std::vector<int> ncurves_on_face(FACE_COUNT);
  int total_ncurves = 0;

  for (int i = 0; i < FACE_COUNT; i++) {
    Vector P0, P1, P2;
    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);

    const double area = TriComputeArea(P0, P1, P2);
    ncurves_on_face[i] = 100000 * area;
    total_ncurves += ncurves_on_face[i];
  }
  std::cout << "total curve count: " << total_ncurves << "\n";

  const int total_ncps = 4 * total_ncurves;
  std::vector<Vector> P(total_ncps);
  std::vector<double> width(total_ncps);
  std::vector<Color>  Cd(total_ncps);
  std::vector<int>    indices(total_ncurves);

  std::vector<Vector> sourceP(total_ncurves);
  std::vector<Vector> sourceN(total_ncurves);

  std::cout << "Computing curve's positions ..." << "\n";

  int curve_id = 0;
  for (int i = 0; i < FACE_COUNT; i++) {
    Vector P0, P1, P2;
    Vector N0, N1, N2;
    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);
    MshGetFacePointNormal(&mesh, i, &N0, &N1, &N2);

    const int CURVE_COUNT = ncurves_on_face[i];

    for (int j = 0; j < CURVE_COUNT; j++) {
      srand(12.34*i + 1232*j);
      const double u = (((double) rand()) / RAND_MAX);
      srand(21.43*i + 213*j);
      const double v = (1-u) * (((double) rand()) / RAND_MAX);

      const double t = 1-u-v;
      Vector src_P = t * P0 + u * P1 + v * P2;
      Vector src_N = t * N0 + u * N1 + v * N2;
      src_N = Normalize(src_N);

      srand(i+j);
      const double gravity = .5 + .5 * (static_cast<double>(rand()) / RAND_MAX);
      src_N.y -= gravity;
      src_N = Normalize(src_N);

      sourceP[curve_id] = src_P;
      sourceN[curve_id] = src_N;
      curve_id++;
    }
  }
  assert(curve_id == total_ncurves);

  int cp_id = 0;
  curve_id = 0;
  std::cout << "Generating curves ...\n";

  for (int i = 0; i < total_ncurves; i++) {
    for (int vtx = 0; vtx < 4; vtx++) {
      // noise
      Vector noisevec;
      srand(12*i + 49*vtx);
      if (vtx > 0) {
        noisevec.x = (((double) rand()) / RAND_MAX);
        noisevec.y = (((double) rand()) / RAND_MAX);
        noisevec.z = (((double) rand()) / RAND_MAX);
      }

      // P
      Vector &src_P = sourceP[curve_id];
      Vector &src_N = sourceN[curve_id];

      const double LENGTH = .02;
      const double noiseamp = .75 * LENGTH;
      const Vector dst_P = src_P + noiseamp * noisevec + vtx * LENGTH/3. * src_N;
      P[cp_id] = dst_P;

      // width
      if (vtx == 0) {
        double *w = &width[cp_id];
        w[0] = .003;
        w[1] = .002;
        w[2] = .001;
        w[3] = .0001;
      }

      // Cd
      double amp = 1;
      const Color C_dark(.8, .5, .3);
      const Color C_light(.9, .88, .85);
      const Vector freq(3, 3, 3);
      const Vector offset(0, 1, 0);

      const Vector src_Q = src_P * freq + offset;
      double C_noise = amp * PerlinNoise(src_Q, 2, .5, 2);
      C_noise = SmoothStep(.55, .75, C_noise);
      const Color dst_Cd = Lerp(C_dark, C_light, C_noise);
      Cd[cp_id] = dst_Cd;

      // cp_id
      cp_id++;
    }

    indices[curve_id] = 4*i;
    curve_id++;
  }
  assert(cp_id == total_ncps);

  // count
  curve.SetVertexCount(total_ncps);
  curve.SetCurveCount(total_ncurves);

  // P
  curve.AddVertexPosition();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexPosition(i, P[i]);
  }
  // width
  curve.AddVertexWidth();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexWidth(i, width[i]);
  }
  // Cd
  curve.AddVertexColor();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexColor(i, Cd[i]);
  }
  // indices
  curve.AddCurveIndices();
  for (int i = 0; i < total_ncurves; i++) {
    curve.SetCurveIndices(i, indices[i]);
  }

  curve.ComputeBounds();

  return 0;
}

static int generate_hair(const Mesh &mesh, Curve &curve)
{
  curve.Clear();

  const Box bounds = mesh.GetBounds();
  const double ymin = bounds.min.y;
  const double ymax = bounds.max.y;
  const double zmin = bounds.min.z;
  const double zmax = bounds.max.z;

  const int FACE_COUNT = mesh.GetFaceCount();
  std::cout << "face cout: " << FACE_COUNT << "\n";

  // count total_ncurves
  std::vector<int> ncurves_on_face(FACE_COUNT);
  const int N_CURVES_PER_HAIR = 5;
  int total_ncurves = 0;

  for (int i = 0; i < FACE_COUNT; i++) {
    Vector P0, P1, P2;
    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);
    const double area = TriComputeArea(P0, P1, P2);

    const double ycenter = (P0.y + P1.y + P2.y) / 3.;
    const double ynml = (ycenter - ymin) / (ymax - ymin);
    const double zcenter = (P0.z + P1.z + P2.z) / 3.;
    const double znml = (zcenter - zmin) / (zmax - zmin);

    ncurves_on_face[i] = 100000 * area;

    if (ynml < .5 || znml > .78) {
      ncurves_on_face[i] = 0;
    }

    total_ncurves += ncurves_on_face[i] * N_CURVES_PER_HAIR;
  }
  std::cout << "total curve count: " << total_ncurves << "\n";

  const int total_ncps = 4 * total_ncurves;
  std::vector<Vector> P(total_ncps);
  std::vector<double> width(total_ncps);
  std::vector<Color>  Cd(total_ncps);
  std::vector<Vector> velocity(total_ncps);
  std::vector<int>    indices(total_ncurves);

  std::cout << "Computing curve's positions ...\n";

  XorShift rng;
  int strand_id = 0;
  int curve_id = 0;
  int cp_id = 0;

  for (int i = 0; i < FACE_COUNT; i++) {
    Vector P0, P1, P2;
    Vector N0, N1, N2;
    MshGetFacePointPosition(&mesh, i, &P0, &P1, &P2);
    MshGetFacePointNormal(&mesh, i, &N0, &N1, &N2);

    for (int j = 0; j < ncurves_on_face[i]; j++) {
      const double u = rng.NextFloat01();
      const double v = (1-u) * rng.NextFloat01();
      const double t = 1-u-v;

      const Vector src_P = t * P0 + u * P1 + v * P2;
      Vector src_N = t * N0 + u * N1 + v * N2;
      src_N = Normalize(src_N);
      src_N.y = Min(src_N.y, .1);

      if (src_N.x < .1 && src_N.z < .1) {
        src_N.x /= src_N.x;
        src_N.z /= src_N.z;
        src_N.x *= .5;
        src_N.z *= .5;
      }
      src_N = Normalize(src_N);

      Vector next_P = src_P;
      Vector next_N = src_N;

      for (int k = 0; k < N_CURVES_PER_HAIR; k++) {
        // the first cp_id of curve
        indices[curve_id] = cp_id;

        for (int vtx = 0; vtx < 4; vtx++) {
          const double w[4] = {1, .5, .2, .05};
          P[cp_id] = next_P;
          Cd[cp_id] = Color(.9, .8, .5);

          if (k == N_CURVES_PER_HAIR - 1) {
            width[cp_id] = .0005 * w[vtx];
          }
          else {
            width[cp_id] = .0005;
          }

          // update the 'next' next_P if not the last control point
          if (vtx != 3) {
            const Vector curr_P = P[cp_id];
            const double amp = .002 * .1;
            const double freq = 100;
            const double segment_len = .01;

            const Vector Q = curr_P * Vector(freq, 2, freq);
            const Vector noise_vec = PerlinNoise3d(Q, 2, .5, 2);

            next_P += segment_len * next_N + Vector(amp, 0, amp) * noise_vec;
            next_N = next_P - curr_P;
            next_N = Normalize(next_N);

            next_N.y += -.5;
            next_N = Normalize(next_N);
          }
          // compute velocity
          {
            const Vector curr_P = P[cp_id];
            const double amp = .01;
            const double freq = 1;

            const Vector Q = freq * curr_P + Vector(0, 5, 0);
            const Vector noise_vec = PerlinNoise3d(Q, 2, .5, 2);

            const double vmult = SmoothStep(1, N_CURVES_PER_HAIR, k);
            const Vector curr_v = vmult * amp * noise_vec;

            velocity[cp_id] = curr_v;
          }

          cp_id++;
        }
        curve_id++;
      }

      strand_id++;
    }
  }
  assert(strand_id == total_ncurves / N_CURVES_PER_HAIR);

  // count
  curve.SetVertexCount(total_ncps);
  curve.SetCurveCount(total_ncurves);

  // P
  curve.AddVertexPosition();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexPosition(i, P[i]);
  }
  // width
  curve.AddVertexWidth();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexWidth(i, width[i]);
  }
  // Cd
  curve.AddVertexColor();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexColor(i, Cd[i]);
  }
  // velocity
  curve.AddVertexVelocity();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexVelocity(i, velocity[i]);
  }
  // indices
  curve.AddCurveIndices();
  for (int i = 0; i < total_ncurves; i++) {
    curve.SetCurveIndices(i, indices[i]);
  }

  curve.ComputeBounds();

  return 0;
}
