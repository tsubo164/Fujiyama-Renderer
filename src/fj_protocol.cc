// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_protocol.h"
#include "fj_socket.h"
#include "fj_color.h"
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

int SendRenderTileDone(Socket &socket, int render_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax,
    const FrameBuffer &tile)
{
  int32_t msg[9];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_RENDER_TILE_DONE;
  msg[2] = render_id;
  msg[3] = tile_id;
  msg[4] = xmin;
  msg[5] = ymin;
  msg[6] = xmax;
  msg[7] = ymax;
  msg[8] = tile.GetChannelCount();

  const int32_t tile_size =
      sizeof(float) *
      tile.GetWidth() *
      tile.GetHeight() *
      tile.GetChannelCount();
  msg[0] += tile_size;

  int err = socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  err = socket.Send(reinterpret_cast<const char *>(tile.GetReadOnly(0, 0, 0)), tile_size);

  return err;
}

Message::Message()
{
}

Message::~Message()
{
}

int RecieveMessage(Socket &socket, Message &message, FrameBuffer &tile)
{
  int32_t body[16] = {0};
  // TODO NEED THIS?  std::vector<int32_t> body(16);

  // reading size of message
  int err = socket.Recieve(reinterpret_cast<char *>(&body[0]), sizeof(body[0]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  // reading type of message
  err = socket.Recieve(reinterpret_cast<char *>(&body[1]), sizeof(body[1]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  const int32_t size_of_msg = body[0];
  const int32_t type_of_msg = body[1];

  // special case
  if (type_of_msg == MSG_RENDER_TILE_DONE) {
    err = socket.Recieve(reinterpret_cast<char *>(&body[2]), 7 * sizeof(body[0]));
    if (err == -1) {
      // TODO ERROR HANDLING
      return -1;
    }
    message.type      = body[1];
    message.render_id = body[2];
    message.tile_id   = body[3];
    message.xmin      = body[4];
    message.ymin      = body[5];
    message.xmax      = body[6];
    message.ymax      = body[7];
    message.channel_count = body[8];

    tile.Resize(
        message.xmax - message.xmin,
        message.ymax - message.ymin,
        message.channel_count);

    for (int y = 0; y < tile.GetHeight(); y++) {
      for (int x = 0; x < tile.GetWidth(); x++) {
        float pixel[4] = {0};
        err = socket.Recieve(reinterpret_cast<char *>(pixel), sizeof(pixel));
        if (err == -1) {
          // TODO ERROR HANDLING
        }
        const Color4 color(pixel[0], pixel[1], pixel[2], pixel[3]);
        tile.SetColor(x, y, color);
      }
    }

    return 0;
  }

  //body.resize(size_of_msg);
  err = socket.Recieve(reinterpret_cast<char *>(&body[2]), size_of_msg - sizeof(body[1]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

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

  case MSG_RENDER_TILE_DONE:
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

  default:
    break;
  }

  return 0;
}

int RecieveEOF(Socket &socket)
{
  char unused;
  const int s = socket.Recieve(&unused, sizeof(unused));
  if (s == 0) {
    return 0;
  } else {
    return -1;
  }
}

} // namespace xxx
