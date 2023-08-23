#ifndef _SPLITTER_UTIL_HPP_
#define _SPLITTER_UTIL_HPP_

#include <chrono>
#include <cstdio>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#ifdef _WIN32
#include <cassert>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace splitter {

template <typename Func, typename... Args>
static auto MeasureRunningTime(Func&& func, Args&&... args) {
  // Check if the function is invocable with the given arguments
  static_assert(std::is_invocable_v<Func, Args...>,
                "Func is not invocable with the provided arguments");

  using FuncReturnType = std::invoke_result_t<Func, Args...>;

  auto start = std::chrono::steady_clock::now();
  if constexpr (std::is_same_v<FuncReturnType, void>) {
    std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    auto end = std::chrono::steady_clock::now();
    long elapsed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    return elapsed_time;
  } else {
    FuncReturnType result =
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    auto end = std::chrono::steady_clock::now();
    long elapsed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    return std::tuple<long, FuncReturnType>(elapsed_time, result);
  }
}

static bool EndsWith(std::string_view sv, const std::string_view suffix) {
  if (sv.length() < suffix.length()) {
    return false;
  }
  return sv.compare(sv.length() - suffix.length(), suffix.length(), suffix) ==
         0;
}

/// Read the entire file into a string. It's the caller's responsibility to
/// free the returned string.
/// @param file_path The path to the file to read.
/// @return The contents of the file, or nullptr if there was an error.
[[maybe_unused]] static char* ReadLargeFile(const std::string& file_path) {
  FILE* fp = fopen(file_path.c_str(), "rb");
  if (!fp) {
    fprintf(stderr, "error opening file: %s\n", file_path.c_str());
    return nullptr;
  }

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "error allocating memory for file: %s\n",
            file_path.c_str());
    return nullptr;
  }

  fread(buffer, file_size, 1, fp);
  fclose(fp);

  buffer[file_size] = '\0';

  return buffer;
}

/// @brief mmap a large file, it's the caller's responsibility to unmap the file
/// @param file_path the path to the file to read.
/// @param file_size out parameter, the size of the file.
/// @return the mapped file, or nullptr if there was an error.
[[maybe_unused]] static char* MmapLargeFile(const std::string& file_path,
                                            size_t& file_size) {
#ifdef _WIN32
  assert(false && "not implemented");
#endif
  int fd = open(file_path.c_str(), O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "error opening file: %s\n", file_path.c_str());
    return nullptr;
  }

  struct stat file_stat;
  if (fstat(fd, &file_stat) == -1) {
    fprintf(stderr, "error getting file stats\n");
    close(fd);
    return nullptr;
  }

  file_size = file_stat.st_size;
  char* buffer = (char*)mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE, fd, 0);
  if (buffer == MAP_FAILED) {
    fprintf(stderr, "error mapping file: %s\n", file_path.c_str());
    close(fd);
    return nullptr;
  }

  close(fd);
  return buffer;
}
}  // namespace splitter

#endif  // _SPLITTER_UTIL_HPP_