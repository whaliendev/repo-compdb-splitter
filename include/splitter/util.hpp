#ifndef _SPLITTER_UTIL_HPP_
#define _SPLITTER_UTIL_HPP_

#include <chrono>
#include <functional>
#include <string_view>
#include <type_traits>

namespace splitter {
template <typename Func, typename... Args>
static typename std::enable_if<std::is_invocable_v<Func, Args...>, long>::type
MeasureRunningTime(Func&& func, Args&&... args) {
  auto start = std::chrono::high_resolution_clock::now();
  std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
      .count();
}

static bool EndsWith(std::string_view sv, const std::string_view suffix) {
  if (sv.length() < suffix.length()) {
    return false;
  }
  return sv.compare(sv.length() - suffix.length(), suffix.length(), suffix) ==
         0;
}
}  // namespace splitter

#endif  // _SPLITTER_UTIL_HPP_