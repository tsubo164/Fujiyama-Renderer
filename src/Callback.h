/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef CALLBACK_H
#define CALLBACK_H

#include "Rectangle.h"

#ifdef __cplusplus
extern "C" {
#endif

struct FrameInfo {
  int worker_count;
  int tile_count;
  struct Rectangle render_region;
};

struct TileInfo {
  int worker_id;
  int region_id;
  int total_region_count;
  int total_sample_count;
  struct Rectangle region;
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

#endif /* XXX_H */
