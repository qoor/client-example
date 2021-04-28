/**
 * \file        connection.cc
 *
 * \brief       Client connection interface
 *
 * \date        2021.04.27.
 *
 * \author      Sangwoo Ma <swma@markany.com>
 *
 * \copyright   MarkAny Inc. 2021.
 */
#include "connection.h"

bool Connection::Connect(const char* ip, unsigned short port) {
  assert(ip && "Invalid ip string pointer");
  if (!Init()) return false;

  // setting the target server address
  struct sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);
  addr.sin_family = PF_INET;
  memset(addr.sin_zero, 0x00, sizeof(addr.sin_zero));
  if (connect(GetFd(), reinterpret_cast<const struct sockaddr*>(&addr),
              sizeof(addr))) {
    printf("Failed to connect to the server (%s:%d)\n", ip, port);
    printf("Reason: (%d) %s\n", errno, strerror(errno));
    return false;
  }

  if (!ConnectInternal(ip, port)) {
    Close();
    return false;
  }
  return true;
}

bool SslConnection::Read() {
  if (!ssl_) return false;
  if (!handler_) {
    printf("Handler object is not registered.\n");
    return false;
  }

  char data[BUFSIZ];
  size_t bytes = SSL_read(ssl_, data, sizeof(data));
  if (0 >= bytes) {
    Close();
    printf("Server closed the connection.\n");
    return true;
  }
  return handler_->OnRead(data, bytes, sizeof(data));
}

bool SslConnection::Write() {
  if (!ssl_) return false;
  if (!handler_) {
    printf("Handler object is not registered.\n");
    return false;
  }

  char data[BUFSIZ];
  size_t size = sizeof(data);
  if (!handler_->OnWrite(data, size)) {
    printf("Send aborted.\n");
    return false;
  }
  if (0 >= SSL_write(ssl_, data, size)) {
    Close();
    return false;
  }
  return true;
}
