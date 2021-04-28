#include "xml.h"

inline static void PrintSpaces(int amount) {
  for (; 0 < amount; --amount) printf(" ");
}

bool Xml::GenerateSample() {
  // Clear last document first
  Reset();

  // Do not release these resources because XML document want these
  tinyxml2::XMLDeclaration* decl = doc_.NewDeclaration();
  tinyxml2::XMLElement* map = doc_.NewElement("map");
  tinyxml2::XMLElement* object = doc_.NewElement("object");
  if (!decl || !map || !object) return false;

  // <map type="object" version="alpha">
  map->SetAttribute("type", "object");
  map->SetAttribute("version", "alpha");

  // <object>Hello, I'm a object</object>
  object->SetText("Hello, I'm a object.");

  // <map> <-    attach "object" element to "map" element
  //         |
  //     <object ... />
  map->LinkEndChild(object);

  // (XML document)
  // <?xml ...?> <- add declaration
  // <map> <- add root element "map"
  //     ... <- maybe one "object" element
  doc_.LinkEndChild(decl);
  doc_.LinkEndChild(map);
  return true;
}

void Xml::PrintChildNodes(const tinyxml2::XMLElement* elem, int indent) const {
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
