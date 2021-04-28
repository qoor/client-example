/**
 * \file        connection.h
 *
 * \brief       Client connection interface
 *
 * \date        2021.04.28.
 *
 * \author      Sangwoo Ma <swma@markany.com>
 *
 * \copyright   MarkAny Inc. 2021.
 */
#ifndef CLIENT_CONNECTION_H_
#define CLIENT_CONNECTION_H_

#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>

// Uncomment to running network communicate
// #define COMMUNICATE_SERVER

class ConnectionHandler;
class Connection {
 public:
  // RAII
  virtual ~Connection() { Close(); }

  // Server Connect Interface
  bool Connect(const char* ip, unsigned short port);

  void Close() {
    CloseInternal();

    if (0 <= fd_) {
      close(fd_);
      fd_ = -1;
    }
  }

 protected:
  // Create internal socket
  bool Init() {
    Close();

    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > fd_) {
      printf("Failed to create socket file descriptor.\n");
      return false;
    }
    return true;
  }

  int GetFd() { return fd_; }

 private:
  virtual bool ConnectInternal(const char*, unsigned short) { return true; }
  virtual void CloseInternal() {}

  int fd_ = -1;
};

class SslConnection : public Connection {
 public:
  // Only can connect/disconnect
  SslConnection() { InitSslLibrary(); }
  // connect/disconnect and I/O support
  SslConnection(std::unique_ptr<ConnectionHandler> handler)
      : handler_(std::move(handler)) {
    InitSslLibrary();
  }
  // RAII
  ~SslConnection() { ResetSsl(); }

  // Connect and communication once and close the connection
  bool ConnectOnce(const char* ip, unsigned short port) {
    assert(ip && "Invalid ip string pointer");
    if (!Connect(ip, port)) return false;

// communication ...
#ifdef COMMUNICATE_SERVER
    Read();
    Write();
#endif

    Close();
    return true;
  }

  // Receive incomming data from server
  bool Read();
  // Send response to server
  bool Write();

  void SetHandler(std::unique_ptr<ConnectionHandler> handler) {
    handler_ = std::move(handler);
  }

 private:
  // Initialize OpenSSL library
  void InitSslLibrary() {
    SSL_library_init();
    SSL_load_error_strings();
  }

  // Reset all ssl connection informations
  void ResetSsl() {
    if (ssl_) {
      SSL_free(ssl_);
      ssl_ = nullptr;
    }
    if (ssl_ctx_) {
      SSL_CTX_free(ssl_ctx_);
      ssl_ctx_ = nullptr;
    }
  }

  bool ConnectInternal(const char*, unsigned short) override {
    return ApplySsl();
  }

  void CloseInternal() override { ResetSsl(); }

  // Apply SSL connection to normal connection
  bool ApplySsl() {
    const SSL_METHOD* method = TLS_client_method();
    if (!method) {
      printf("This machine does not support any SSL features.\n");
      return false;
    }

    ssl_ctx_ = SSL_CTX_new(method);
    if (!ssl_ctx_) {
      printf("Failed to create SSL context object.\n");
      return false;
    }

    ssl_ = SSL_new(ssl_ctx_);
    if (!ssl_) {
      ResetSsl();
      printf("Failed to create SSL peer.\n");
      return false;
    }

    // Register SSL socket file descriptor
    SSL_set_fd(ssl_, GetFd());
    if (0 > SSL_connect(ssl_)) {
      ResetSsl();
      printf("Failed to connect SSL.\n");
      return false;
    }

    return true;
  }

  SSL* ssl_ = nullptr;
  SSL_CTX* ssl_ctx_ = nullptr;

  // Packet handler
  std::unique_ptr<ConnectionHandler> handler_;
};

class ConnectionHandler {
 public:
  virtual ~ConnectionHandler() = default;

  // Packet read handler
  virtual bool OnRead(const char* data, size_t bytes, size_t size) = 0;
  // Packet write handler
  virtual bool OnWrite(char* data, size_t& size) = 0;

 protected:
  ConnectionHandler(Connection& connection) : connection_(connection) {}

  Connection& GetConnection() { return connection_; }

 private:
  Connection& connection_;
};

#endif
