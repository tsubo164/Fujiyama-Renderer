// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_RENDERER_H
#define FJ_RENDERER_H

#include "fj_compatibility.h"
#include "fj_callback.h"
#include "fj_progress.h"
#include "fj_timer.h"

namespace fj {

class Renderer;
class FrameBuffer;
class ObjectGroup;
class Camera;
class Light;

class FrameProgress {
public:
  FrameProgress() :
      timer(),
      progress(),
      iteration_list(),
      current_segment(0) {}
  ~FrameProgress() {}

  Timer timer;
  Progress progress;
  Iteration iteration_list[10]; // one for 10% 100% in total
  int current_segment;
};

class Renderer {
public:
  Renderer();
  ~Renderer();

  void SetResolution(int xres, int yres);
  void SetRenderRegion(int xmin, int ymin, int xmax, int ymax);
  void SetPixelSamples(int xrate, int yrate);
  void SetTileSize(int xtilesize, int ytilesize);
  void SetFilterWidth(float xfwidth, float yfwidth);
  void SetSampleJitter(float jitter);
  void SetSampleTimeRange(double start_time, double end_time);

  void SetShadowEnable(int enable);
  void SetMaxReflectDepth(int max_depth);
  void SetMaxRefractDepth(int max_depth);

  void SetRaymarchStep(double step);
  void SetRaymarchShadowStep(double step);
  void SetRaymarchReflectStep(double step);
  void SetRaymarchRefractStep(double step);

  void SetCamera(Camera *cam);
  void SetFrameBuffers(FrameBuffer *fb);
  void SetTargetObjects(ObjectGroup *grp);
  void SetTargetLights(Light **lights, int nlights);

  // use max thread if use_max_thread is 1, otherwise takes account for thread_count
  void SetUseMaxThread(int use_max_thread);
  void SetThreadCount(int thread_count);
  int GetThreadCount() const;

  void SetFrameReportCallback(void *data,
      FrameStartCallback frame_start,
      FrameDoneCallback frame_done);

  void SetTileReportCallback(void *data,
      TileStartCallback tile_start,
      SampleDoneCallback sample_done,
      TileDoneCallback tile_done);

  int RenderScene();

public:
  int prepare_rendering();
  int execute_rendering();

  int preprocess_camera() const;
  int preprocess_framebuffer() const;
  int preprocess_lights();

  Camera *camera_;
  FrameBuffer *framebuffer_;
  ObjectGroup *target_objects_;
  Light **target_lights_;
  int nlights_;

  int resolution_[2];
  Rectangle frame_region_;
  int pixelsamples_[2];
  int tilesize_[2];
  float filterwidth_[2];
  float jitter_;
  double sample_time_start_;
  double sample_time_end_;

  int cast_shadow_;
  int max_reflect_depth_;
  int max_refract_depth_;

  double raymarch_step_;
  double raymarch_shadow_step_;
  double raymarch_reflect_step_;
  double raymarch_refract_step_;

  int use_max_thread_;
  int thread_count_;

  FrameReport frame_report_;
  TileReport tile_report_;
  FrameProgress frame_progress_;

  int32_t frame_id_;
};

} // namespace xxx

#endif // FJ_XXX_H
