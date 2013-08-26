/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Callback.h"
#include <stddef.h>

static Interrupt no_frame_report(void *data, const struct FrameInfo *info)
{
  return CALLBACK_CONTINUE;
}

static Interrupt no_tile_report(void *data, const struct TileInfo *info)
{
  return CALLBACK_CONTINUE;
}
static Interrupt no_sample_report(void *data)
{
  return CALLBACK_CONTINUE;
}

Interrupt CbReportFrameStart(struct FrameReport *report, const struct FrameInfo *info)
{
  return report->start(report->data, info);
}

Interrupt CbReportFrameDone(struct FrameReport *report, const struct FrameInfo *info)
{
  return report->done(report->data, info);
}

Interrupt CbReportTileStart(struct TileReport *report, const struct TileInfo *info)
{
  return report->start(report->data, info);
}

Interrupt CbReportTileDone(struct TileReport *report, const struct TileInfo *info)
{
  return report->done(report->data, info);
}

Interrupt CbReportSampleDone(struct TileReport *report)
{
  return report->sample_done(report->data);
}

void CbSetFrameReport(struct FrameReport *report, void *data,
    FrameStartCallback frame_start,
    FrameDoneCallback frame_done)
{
  report->data = data;
  report->start = (frame_start == NULL) ? no_frame_report : frame_start;
  report->done  = (frame_done  == NULL) ? no_frame_report : frame_done;
}

void CbSetTileReport(struct TileReport *report, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done)
{
  report->data = data;
  report->start = (tile_start == NULL) ? no_tile_report : tile_start;
  report->done  = (tile_done  == NULL) ? no_tile_report : tile_done;
  report->sample_done  = (sample_done  == NULL) ? no_sample_report : sample_done;
}
