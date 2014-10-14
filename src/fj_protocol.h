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
  MSG_RENDER_FRAME_DONE
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
  int32_t x_tile_count;
  int32_t y_tile_count;

public:
};

int SendMessage(Socket &socket, const Message &message);

int SendRenderFrameStart(Socket &socket, int render_id,
    int xres, int yres, int channel_count,
    int x_tile_count, int y_tile_count);

int SendReply(Socket &socket, int render_id);

int RecieveMessage(Socket &socket, Message &message);

int RecieveRenderFrameStart(Socket &socket,
    int &render_id, int &xres, int &yres, int &channel_count,
    int &x_tile_count, int &y_tile_count);

} // namespace xxx

#endif // FJ_XXX_H
