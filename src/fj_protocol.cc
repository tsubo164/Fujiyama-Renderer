// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_protocol.h"
#include "fj_socket.h"
#include "fj_color.h"
#include <iostream>

#define CONVERT_MSG_RENDER_FRAME_START(CONV) do { \
  CONV(0, size          ) \
  CONV(1, type          ) \
  CONV(2, frame_id      ) \
  CONV(3, xres          ) \
  CONV(4, yres          ) \
  CONV(5, channel_count ) \
  CONV(6, tile_count    ) \
  } while(0)
#define SIZEOF_RENDER_FRAME_START \
       7

#define CONVERT_MSG_RENDER_FRAME_DONE(CONV) \
  CONV(0, size          ) \
  CONV(1, type          ) \
  CONV(2, frame_id      )
#define SIZEOF_RENDER_FRAME_DONE \
       3

#define CONVERT_MSG_RENDER_TILE_START(CONV) do { \
  CONV(0, size          ) \
  CONV(1, type          ) \
  CONV(2, frame_id      ) \
  CONV(3, tile_id       ) \
  CONV(4, xmin          ) \
  CONV(5, ymin          ) \
  CONV(6, xmax          ) \
  CONV(7, ymax          ) \
  } while(0)
#define SIZEOF_RENDER_TILE_START \
       8

#define CONVERT_MSG_RENDER_TILE_DONE(CONV) do { \
  CONV(0, size          ) \
  CONV(1, type          ) \
  CONV(2, frame_id      ) \
  CONV(3, tile_id       ) \
  CONV(4, xmin          ) \
  CONV(5, ymin          ) \
  CONV(6, xmax          ) \
  CONV(7, ymax          ) \
  CONV(8, channel_count ) \
  } while(0)
#define SIZEOF_RENDER_TILE_DONE \
       9

#define CONVERT_MSG_RENDER_FRAME_ABORT(CONV) \
  CONV(0, size          ) \
  CONV(1, type          ) \
  CONV(2, frame_id      )
#define SIZEOF_RENDER_FRAME_ABORT \
       3

#define MSG_TO_ARRAY(i,name) array[i] = name;
#define ARRAY_TO_MSG(i,name) message.name = body[i];

