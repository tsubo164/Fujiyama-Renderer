// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_CALLBACK_H
#define FJ_CALLBACK_H

#include "fj_rectangle.h"
#include <cstddef>

namespace fj {

class FrameBuffer;

class FrameInfo {
public:
  FrameInfo() :
      worker_count(0),
      tile_count(0),
      xres(0),
      yres(0),
      frame_region(),
      framebuffer(NULL)
  {}
  ~FrameInfo() {}

public:
  int worker_count;
  int tile_count;
  int xres;
  int yres;
  Rectangle frame_region;
  
  const FrameBuffer *framebuffer;
};

class TileInfo {
public:
  TileInfo() :
      worker_id(0),
      region_id(0),
      total_region_count(0),
      total_sample_count(0),
      tile_region(),
      framebuffer(NULL)
  {}
  ~TileInfo() {}

public:
  int worker_id;
  int region_id;
  int total_region_count;
  int total_sample_count;
  Rectangle tile_region;

  const FrameBuffer *framebuffer;
};

enum {
  CALLBACK_CONTINUE = 0,
  CALLBACK_INTERRUPT = -1
};

typedef int Interrupt;
typedef Interrupt (*FrameStartCallback)(void *data, const FrameInfo *info);
typedef Interrupt (*FrameDoneCallback)(void *data, const FrameInfo *info);

typedef Interrupt (*TileStartCallback)(void *data, const TileInfo *info);
typedef Interrupt (*TileDoneCallback)(void *data, const TileInfo *info);

typedef Interrupt (*SampleDoneCallback)(void *data);

class FrameReport {
public:
  FrameReport() : data(NULL), start(NULL), done(NULL) {}
  ~FrameReport() {}

public:
  void *data;
  FrameStartCallback start;
  FrameDoneCallback done;
};

class TileReport {
public:
  TileReport() : data(NULL), start(NULL), sample_done(NULL), done(NULL) {}
  ~TileReport() {}

public:
  void *data;
  TileStartCallback start;
  SampleDoneCallback sample_done;
  TileDoneCallback done;
};

extern Interrupt CbReportFrameStart(FrameReport *report, const FrameInfo *info);
extern Interrupt CbReportFrameDone(FrameReport *report, const FrameInfo *info);

extern Interrupt CbReportTileStart(TileReport *report, const TileInfo *info);
extern Interrupt CbReportTileDone(TileReport *report, const TileInfo *info);

extern Interrupt CbReportSampleDone(TileReport *report);

extern void CbSetFrameReport(FrameReport *report, void *data,
    FrameStartCallback frame_start,
    FrameDoneCallback frame_done);

extern void CbSetTileReport(TileReport *report, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done);

} // namespace xxx

#endif // FJ_XXX_H
