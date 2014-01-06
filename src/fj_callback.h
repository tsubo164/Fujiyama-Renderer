/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef FJ_CALLBACK_H
#define FJ_CALLBACK_H

#include "fj_rectangle.h"

#ifdef __cplusplus
extern "C" {
#endif

struct FrameInfo {
  int worker_count;
  int tile_count;
  struct Rectangle frame_region;
  
  const struct FrameBuffer *framebuffer;
};

struct TileInfo {
  int worker_id;
  int region_id;
  int total_region_count;
  int total_sample_count;
  struct Rectangle tile_region;

  const struct FrameBuffer *framebuffer;
};

enum {
  CALLBACK_CONTINUE = 0,
  CALLBACK_INTERRUPT = -1
};

typedef int Interrupt;
typedef Interrupt (*FrameStartCallback)(void *data, const struct FrameInfo *info);
typedef Interrupt (*FrameDoneCallback)(void *data, const struct FrameInfo *info);

typedef Interrupt (*TileStartCallback)(void *data, const struct TileInfo *info);
typedef Interrupt (*TileDoneCallback)(void *data, const struct TileInfo *info);

typedef Interrupt (*SampleDoneCallback)(void *data);

struct FrameReport {
  void *data;
  FrameStartCallback start;
  FrameDoneCallback done;
};

struct TileReport {
  void *data;
  TileStartCallback start;
  SampleDoneCallback sample_done;
  TileDoneCallback done;
};

extern Interrupt CbReportFrameStart(struct FrameReport *report, const struct FrameInfo *info);
extern Interrupt CbReportFrameDone(struct FrameReport *report, const struct FrameInfo *info);

extern Interrupt CbReportTileStart(struct TileReport *report, const struct TileInfo *info);
extern Interrupt CbReportTileDone(struct TileReport *report, const struct TileInfo *info);

extern Interrupt CbReportSampleDone(struct TileReport *report);

extern void CbSetFrameReport(struct FrameReport *report, void *data,
    FrameStartCallback frame_start,
    FrameDoneCallback frame_done);

extern void CbSetTileReport(struct TileReport *report, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FJ_XXX_H */
