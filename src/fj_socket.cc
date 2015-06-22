// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_socket.h"
#include <cstring>

namespace fj {

#if defined(FJ_WINDOWS)
  // Windows
  enum {
    FJ_SHUTDOWN_READ        = SD_RECEIVE,
    FJ_SHUTDOWN_WRITE       = SD_SEND,
    FJ_SHUTDOWN_READ_WRITE  = SD_BOTH
  };
  inline int close_socket(int fd)
  {
    return closesocket(fd);
  }
  inline const char *socket_error_message()
  {
    static std::string message;

    LPVOID lpMsgBuf = NULL;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        WSAGetLastError(),
        0, // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
    );
    // Process any inserts in lpMsgBuf.
    message = (LPCTSTR) lpMsgBuf;
    // Free the buffer.
    LocalFree( lpMsgBuf );

    return message.c_str();
  }
  inline int socket_start_up()
  {
    WSADATA wsad;
    return WSAStartup(MAKEWORD(2, 0), &wsad);
  }
  inline int socket_clean_up()
  {
    return WSACleanup();
  }
#else
  // MacOSX Linux
  enum {
    FJ_SHUTDOWN_READ       = SHUT_RD,
    FJ_SHUTDOWN_WRITE      = SHUT_WR,
    FJ_SHUTDOWN_READ_WRITE = SHUT_RDWR
  };
  inline int close_socket(int fd)
  {
    return close(fd);
  }
  inline const char *socket_error_message()
  {
    return strerror(errno);
  }
  inline int socket_start_up()
  {
    return 0;
  }
  inline int socket_clean_up()
  {
    return 0;
  }
#endif

const int DEFAULT_PORT = 50505;

inline static struct sockaddr *get_sockaddr(struct sockaddr_in * addr_in)
{
  return reinterpret_cast<struct sockaddr *>(addr_in);
}

int SocketStartup()
{
  return socket_start_up();
}

int SocketCleanup()
{
  return socket_clean_up();
}

const char *SocketErrorMessage()
{
  return socket_error_message();
}

Socket::Socket() :
    fd_(FJ_SOCKET_INVALID), len_(sizeof(address_)), address_()
{
}

Socket::~Socket()
{
  Close();
}

socket_id Socket::Open()
{
  if (IsOpen()) {
    return fd_;
  }

  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ == FJ_SOCKET_INVALID) {
    return FJ_SOCKET_INVALID;
  }

  address_.sin_family = AF_INET;
  SetAddress("");
  SetPort(DEFAULT_PORT);

  return fd_;
}

bool Socket::IsOpen() const
{
  return fd_ != FJ_SOCKET_INVALID; // stderr
}

void Socket::Close()
{
  if (IsOpen()) {
    close_socket(fd_);
    fd_ = FJ_SOCKET_INVALID;
    std::memset(&address_, 0, sizeof(address_));
  }
}

void Socket::Shutdown()
{
  if (IsOpen()) {
    shutdown(fd_, FJ_SHUTDOWN_READ_WRITE);
  }
}

void Socket::ShutdownRead()
{
  if (IsOpen()) {
    shutdown(fd_, FJ_SHUTDOWN_READ);
  }
}

void Socket::ShutdownWrite()
{
  if (IsOpen()) {
    shutdown(fd_, FJ_SHUTDOWN_WRITE);
  }
}

int Socket::GetFileDescriptor() const
{
  return fd_;
}

int Socket::EnableNoDelay()
{
  int enable = 1;
  const int result = setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
      reinterpret_cast<char *>(&enable), sizeof(enable));

  if (result == FJ_SOCKET_ERROR) {
    return -1;
  }

  return 0;
}

int Socket::EnableReuseAddr()
{
  int enable = 1;
  const int result = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
      reinterpret_cast<char *>(&enable), sizeof(enable));

  if (result == FJ_SOCKET_ERROR) {
    return -1;
  }

  return 0;
}

void Socket::SetAddress(const std::string &address)
{
  if (address == "") {
    address_.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    address_.sin_addr.s_addr = inet_addr(address.c_str());
  }
}

void Socket::SetPort(int port)
{
  address_.sin_port = htons(port);
}

int Socket::Connect()
{
  const int fd = Open();
  if (fd == FJ_SOCKET_INVALID) {
    return -1;
  }

  len_ = sizeof(address_);
  const int result = connect(fd_, get_sockaddr(&address_), len_);
  if (result == FJ_SOCKET_ERROR) {
    return -1;
  }

  return fd_;
}

int Socket::Bind()
{
  const int fd = Open();
  if (fd == FJ_SOCKET_INVALID) {
    return -1;
  }

  len_ = sizeof(address_);
  const int result = bind(fd_, get_sockaddr(&address_), len_);
  if (result == FJ_SOCKET_ERROR) {
    return -1;
  }

  return fd_;
}

int Socket::Listen()
{
  const int result = listen(fd_, SOMAXCONN);
  if (result == FJ_SOCKET_ERROR) {
    return -1;
  }

  return 0;
}

socket_id Socket::Accept(Socket &accepted)
{
  accepted.fd_ = accept(fd_, get_sockaddr(&accepted.address_), &accepted.len_);
  if (accepted.fd_ == FJ_SOCKET_INVALID) {
    return FJ_SOCKET_INVALID;
  }

  return accepted.fd_;
}

socket_id Socket::AcceptOrTimeout(Socket &accepted, int sec, int micro_sec)
{
  struct timeval timeout;
  timeout.tv_sec = sec;
  timeout.tv_usec = micro_sec;

  fd_set read_mask;
  FD_ZERO(&read_mask);
  FD_SET(fd_, &read_mask);

  const int result = select(fd_ + 1, &read_mask, NULL, NULL, &timeout);
  if (result == FJ_SOCKET_ERROR) {
    // error
    return FJ_SOCKET_INVALID;
  }
  else if (result == 0) {
    // time out
    return FJ_SOCKET_TIMEOUT;
  }

  if (FD_ISSET(fd_, &read_mask)) {
    return Accept(accepted);
  } else {
    return FJ_SOCKET_INVALID;
  }
}

int Socket::Receive(char *data, size_t count)
{
  return recv(fd_, data, count, MSG_WAITALL);
}

int Socket::Send(const char *data, size_t count)
{
  return send(fd_, data, count, 0);
}

} // namespace xxx
