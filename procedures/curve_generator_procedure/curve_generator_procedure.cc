// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_triangle.h"
#include "fj_random.h"
#include "fj_noise.h"
#include <iostream>

using namespace fj;

class CurveGeneratorProcedure : Procedure {
public:
  CurveGeneratorProcedure() : mesh(NULL), add_velocity(false) {}
  virtual ~CurveGeneratorProcedure() {}

public:
  Mesh *mesh;
  Curve *curve;
  bool add_velocity;

private:
  virtual int run() const;
};

static void *MyCreateFunction(void);
static void MyDeleteFunction(void *self);
static const char MyPluginName[] = "CurveGeneratorProcedure";

static int set_mesh(void *self, const PropertyValue &value);
static int set_curve(void *self, const PropertyValue &value);
static int set_add_velocity(void *self, const PropertyValue &value);

static int generate_curve(const Mesh &mesh, Curve &curve, bool add_velocity);

static const Property MyPropertyList[] = {
  Property("mesh",         PropMesh(NULL),  set_mesh),
  Property("curve",        PropCurve(NULL), set_curve),
  Property("add_velocity", PropScalar(0),   set_add_velocity),
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

  const int err = generate_curve(*mesh, *curve, add_velocity);

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

static int set_add_velocity(void *self, const PropertyValue &value)
{
  CurveGeneratorProcedure *crvgen = (CurveGeneratorProcedure *) self;

  crvgen->add_velocity = value.vector[0] > 0;

  return 0;
}

static int generate_curve(const Mesh &mesh, Curve &curve, bool add_velocity)
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
  // velocity
  /*
  curve.AddVertexVelocity();
  for (int i = 0; i < total_ncps; i++) {
    curve.SetVertexVelocity(i, velocity[i]);
  }
  */
  // indices
  curve.AddCurveIndices();
  for (int i = 0; i < total_ncurves; i++) {
    curve.SetCurveIndices(i, indices[i]);
  }

  curve.ComputeBounds();
  std::cout << curve.GetBounds() << "\n";

  return 0;
}
