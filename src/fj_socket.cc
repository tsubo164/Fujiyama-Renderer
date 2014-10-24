// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_socket.h"

namespace fj {

const int DEFAULT_PORT = 50505;

inline static struct sockaddr *get_sockaddr(struct sockaddr_in * addr_in)
{
  return reinterpret_cast<struct sockaddr *>(addr_in);
}

Socket::Socket() :
    fd_(-1), len_(sizeof(address_)), address_()
{
}

Socket::~Socket()
{
  Close();
}

int Socket::Open()
{
  if (IsOpen()) {
    return 0;
  }

  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  address_.sin_family = AF_INET;
  SetAddress("");
  SetPort(DEFAULT_PORT);

  return 0;
}

bool Socket::IsOpen() const
{
  return fd_ > 2; // stderr
}

void Socket::Close()
{
  if (IsOpen()) {
    close(fd_);
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
  Open();

  len_ = sizeof(address_);
  const int err = connect(fd_, get_sockaddr(&address_), len_);
  // TODO error handling
  return err;
}

int Socket::Bind()
{
  Open();

  len_ = sizeof(address_);
  const int err = bind(fd_, get_sockaddr(&address_), len_);
  // TODO error handling
  return err;
}

void Socket::Listen()
{
  listen(fd_, SOMAXCONN);
}

int Socket::Accept(Socket &accepted)
{
  accepted.fd_ = accept(fd_, get_sockaddr(&accepted.address_), &accepted.len_);
  return 0;
}

int Socket::AcceptOrTimeout(Socket &accepted, int sec, int micro_sec)
{
  struct timeval timeout;
  timeout.tv_sec = sec;
  timeout.tv_usec = micro_sec;

  fd_set read_mask;
  FD_ZERO(&read_mask);
  FD_SET(fd_, &read_mask);

  const int result = select(fd_ + 1, &read_mask, NULL, NULL, &timeout);
  if (result == -1) {
    // error
    return -1;
  }
  else if (result == 0) {
    // time out
    return 0;
  }

  if (FD_ISSET(fd_, &read_mask)) {
    Accept(accepted);
    return fd_;
  } else {
    // shouldn't be here
    return -1;
  }
}

int Socket::Recieve(char *data, size_t count)
{
  return recv(fd_, data, count, MSG_WAITALL);
}

int Socket::Send(const char *data, size_t count)
{
  return send(fd_, data, count, 0);
}

} // namespace xxx
