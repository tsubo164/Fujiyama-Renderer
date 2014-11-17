// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SOCKET_H
#define FJ_SOCKET_H

#include "fj_compatibility.h"
#include <string>

#if defined(FJ_WINDOWS)
  // Windows
  #include <winsock2.h>
  namespace fj {
    typedef SOCKET socket_id;
  }
#else
  // MacOSX Linux
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <cstring>
  #include <cerrno>
  namespace fj {
    typedef int socket_id;
  }
#endif

namespace fj {

enum SocketID {
  SOCKET_ID_INVALID = -1,
  SOCKET_ID_TIMEOUT = 0
};

int SocketStartup();
int SocketCleanup();
const char *SocketErrorMessage();

class Socket {
public:
  Socket();
  // Closes socket if opened
  ~Socket();

  socket_id Open();
  bool IsOpen() const;
  void Close();

  // Disconnections
  void Shutdown();
  void ShutdownRead();
  void ShutdownWrite();

  int GetFileDescriptor() const;

  int SetNoDelay();
  void SetAddress(const std::string &address);
  void SetPort(int port);

  // Connections
  int Connect();
  int Bind();
  int Listen();
  socket_id Accept(Socket &accepted);
  socket_id AcceptOrTimeout(Socket &accepted, int sec, int micro_sec);

  int Receive(char *data, size_t count);
  int Send(const char *data, size_t count);

private:
  socket_id fd_;
  socklen_t len_;
  struct sockaddr_in address_;
};

} // namespace xxx

#endif // FJ_XXX_H
