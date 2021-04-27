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
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <tinyxml2.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

// Uncomment to running network communicate
// #define COMMUNICATE_SERVER

using namespace tinyxml2;

inline static void PrintSpaces(int amount) {
  for (; 0 < amount; --amount) printf(" ");
}

class ConnectionHandler {
 public:
  // Packet read handler
  virtual bool OnRead(const char* data, size_t size) = 0;
  // Packet write handler
  virtual bool OnWrite(char* data, size_t& size) = 0;
};

class MyConnectionHandler : public ConnectionHandler {
 public:
  bool OnRead(const char* data, size_t size) {
    assert(data && 0 < size && "Invalid read data");
    printf("data: %s\n", data);
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

class Connection {
 public:
  // Socket RAII
  virtual ~Connection() { Close(); }

  // Server Connect Interface
  virtual bool Connect(const char* ip, unsigned short port) = 0;

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

  void Close() {
    if (0 <= fd_) {
      close(fd_);
      fd_ = -1;
    }
  }

  int GetFd() { return fd_; }

 private:
  int fd_ = -1;
};

class SslConnection : public Connection {
 public:
  SslConnection() { InitSslLibrary(); }
  SslConnection(std::unique_ptr<ConnectionHandler> handler)
      : handler_(std::move(handler)) {
    InitSslLibrary();
  }
  ~SslConnection() { Reset(); }

  bool Connect(const char* ip, unsigned short port) override {
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

    return ApplySsl();
  }

  // Connect and communication once and close the connection
  bool ConnectOnce(const char* ip, unsigned short port) {
    assert(ip && "Invalid ip string pointer");
    if (!Connect(ip, port)) return false;

// communication ...
#ifdef COMMUNICATE_SERVER
    Read();
    Write();
#endif

    Reset();
    Close();
    return true;
  }

 private:
  // Initialize OpenSSL library
  void InitSslLibrary() {
    SSL_library_init();
    SSL_load_error_strings();
  }

  // Reset all ssl connection informations
  void Reset() {
    if (ssl_) {
      SSL_free(ssl_);
      ssl_ = nullptr;
    }
    if (ssl_ctx_) {
      SSL_CTX_free(ssl_ctx_);
      ssl_ctx_ = nullptr;
    }
  }

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
      Reset();
      printf("Failed to create SSL peer.\n");
      return false;
    }

    SSL_set_fd(ssl_, GetFd());
    if (0 > SSL_connect(ssl_)) {
      Reset();
      printf("Failed to connect SSL.\n");
      return false;
    }

    return true;
  }

  // Receive incomming data from server
  bool Read() {
    if (!ssl_) return false;
    if (!handler_) {
      printf("Handler object is not registered.\n");
      return false;
    }

    char data[BUFSIZ];
    if (0 >= SSL_read(ssl_, data, sizeof(data))) {
      Reset();
      Close();
      printf("Server closed the connection.\n");
      return true;
    }
    return handler_->OnRead(data, sizeof(data));
  }

  // Send response to server
  bool Write() {
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
      Reset();
      Close();
      return false;
    }
    return true;
  }

  SSL* ssl_ = nullptr;
  SSL_CTX* ssl_ctx_ = nullptr;

  // Packet handler
  std::unique_ptr<ConnectionHandler> handler_;
};

class Xml {
 public:
  Xml() = default;
  explicit Xml(const std::string& file) { LoadFromFile(file.c_str()); }
  explicit Xml(const char* file) { LoadFromFile(file); }
  Xml(const char* data, size_t size) { LoadFromData(data, size); }

  // Load XML docuemtn from file path
  bool LoadFromFile(const char* file) {
    if (!file) {
      printf("Invalid file path pointer.\n");
      return false;
    }

    Reset();
    int error = doc_.LoadFile(file);
    if (XML_SUCCESS != error) {
      Reset();
      printf("Failed to load file: %s\n", file);
      return false;
    }
    return true;
  }

  // Load XML document from string
  bool LoadFromData(const char* data, size_t size) {
    if (!data || 0 >= size) {
      printf("Invalid XML data or data size\n");
      return false;
    }

    Reset();
    if (XML_SUCCESS != doc_.Parse(data, size)) {
      printf("Failed to parse XML data.\n");
      return false;
    }
    return true;
  }

  // Clear all of XML document data
  void Reset() { doc_.Clear(); }

  // Print all nodes (+ attributes) recursively
  void PrintAllNodes() const { PrintChildNodes(doc_.FirstChildElement()); }

  // Generate simple document using TinyXML2 APIs
  bool GenerateSample() {
    // Clear last document first
    Reset();

    // Do not release these resources because XML document want these
    XMLDeclaration* decl = doc_.NewDeclaration();
    XMLElement* map = doc_.NewElement("map");
    XMLElement* object = doc_.NewElement("object");
    if (!decl || !map || !object) return false;

    // <map> <-    attach "object" element to "map" element
    //         |
    //     <object ... />
    map->LinkEndChild(object);

    // (XML document)
    // <?xml ...?> <- add declaration
    // <map> <- add root element
    //     ... <- maybe one "object" element
    doc_.LinkEndChild(decl);
    doc_.LinkEndChild(map);
    return true;
  }

  bool SaveToFile(const std::string& path) { return SaveToFile(path.c_str()); }
  bool SaveToFile(const char* path) { return doc_.SaveFile(path); }

 private:
  static constexpr int kIndentLength = 4;

  // Print all of childs recursively (recurse function)
  void PrintChildNodes(const XMLElement* elem, int indent = 0) const {
    if (!elem) return;

    const char* name = elem->Name();
    const char* text = elem->GetText();

    // Open element visually
    PrintSpaces(indent);
    printf("<(element)%s", name);
    for (const auto* attr = elem->FirstAttribute(); attr; attr = attr->Next()) {
      printf(" (attribute)%s=%s", attr->Name(), attr->Value());
    }
    printf(">");

    if (!text) {
      // Depth-First Search for touch all childrens
      printf("\n");
      for (elem = elem->FirstChildElement(); elem;
           elem = elem->NextSiblingElement()) {
        PrintChildNodes(elem, indent + kIndentLength);
      }
      PrintSpaces(indent);
    } else {
      // Print element its value
      printf("(text value) %s", text);
    }

    // Close element visually
    printf("</(element)%s>\n", name);
  }

  XMLDocument doc_;
};

int main() {
  // Static target server information
  static constexpr const char kServerIp[] = "121.156.65.168";
  static constexpr unsigned short kServerPort = 40001;

  SslConnection conn(std::make_unique<MyConnectionHandler>());
  if (!conn.ConnectOnce(kServerIp, kServerPort)) return EXIT_FAILURE;

  Xml xml;
  if (!xml.LoadFromFile("load.xml")) return EXIT_FAILURE;
  xml.PrintAllNodes();

  xml.GenerateSample();
  xml.SaveToFile("generated.xml");

  printf("\n\nBye\n");
  return EXIT_SUCCESS;
}
