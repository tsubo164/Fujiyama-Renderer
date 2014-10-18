// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_protocol.h"
#include "fj_socket.h"
#include <iostream>

namespace fj {

int SendRenderFrameStart(Socket &socket, int render_id,
    int xres, int yres, int channel_count, int tile_count)
{
  int32_t msg[7];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_RENDER_FRAME_START;
  msg[2] = render_id;
  msg[3] = xres;
  msg[4] = yres;
  msg[5] = channel_count;
  msg[6] = tile_count;

  socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  return 0;
}

int SendRenderFrameDone(Socket &socket, int render_id)
{
  int32_t msg[3];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_RENDER_FRAME_DONE;
  msg[2] = render_id;

  socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  return 0;
}

int SendRenderTileStart(Socket &socket, int render_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax)
{
  int32_t msg[8];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_RENDER_TILE_START;
  msg[2] = render_id;
  msg[3] = tile_id;
  msg[4] = xmin;
  msg[5] = ymin;
  msg[6] = xmax;
  msg[7] = ymax;

  const int err = socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  return err;
}

int SendReply(Socket &socket, int render_id)
{
  int32_t msg[3];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_REPLY_OK;
  msg[2] = render_id;

  int err = socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  return err;
}

Message::Message()
{
}

Message::~Message()
{
}

int SendMessage(Socket &socket, const Message &message)
{
  switch (message.type) {

  case MSG_RENDER_FRAME_START:
    int32_t msg[7];
    msg[0] = sizeof(msg) - sizeof(msg[0]);
    msg[1] = message.type;
    msg[2] = message.render_id;
    msg[3] = message.xres;
    msg[4] = message.yres;
    msg[5] = message.channel_count;
    msg[6] = message.tile_count;
    socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
    break;

  default:
    break;
  }

  return 0;
}

int RecieveMessage(Socket &socket, Message &message)
{
  int32_t body[32] = {0};

  int err = socket.Recieve(reinterpret_cast<char *>(&body[0]), sizeof(body[0]));
  if (err == -1) {
    return -1;
  }

  const int32_t size_of_msg = body[0];
  socket.Recieve(reinterpret_cast<char *>(&body[1]), size_of_msg);

  const int32_t type_of_msg = body[1];


  switch (type_of_msg) {

  case MSG_RENDER_FRAME_START:
    if (size_of_msg != 6 * sizeof(body[0])) {
      break;
    } else {
      message.type          = body[1];
      message.render_id     = body[2];
      message.xres          = body[3];
      message.yres          = body[4];
      message.channel_count = body[5];
      message.tile_count    = body[6];
    }
    break;

  case MSG_RENDER_FRAME_DONE:
    if (size_of_msg != 2 * sizeof(body[0])) {
      break;
    } else {
      message.type          = body[1];
      message.render_id     = body[2];
    }
    break;

  case MSG_RENDER_TILE_START:
    if (size_of_msg != 7 * sizeof(body[0])) {
      break;
    } else {
      message.type      = body[1];
      message.render_id = body[2];
      message.tile_id   = body[3];
      message.xmin      = body[4];
      message.ymin      = body[5];
      message.xmax      = body[6];
      message.ymax      = body[7];
    }
    break;

  case MSG_REPLY_OK:
    if (size_of_msg != 2 * sizeof(body[0])) {
      break;
    } else {
      message.type          = body[1];
      message.render_id     = body[2];
    }
    break;

  default:
    break;
  }

  return 0;
}

} // namespace xxx
