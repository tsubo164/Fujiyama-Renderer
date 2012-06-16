/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef RENDERER_H
#define RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

struct Renderer;
struct FrameBuffer;
struct ObjectGroup;
struct Camera;

extern struct Renderer *RdrNew(void);
extern void RdrFree(struct Renderer *renderer);

extern void RdrSetResolution(struct Renderer *renderer, int xres, int yres);
extern void RdrSetRenderRegion(struct Renderer *renderer, int xmin, int ymin, int xmax, int ymax);
extern void RdrSetPixelSamples(struct Renderer *renderer, int xrate, int yrate);
extern void RdrSetTileSize(struct Renderer *renderer, int xtilesize, int ytilesize);
extern void RdrSetFilterWidth(struct Renderer *renderer, float xfwidth, float yfwidth);
extern void RdrSetSampleJitter(struct Renderer *renderer, float jitter);

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

extern int RdrRender(struct Renderer *renderer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

