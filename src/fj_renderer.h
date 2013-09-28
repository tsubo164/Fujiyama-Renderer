/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef RENDERER_H
#define RENDERER_H

#include "fj_callback.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Renderer;
struct FrameBuffer;
struct ObjectGroup;
struct Camera;
struct Light;

extern struct Renderer *RdrNew(void);
extern void RdrFree(struct Renderer *renderer);

extern void RdrSetResolution(struct Renderer *renderer, int xres, int yres);
extern void RdrSetRenderRegion(struct Renderer *renderer, int xmin, int ymin, int xmax, int ymax);
extern void RdrSetPixelSamples(struct Renderer *renderer, int xrate, int yrate);
extern void RdrSetTileSize(struct Renderer *renderer, int xtilesize, int ytilesize);
extern void RdrSetFilterWidth(struct Renderer *renderer, float xfwidth, float yfwidth);
extern void RdrSetSampleJitter(struct Renderer *renderer, float jitter);
extern void RdrSetSampleTimeRange(struct Renderer *renderer, double start_time, double end_time);

extern void RdrSetShadowEnable(struct Renderer *renderer, int enable);
extern void RdrSetMaxReflectDepth(struct Renderer *renderer, int max_depth);
extern void RdrSetMaxRefractDepth(struct Renderer *renderer, int max_depth);

extern void RdrSetRaymarchStep(struct Renderer *renderer, double step);
extern void RdrSetRaymarchShadowStep(struct Renderer *renderer, double step);
extern void RdrSetRaymarchReflectStep(struct Renderer *renderer, double step);
extern void RdrSetRaymarchRefractStep(struct Renderer *renderer, double step);

extern void RdrSetCamera(struct Renderer *renderer, struct Camera *cam);
extern void RdrSetFrameBuffers(struct Renderer *renderer, struct FrameBuffer *fb);
extern void RdrSetTargetObjects(struct Renderer *renderer, struct ObjectGroup *grp);
extern void RdrSetTargetLights(struct Renderer *renderer, struct Light **lights, int nlights);

extern int RdrRender(struct Renderer *renderer);

extern void RdrSetFrameReportCallback(struct Renderer *renderer, void *data,
    FrameStartCallback frame_start,
    FrameDoneCallback frame_done);

extern void RdrSetTileReportCallback(struct Renderer *renderer, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */