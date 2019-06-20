// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROTOCOL_H
#define FJ_PROTOCOL_H

#include "fj_framebuffer.h"
#include "fj_types.h"
#include <vector>

namespace fj {

class Socket;

enum {
  MSG_NONE = 0,
  MSG_RENDER_FRAME_START,
  MSG_RENDER_FRAME_DONE,
  MSG_RENDER_FRAME_ABORT,
  MSG_RENDER_TILE_START,
  MSG_RENDER_TILE_DONE
};

class FJ_API Message {
public:
  Message();
  ~Message();

  int32_t size;
  int32_t type;
  int32_t frame_id;
  int32_t xres;
  int32_t yres;
  int32_t channel_count;
  int32_t tile_count;

  int32_t tile_id;
  int32_t xmin;
  int32_t ymin;
  int32_t xmax;
  int32_t ymax;

public:
};

FJ_API int SendRenderFrameStart(Socket &socket, int32_t frame_id,
    int xres, int yres, int channel_count, int tile_count);

FJ_API int SendRenderFrameDone(Socket &socket, int32_t frame_id);

FJ_API int SendRenderFrameAbort(Socket &socket, int32_t frame_id);

FJ_API int SendRenderTileStart(Socket &socket, int32_t frame_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax);

FJ_API int SendRenderTileDone(Socket &socket, int32_t frame_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax,
    const FrameBuffer &tile);

FJ_API int ReceiveMessage(Socket &socket, Message &message, FrameBuffer &tile);
FJ_API int ReceiveEOF(Socket &socket);

FJ_API int ReceiveReply(Socket &socket, Message &message);

} // namespace xxx

#endif // FJ_XXX_H
