// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_protocol.h"
#include "fj_socket.h"
#include <iostream>

namespace fj {

int SendRenderFrameStart(Socket &socket,
    int render_id, int xres, int yres, int channel_count,
    int x_tile_count, int y_tile_count)
{
  int32_t msg[8];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_RENDER_FRAME_START;
  msg[2] = render_id;
  msg[3] = xres;
  msg[4] = yres;
  msg[5] = channel_count;
  msg[6] = x_tile_count;
  msg[7] = y_tile_count;

  socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  return 0;
}

int SendReply(Socket &socket, int render_id)
{
  int32_t msg[3];

  msg[0] = sizeof(msg) - sizeof(msg[0]);
  msg[1] = MSG_REPLY_OK;
  msg[2] = render_id;

  socket.Send(reinterpret_cast<char *>(msg), sizeof(msg));
  return 0;
}

int RecieveRenderFrameStart(Socket &socket,
    int &render_id, int &xres, int &yres, int &channel_count,
    int &x_tile_count, int &y_tile_count)
{
  int32_t size_of_msg = 0;

  socket.Recieve(reinterpret_cast<char *>(size_of_msg), sizeof(size_of_msg));

  return 0;
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
    int32_t msg[8];
    msg[0] = sizeof(msg) - sizeof(msg[0]);
    msg[1] = message.type;
    msg[2] = message.render_id;
    msg[3] = message.xres;
    msg[4] = message.yres;
    msg[5] = message.channel_count;
    msg[6] = message.x_tile_count;
    msg[7] = message.y_tile_count;
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

  socket.Recieve(reinterpret_cast<char *>(&body[0]), sizeof(body[0]));

  const int32_t size_of_msg = body[0];
  socket.Recieve(reinterpret_cast<char *>(&body[1]), size_of_msg);

  const int32_t type_of_msg = body[1];


  switch (type_of_msg) {

  case MSG_RENDER_FRAME_START:
    if (size_of_msg != 7 * sizeof(body[0])) {
      break;
    } else {
      message.type          = body[1];
      message.render_id     = body[2]; 
      message.xres          = body[3]; 
      message.yres          = body[4]; 
      message.channel_count = body[5]; 
      message.x_tile_count  = body[6]; 
      message.y_tile_count  = body[7]; 
    }
    break;

  default:
    break;
  }

  return 0;
}

} // namespace xxx
