// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SOCKET_H
#define FJ_SOCKET_H

#include "fj_compatibility.h"

#if defined(FJ_WINDOWS)
  // Windows
  #include <winsock2.h>
  typedef SOCKET socket_id;
  #define FJ_SHUTDOWN_READ       SD_RECEIVE
  #define FJ_SHUTDOWN_WRITE      SD_SEND
  #define FJ_SHUTDOWN_READ_WRITE SD_BOTH
  #define FJ_SOCKET_ERROR        SOCKET_ERROR
  #define FJ_INVALID_SOCKET      INVALID_SOCKET
  inline int fj_close_socket(int fd)
  {
    return closesocket(fd);
  }
  inline const char *fj_socket_error_message()
  {
    return WSAGetLastError();
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
  typedef int socket_id;
  #define FJ_SHUTDOWN_READ       SHUT_RD
  #define FJ_SHUTDOWN_WRITE      SHUT_WR
  #define FJ_SHUTDOWN_READ_WRITE SHUT_RDWR
  #define FJ_SOCKET_ERROR        -1
  #define FJ_INVALID_SOCKET      -1
  inline int fj_close_socket(int fd)
  {
    return close(fd);
  }
  inline const char *fj_socket_error_message()
  {
    return strerror(errno);
  }
#endif

#include <string>

enum SocketID {
  SOCKET_ID_INVALID = -1,
  SOCKET_ID_TIMEOUT = 0
};

namespace fj {

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
