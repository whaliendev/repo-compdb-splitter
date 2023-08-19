#ifndef _SP_FILESYSTEM_H
#define _SP_FILESYSTEM_H

#if __has_include(<filesystem>)
#include <filesystem>
namespace splitter {
namespace fs = std::filesystem;
}
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace splitter {
namespace fs = std::experimental::filesystem;
}
#else
#error "missing <filesystem> header."
#endif

#endif  // _SP_FILESYSTEM_H