/**
 * \file        client.cc
 *
 * \brief       Simple client example source
 *
 * \date        2021.04.27.
 *
 * \author      Sang Woo Ma <swma@markany.com>
 *
 * \copyright   MarkAny Inc. 2021.
 */

#include <cassert>
#include <cstdio>
#include <cstring>

#include "connection.h"
#include "xml.h"

class MyConnectionHandler : public ConnectionHandler {
 public:
  MyConnectionHandler(Connection& connection) : ConnectionHandler(connection) {}

  bool OnRead(const char* data, size_t bytes, size_t size) {
    assert(data && 0 < size && "Invalid read data");
    printf("data: %s\n", data);
    printf("  bytes: %zu\n", bytes);
    printf("  size: %zu\n", size);
    return true;
  }

  bool OnWrite(char* data, size_t& size) {
    assert(data && "Invalid write buffer");

    static const char src[] = "asdf";
    static const size_t src_size = sizeof(src) - 1;

    // first size is a maximum buffer size
    strncpy(data, "asdf", size);
    size = src_size;
    data[size] = '\0';

    printf("data: %s\n", data);
    printf("  size: %zu\n", size);
    return true;
  }
};

int main() {
  // Static target server information
  static constexpr const char kServerIp[] = "121.156.65.168";
  static constexpr unsigned short kServerPort = 40001;

  SslConnection conn;
  auto handler = std::make_unique<MyConnectionHandler>(conn);
  // if (!conn.ConnectOnce(kServerIp, kServerPort)) return EXIT_FAILURE;
  conn.SetHandler(std::move(handler));
  conn.Connect(kServerIp, kServerPort);
  conn.Write();

  Xml xml;
  if (!xml.LoadFromFile("load.xml")) return EXIT_FAILURE;
  xml.PrintAllNodes();

  xml.GenerateSample();
  xml.SaveToFile("generated.xml");

  printf("\n\nBye\n");
  return EXIT_SUCCESS;
}
