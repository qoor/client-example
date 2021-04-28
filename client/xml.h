/**
 * \file        xml.h
 *
 * \brief       Xml simple interface
 *
 * \date        2021.04.28.
 *
 * \author      Sangwoo Ma <swma@markany.com>
 *
 * \copyright   MarkAny Inc. 2021.
 */
#ifndef CLIENT_XML_H_
#define CLIENT_XML_H_

#include <tinyxml2.h>

#include <string>

// Xml wrapped class
// NOTE: TinyXML2 only support UTF-8, not UTF-16
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
    if (tinyxml2::XML_SUCCESS != error) {
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
    if (tinyxml2::XML_SUCCESS != doc_.Parse(data, size)) {
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
  bool GenerateSample();

  bool SaveToFile(const std::string& path) { return SaveToFile(path.c_str()); }
  bool SaveToFile(const char* path) { return doc_.SaveFile(path); }

 private:
  static constexpr int kIndentLength = 4;

  // Print all of childs recursively (recurse function)
  void PrintChildNodes(const tinyxml2::XMLElement* elem, int indent = 0) const;

  tinyxml2::XMLDocument doc_;
};

#endif