namespace fj {

int SendRenderFrameStart(Socket &socket, int32_t frame_id,
    int xres, int yres, int channel_count, int tile_count)
{
  int32_t array[SIZEOF_RENDER_FRAME_START];
  const int32_t size = sizeof(array) - sizeof(array[0]);
  const int32_t type = MSG_RENDER_FRAME_START;
  CONVERT_MSG_RENDER_FRAME_START(MSG_TO_ARRAY);
  socket.Send(reinterpret_cast<char *>(array), sizeof(array));
  return 0;
}

int SendRenderFrameDone(Socket &socket, int32_t frame_id)
{
  int32_t array[SIZEOF_RENDER_FRAME_DONE];
  const int32_t size = sizeof(array) - sizeof(array[0]);
  const int32_t type = MSG_RENDER_FRAME_DONE;
  CONVERT_MSG_RENDER_FRAME_DONE(MSG_TO_ARRAY)
  socket.Send(reinterpret_cast<char *>(array), sizeof(array));
  return 0;
}

int SendRenderFrameAbort(Socket &socket, int32_t frame_id)
{
  int32_t array[SIZEOF_RENDER_FRAME_ABORT];
  const int32_t size = sizeof(array) - sizeof(array[0]);
  const int32_t type = MSG_RENDER_FRAME_ABORT;
  CONVERT_MSG_RENDER_FRAME_ABORT(MSG_TO_ARRAY)
  socket.Send(reinterpret_cast<char *>(array), sizeof(array));
  return 0;
}

int SendRenderTileStart(Socket &socket, int32_t frame_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax)
{
  int32_t array[SIZEOF_RENDER_TILE_START];
  const int32_t size = sizeof(array) - sizeof(array[0]);
  const int32_t type = MSG_RENDER_TILE_START;
  CONVERT_MSG_RENDER_TILE_START(MSG_TO_ARRAY);
  socket.Send(reinterpret_cast<char *>(array), sizeof(array));
  return 0;
}

int SendRenderTileDone(Socket &socket, int32_t frame_id,
    int tile_id, int xmin, int ymin, int xmax, int ymax,
    const FrameBuffer &tile)
{
  const int32_t tile_size =
      sizeof(float) *
      tile.GetWidth() *
      tile.GetHeight() *
      tile.GetChannelCount();
  const int32_t channel_count = tile.GetChannelCount();

  int32_t array[SIZEOF_RENDER_TILE_DONE];
  const int32_t size = sizeof(array) - sizeof(array[0]) + tile_size;
  const int32_t type = MSG_RENDER_TILE_DONE;
  CONVERT_MSG_RENDER_TILE_DONE(MSG_TO_ARRAY);
  int err = socket.Send(reinterpret_cast<char *>(array), sizeof(array));
  err = socket.Send(reinterpret_cast<const char *>(tile.GetReadOnly(0, 0, 0)), tile_size);
  return err;
}

Message::Message()
{
}

Message::~Message()
{
}

int ReceiveMessage(Socket &socket, Message &message, FrameBuffer &tile)
{
  int32_t body[16] = {0};

  // reading size of message
  int err = socket.Receive(reinterpret_cast<char *>(&body[0]), sizeof(body[0]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  if (err == 0) {
    return -1;
  }

  // reading type of message
  err = socket.Receive(reinterpret_cast<char *>(&body[1]), sizeof(body[1]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  const int32_t size_of_msg = body[0];
  const int32_t type_of_msg = body[1];

  // special case
  if (type_of_msg == MSG_RENDER_TILE_DONE) {
    err = socket.Receive(reinterpret_cast<char *>(&body[2]),
        (SIZEOF_RENDER_TILE_DONE - 2) * sizeof(body[0]));
    if (err == -1) {
      // TODO ERROR HANDLING
      return -1;
    }
    CONVERT_MSG_RENDER_TILE_DONE(ARRAY_TO_MSG);

    tile.Resize(
        message.xmax - message.xmin,
        message.ymax - message.ymin,
        message.channel_count);

    for (int y = 0; y < tile.GetHeight(); y++) {
      for (int x = 0; x < tile.GetWidth(); x++) {
        float pixel[4] = {0};
        err = socket.Receive(reinterpret_cast<char *>(pixel), sizeof(pixel));
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
  err = socket.Receive(reinterpret_cast<char *>(&body[2]), size_of_msg - sizeof(body[1]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  switch (type_of_msg) {

  case MSG_RENDER_FRAME_START:
    if (size_of_msg != (SIZEOF_RENDER_FRAME_START - 1) * sizeof(body[0])) {
      break;
    } else {
      CONVERT_MSG_RENDER_FRAME_START(ARRAY_TO_MSG);
    }
    break;

  case MSG_RENDER_FRAME_DONE:
    if (size_of_msg != (SIZEOF_RENDER_FRAME_DONE - 1) * sizeof(body[0])) {
      break;
    } else {
      CONVERT_MSG_RENDER_FRAME_DONE(ARRAY_TO_MSG);
    }
    break;

  case MSG_RENDER_TILE_START:
    if (size_of_msg != (SIZEOF_RENDER_TILE_START - 1) * sizeof(body[0])) {
      break;
    } else {
      CONVERT_MSG_RENDER_TILE_START(ARRAY_TO_MSG);
    }
    break;

  case MSG_RENDER_TILE_DONE:
    if (size_of_msg != (SIZEOF_RENDER_TILE_DONE - 1) * sizeof(body[0])) {
      break;
    } else {
      CONVERT_MSG_RENDER_TILE_DONE(ARRAY_TO_MSG);
    }
    break;

  default:
    break;
  }

  return 0;
}

int ReceiveEOF(Socket &socket)
{
  char unused;
  const int s = socket.Receive(&unused, sizeof(unused));
  if (s == 0) {
    return 0;
  } else {
    return -1;
  }
}

int ReceiveReply(Socket &socket, Message &message)
{
  int32_t body[16] = {0};

  // reading size of message
  int err = socket.Receive(reinterpret_cast<char *>(&body[0]), sizeof(body[0]));
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  if (err == 0) {
    return -1;
  }

  const int32_t size_of_msg = body[0];

  // reading type of message
  err = socket.Receive(reinterpret_cast<char *>(&body[1]), size_of_msg);
  if (err == -1) {
    // TODO ERROR HANDLING
    return -1;
  }

  const int32_t type_of_msg = body[1];

  switch (type_of_msg) {

  case MSG_RENDER_FRAME_ABORT:
    if (size_of_msg != (SIZEOF_RENDER_FRAME_ABORT - 1) * sizeof(body[0])) {
      break;
    } else {
      CONVERT_MSG_RENDER_FRAME_ABORT(ARRAY_TO_MSG);
    }
    break;
  default:
    break;
  }

  return 0;
}

} // namespace xxx
