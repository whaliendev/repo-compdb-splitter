#ifndef _SPLITTER_TRIE_HPP_
#define _SPLITTER_TRIE_HPP_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>

#include "splitter/filesystem.h"

namespace splitter {
class Trie {
 public:
  Trie() : root_(new TrieNode()) {}

  Trie(Trie&& other) noexcept : root_{std::exchange(other.root_, nullptr)} {}

  Trie& operator=(Trie copy) {
    copy.swap(*this);
    return *this;
  }

  ~Trie() { deleteTrie(root_); };

  void Insert(const std::string& path) {
    std::string mut_path = path;

    std::replace(mut_path.begin(), mut_path.end(), '\\', '/');

    TrieNode* current = root_;

    fs::path dir_path = fs::path(mut_path);
    for (const auto& segment : dir_path) {
      const char* segmentStr = segment.c_str();
      if (current->children.find(segmentStr) == current->children.end()) {
        current->children[segmentStr] = new TrieNode();
      } else {
        // in case there are longer file path comes later
        current->children[segmentStr]->end = false;
      }
      current = current->children[segmentStr];
    }
    current->end = true;
  }

  std::string SearchPrefix(std::string_view path) const {
    fs::path prefix;

    TrieNode* current = root_;
    fs::path dir_path = fs::path(path);
    for (const auto& segment : dir_path) {
      const char* segmentStr = segment.c_str();
      if (current->children.find(segmentStr) == current->children.end()) {
        return prefix;
      }
      prefix = prefix / segmentStr;
      current = current->children[segmentStr];
    }
    return prefix;
  }

  bool Search(std::string_view path) const {
    TrieNode* current = root_;
    fs::path dir_path = fs::path(path);
    for (const auto& segment : dir_path) {
      const char* segmentStr = segment.c_str();
      if (current->children.find(segmentStr) == current->children.end()) {
        return false;
      }
      current = current->children[segmentStr];
    }
    return current->end;
  }

  std::string ToString() const {
    std::stringstream ss;
    ss << ".";
    toStringHelper(root_, "", ss);
    return ss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const Trie& trie) {
    os << trie.ToString();
    return os;
  }

  void swap(Trie& rhs) noexcept {
    using std::swap;
    swap(root_, rhs.root_);
  }

  friend void swap(Trie& a, Trie& b) noexcept { a.swap(b); }

 private:
  struct TrieNode {
    std::unordered_map<std::string, TrieNode*> children;
    bool end;

    TrieNode() : end(false) {}
  };

  void deleteTrie(TrieNode* node) {
    if (node == nullptr) return;
    for (auto& pair : node->children) {
      deleteTrie(pair.second);
    }
    delete node;
  }

  void toStringHelper(const TrieNode* cur, const std::string& indent,
                      std::stringstream& ss) const {
    if (cur == nullptr) return;

    ss << "\n";

    size_t index = 0;
    size_t child_cnt = cur->children.size();
    for (const auto& [segment, child] : cur->children) {
      ss << indent;
      bool last_child = index == child_cnt - 1;
      if (last_child) {
        ss << "└─ ";
      } else {
        ss << "├─ ";
      }
      ss << segment;
      if (!last_child) {
        toStringHelper(child, indent + "|   ", ss);
      } else {
        toStringHelper(child, indent + "    ", ss);
      }
      index++;
    }
  }

  TrieNode* root_;
};
}  // namespace splitter

#endif  // _SPLITTER_TRIE_HPP_