#ifndef _SP_XML_TRIE_HPP_
#define _SP_XML_TRIE_HPP_

#include <cstring>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include "splitter/trie.hpp"

namespace splitter {
static Trie BuildFromXMLs(const std::vector<std::string>& manifest_files) {
  Trie trie;
  for (const auto& manifest_file : manifest_files) {
    rapidxml::file<> fdoc(manifest_file.c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(fdoc.data());

    rapidxml::xml_node<>* root = doc.first_node("manifest");

    for (rapidxml::xml_node<>* node = root->first_node("project"); node;
         node = node->next_sibling("project")) {
      rapidxml::xml_attribute<>* path_attr = node->first_attribute("path");
      if (::strlen(path_attr->value())) {
        trie.Insert(path_attr->value());
      }
    }
  }
  return trie;
}

}  // namespace splitter

#endif