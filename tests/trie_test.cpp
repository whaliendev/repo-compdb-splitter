#include "splitter/trie.hpp"

#include <gtest/gtest.h>

#include <iostream>

// weak test
TEST(Trie, ToStringUnix) {
  std::vector<std::string> paths{"/usr/include",
                                 "/bin",
                                 "/usr/lib64",
                                 "/usr/bin/include/foo",
                                 "/usr/lib64/xorg",
                                 "/usr/lib64/Xorg/conf",
                                 "/bin/time",
                                 "/bin/gcc/g++",
                                 "/bin/clang/clang++/llvm-as",
                                 "/tmp"};

  /* clang-format off */
  std::string expected = R"(
.
└─ /
    ├─ tmp
    ├─ bin
    |   ├─ clang
    |   |   └─ clang++
    |   |       └─ llvm-as
    |   ├─ gcc
    |   |   └─ g++
    |   └─ time
    └─ usr
        ├─ bin
        |   └─ include
        |       └─ foo
        ├─ lib64
        |   ├─ Xorg
        |   |   └─ conf
        |   └─ xorg
        └─ include
)";
  /* clang-format on */

  splitter::Trie trie;
  for (const auto& path : paths) {
    trie.Insert(path);
  }
  ASSERT_EQ(expected, "\n" + trie.ToString());
}

TEST(Trie, ToStringWendows) {
  std::vector<std::string> win_paths{"C:\\Documents\\Newsletters",
                                     "C:\\Desktop",
                                     "D:\\Program Files",
                                     "D:\\Program Files(x86)",
                                     "D:\\Program Files\\Listary\\bin",
                                     "D:\\Program Files\\Steam\\It takes two",
                                     "C:\\3D Objects\\CAD\\crafted",
                                     "E:\\Data\\Opera\\Cache",
                                     "E:",
                                     "F:\\"};

  /* clang-format off */
  std::string win_expected = R"(
.
├─ F:
|   └─ 
├─ E:
|   └─ Data
|       └─ Opera
|           └─ Cache
├─ D:
|   ├─ Program Files(x86)
|   └─ Program Files
|       ├─ Steam
|       |   └─ It takes two
|       └─ Listary
|           └─ bin
└─ C:
    ├─ 3D Objects
    |   └─ CAD
    |       └─ crafted
    ├─ Desktop
    └─ Documents
        └─ Newsletters
)";
  /* clang-format on */

  splitter::Trie win_trie;
  std::for_each(win_paths.begin(), win_paths.end(),
                [&](const auto& path) { win_trie.Insert(path); });
  ASSERT_EQ(win_expected, "\n" + win_trie.ToString());
}

TEST(Trie, SearchPrefixWendows) {
  std::vector<std::string> win_paths{"C:\\Documents\\Newsletters",
                                     "C:\\Desktop",
                                     "D:\\Program Files",
                                     "D:\\Program Files(x86)",
                                     "D:\\Program Files\\Listary\\bin",
                                     "D:\\Program Files\\Steam\\It takes two",
                                     "C:\\3D Objects\\CAD\\crafted",
                                     "E:\\Data\\Opera\\Cache",
                                     "E:",
                                     "F:\\"};
  splitter::Trie trie;
  for (const auto& path : win_paths) {
    trie.Insert(path);
  }

  std::unordered_map<std::string, std::string> test_suite = {
      {"C:", "C:/Users"},
      {"", ""},
      {"", "/bin/gcc/g++/clang"},
      {"D:/Program Files", "D:/Program Files/Bandizip"},
      {"D:/Program Files", "D:/Program Files/Bandizip/bin"},
  };
  for (const auto& [expected, path] : test_suite) {
    ASSERT_EQ(expected, trie.SearchPrefix(path))
        << "expected: " << expected << ", path: " << path << "\n";
  }
}

TEST(Trie, SearchPrefixUnix) {
  std::vector<std::string> paths{"/usr/include",
                                 "/bin",
                                 "/usr/lib64",
                                 "/usr/bin/include/foo",
                                 "/usr/lib64/xorg",
                                 "/usr/lib64/Xorg/conf",
                                 "/bin/time",
                                 "/bin/gcc/g++",
                                 "/bin/clang/clang++/llvm-as",
                                 "/tmp"};
  splitter::Trie trie;
  for (const auto& path : paths) {
    trie.Insert(path);
  }

  std::unordered_map<std::string, std::string> test_suite = {
      {"/usr/lib64", "/usr/lib64/fmt"},
      {"/", "/root"},
      {"/bin/gcc/g++", "/bin/gcc/g++/clang"},
      {"/bin/clang/clang++/llvm-as", "/bin/clang/clang++/llvm-as/gcc"},
      {"/bin/clang/clang++/llvm-as", "/bin/clang/clang++/llvm-as/gcc/g++"},
      {"", ""},
  };
  for (const auto& [expected, path] : test_suite) {
    ASSERT_EQ(expected, trie.SearchPrefix(path));
  }
}

TEST(Trie, Search) {
  std::vector<std::string> paths{"/usr/include",
                                 "/bin",
                                 "/usr/lib64",
                                 "/usr/bin/include/foo",
                                 "/usr/lib64/xorg",
                                 "/usr/lib64/Xorg/conf",
                                 "/bin/time",
                                 "/bin/gcc/g++",
                                 "/bin/clang/clang++/llvm-as",
                                 "/tmp"};
  splitter::Trie trie;
  for (const auto& path : paths) {
    trie.Insert(path);
  }

  std::vector<std::pair<bool, std::string>> test_suite = {
      {false, "/usr/lib64/fmt"},
      {false, "/usr/lib64"},
      {true, "/usr/lib64/xorg"},
      {false, "/"},
      {false, ""},
  };

  for (const auto& [expected, path] : test_suite) {
    ASSERT_EQ(expected, trie.Search(path));
  }
}