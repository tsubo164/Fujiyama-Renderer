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
    enum {
      FJ_SOCKET_TIMEOUT = 0,
      FJ_SOCKET_ERROR   = SOCKET_ERROR,
      FJ_SOCKET_INVALID = INVALID_SOCKET
    };
    typedef SOCKET socket_id;
    typedef int socket_length;
  }
  #ifdef max
    #undef max
  #endif
  #ifdef min
    #undef min
  #endif
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
    enum {
      FJ_SOCKET_TIMEOUT = 0,
      FJ_SOCKET_ERROR   = -1,
      FJ_SOCKET_INVALID = -1
    };
    typedef int socket_id;
    typedef socklen_t socket_length;
  }
#endif

namespace fj {

FJ_API int SocketStartup();
FJ_API int SocketCleanup();
FJ_API const char *SocketErrorMessage();

class FJ_API Socket {
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
  socket_length len_;
  struct sockaddr_in address_;
};

} // namespace xxx

#endif // FJ_XXX_H
