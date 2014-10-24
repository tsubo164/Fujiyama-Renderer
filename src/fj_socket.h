// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SOCKET_H
#define FJ_SOCKET_H

#include "fj_compatibility.h"

//TODO THIS SOMEWHERE ELSE e.g. fj_os.h
#if defined(FJ_WINDOWS)
  #include <winsock2.h>
  typedef SOCKET socket_id;
#else
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int socket_id;
  #define FJ_SHUTDOWN_READ       SHUT_RD
  #define FJ_SHUTDOWN_WRITE      SHUT_WR
  #define FJ_SHUTDOWN_READ_WRITE SHUT_RDWR
#endif

#include <string>

namespace fj {

class Socket {
public:
  Socket();
  ~Socket();

  int Open();
  bool IsOpen() const;
  void Close();
  void Shutdown();
  void ShutdownRead();
  void ShutdownWrite();

  int GetFileDescriptor() const;

  void SetAddress(const std::string &address);
  void SetPort(int port);

  int Connect();
  int Bind();
  void Listen();
  int Accept(Socket &accepted);
  int AcceptOrTimeout(Socket &accepted, int sec, int micro_sec);

  int Recieve(char *data, size_t count);
  int Send(const char *data, size_t count);

private:
  socket_id fd_;
  socklen_t len_;
  struct sockaddr_in address_;
};

} // namespace xxx

#endif // FJ_XXX_H
