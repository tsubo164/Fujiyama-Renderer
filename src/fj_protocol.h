// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_PROTOCOL_H
#define FJ_PROTOCOL_H

#include "fj_types.h"

namespace fj {

class Socket;

enum {
  MSG_NONE = 0,
  MSG_REPLY_OK,
  MSG_RENDER_FRAME_START,
  MSG_RENDER_FRAME_DONE,
  MSG_RENDER_TILE_START,
  MSG_RENDER_TILE_DONE
};

class Message {
public:
  Message();
  ~Message();

  int32_t type;
  int32_t render_id;
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

int SendRenderFrameStart(Socket &socket, int render_id,
    int xres, int yres, int channel_count, int tile_count);

int SendRenderFrameDone(Socket &socket, int render_id);

int SendRenderTileStart(Socket &socket, int render_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax);

int SendRenderTileDone(Socket &socket, int render_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax);

int SendReply(Socket &socket, int render_id);

int RecieveMessage(Socket &socket, Message &message);

int RecieveEOF(Socket &socket);

} // namespace xxx

#endif // FJ_XXX_H
