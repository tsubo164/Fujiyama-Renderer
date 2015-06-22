// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_callback.h"
#include <cstddef>

namespace fj {

static Interrupt no_frame_report(void *data, const FrameInfo *info)
{
  return CALLBACK_CONTINUE;
}

static Interrupt no_tile_report(void *data, const TileInfo *info)
{
  return CALLBACK_CONTINUE;
}
static Interrupt no_sample_report(void *data)
{
  return CALLBACK_CONTINUE;
}

Interrupt CbReportFrameStart(FrameReport *report, const FrameInfo *info)
{
  return report->start(report->data, info);
}

Interrupt CbReportFrameAbort(FrameReport *report, const FrameInfo *info)
{
  return report->abort(report->data, info);
}

Interrupt CbReportFrameDone(FrameReport *report, const FrameInfo *info)
{
  return report->done(report->data, info);
}

Interrupt CbReportTileStart(TileReport *report, const TileInfo *info)
{
  return report->start(report->data, info);
}

Interrupt CbReportTileDone(TileReport *report, const TileInfo *info)
{
  return report->done(report->data, info);
}

Interrupt CbReportSampleDone(TileReport *report)
{
  return report->sample_done(report->data);
}

void CbSetFrameReport(FrameReport *report, void *data,
    FrameStartCallback frame_start,
    FrameAbortCallback frame_abort,
    FrameDoneCallback frame_done)
{
  report->data = data;
  report->start = (frame_start == NULL) ? no_frame_report : frame_start;
  report->abort = (frame_abort == NULL) ? no_frame_report : frame_abort;
  report->done  = (frame_done  == NULL) ? no_frame_report : frame_done;
}

void CbSetTileReport(TileReport *report, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done)
{
  report->data = data;
  report->start = (tile_start == NULL) ? no_tile_report : tile_start;
  report->done  = (tile_done  == NULL) ? no_tile_report : tile_done;
  report->sample_done  = (sample_done  == NULL) ? no_sample_report : sample_done;
}

} // namespace xxx
