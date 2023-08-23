#include "splitter/commands.h"

#if defined(__SSE4_2__)
#define RAPIDJSON_SSE42
#elif defined(__SSE2__)
#define RAPIDJSON_SSE2
#elif defined(__ARM_NEON)
#define RAPIDJSON_NEON
#endif

#include <oneapi/tbb.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <execution>

#ifdef SP_WITH_MMAP
#include "mio/mio.hpp"
#else
#include <rapidjson/filereadstream.h>
#endif

#include <args.hxx>
#include <cassert>

#include "splitter/consts.h"
#include "splitter/filesystem.h"
#include "splitter/trie.hpp"
#include "splitter/util.hpp"
#include "splitter/xml_trie.hpp"

#ifndef _WIN32
#include <sys/mman.h>
#endif

namespace splitter {
namespace _details {
std::vector<std::string> parseManifestFiles(
    args::ValueFlagList<std::string>& manifests,
    args::ValueFlag<std::string>& manifest_folder) {
  std::vector<std::string> manifest_files;
  if (manifests) {
    manifest_files = std::move(args::get(manifests));
    for (const auto& manifest : manifest_files) {
      fs::path manifest_file = fs::path(manifest);
      if (!fs::exists(manifest_file)) {
        fprintf(stderr, "error parsing repo manifest file: %s dosn't exist\n",
                manifest.c_str());
        exit(splitter::ErrNotExist);
      }

      if (!EndsWith(manifest, ".xml")) {
        fprintf(stderr,
                "error paring repo manifest file: %s is not a xml file\n",
                manifest.c_str());
        exit(splitter::ErrNotValid);
      }
    }
  }
  if (manifest_folder) {
    const std::string folder = args::get(manifest_folder);
    if (!fs::exists(fs::path(folder)) || !fs::is_directory(fs::path(folder))) {
      fprintf(stderr,
              "error paring repo manifests folder: %s doesn't exist or isn't a "
              "folder\n",
              manifest_folder->c_str());
      exit(splitter::ErrNotValid);
    }

    for (const auto& entry : fs::directory_iterator(folder)) {
      if (entry.is_regular_file() && entry.path().extension() == ".xml") {
        manifest_files.emplace_back(entry.path().string());
        if (manifest_files.size() >= splitter::MANIFEST_FILES_LIMIT_SIZE) {
          break;
        }
      }
    }
  }

  assert(manifest_files.size() > 0 &&
         "manifest_files vector should be nonempty");

  return manifest_files;
}
}  // namespace _details

namespace commands {
void SplitCommand(args::Subparser& parser) {
  args::Group group(parser, "repo manifests file group",
                    args::Group::Validators::Xor);
  args::ValueFlagList<std::string> manifests(
      group, "manifests", "repo manifest files", {'x', "xml"});
  args::ValueFlag<std::string> manifest_folder(
      group, "manifests directory", "repo manifests directory", {'d', "dir"});

  args::ValueFlag<std::string> CDB_path(
      parser, "path", "path of compile_commands.json to be splitted",
      {'c', "cdb"});

  args::Positional<std::string> dest(parser, "destination",
                                     "destination folder to store splitted CDB",
                                     "/tmp/cdb");
  args::HelpFlag help(parser, "help", "display global help menu",
                      {'h', "help"});

  parser.Parse();

  fprintf(stdout, "parsing split subcommand...\n");
  std::vector<std::string> manifest_files =
      _details::parseManifestFiles(manifests, manifest_folder);
  fprintf(stdout, "manifest files vector size: %lu, first element: %s\n",
          manifest_files.size(), manifest_files[0].c_str());

  const std::string cdb_file = args::get(CDB_path);
  if (!fs::exists(cdb_file) || !fs::is_regular_file(cdb_file) ||
      fs::path(cdb_file).extension() != ".json") {
    fprintf(stderr, "error paring cdb path: %s is illegal\n", cdb_file.c_str());
    exit(splitter::ErrNotValid);
  }
  fprintf(stdout, "cdb_file path: %s\n", cdb_file.c_str());

  const std::string dest_folder = args::get(dest);
  if (!fs::exists(dest_folder)) {
    fs::create_directories(dest_folder);
  } else if (!fs::is_directory(dest_folder)) {
    fprintf(stderr, "error paring destination folder: %s is illegal\n",
            dest_folder.c_str());
    exit(splitter::ErrNotValid);
  }
  if (!fs::is_empty(dest_folder)) {
    fs::remove_all(dest_folder);
  }
  fprintf(stdout, "dest_folder path: %s\n\n", dest_folder.c_str());

  const auto [elapsed, _] =
      MeasureRunningTime(DoSplitCommand, manifest_files, cdb_file, dest_folder);

  fprintf(
      stdout,
      "\n************** Stat **************\n\nit takes %ld ms to split cdb\n",
      elapsed);
}
}  // end of namespace commands

int DoSplitCommand(const std::vector<std::string>& manifest_files,
                   const std::string& cdb_file,
                   const std::string& dest_folder) {
  Trie trie = BuildFromXMLs(manifest_files);

#ifndef NDEBUG
  printf("trie tree: \n%s\n", trie.ToString().c_str());
#endif

#ifdef SP_USE_OS_API
#ifdef _WIN32
  assert(false && "not implemented yet");
#endif

  fprintf(
      stdout,
      "on unix platform, use mmap `MAP_PRIVATE` to do inplace json parse\n");
  size_t file_size = 0;
  char* buffer = MmapLargeFile(cdb_file, file_size);
  if (buffer == nullptr) {
    fprintf(stderr, "error mapping cdb file: %s\n", cdb_file.c_str());
    return ErrNotValid;
  }
  rapidjson::Document doc;
  const auto parse_elapsed =
      MeasureRunningTime([&]() { doc.ParseInsitu(buffer); });

  if (doc.HasParseError()) {
    printf("error parse cdb at offset %lu: %s\n", doc.GetErrorOffset(),
           rapidjson::GetParseError_En(doc.GetParseError()));
    return ErrNotValid;
  }

  fprintf(stdout, "it takes %ld ms to parse cdb\n", parse_elapsed);
#else  // SP_USE_OS_API
#ifdef SP_WITH_MMAP
  fprintf(stdout, "mmap found, using mmap to parse cdb file\n");
  std::error_code error;
  mio::mmap_source mmap =
      mio::make_mmap_source(cdb_file, 0, mio::map_entire_file, error);
  if (error) {
    fprintf(stderr, "error mapping cdb file: %s\n", error.message().c_str());
    return ErrNotValid;
  }
  rapidjson::StringStream ss(mmap.data());
  rapidjson::Document doc;
  const auto parse_elapsed = MeasureRunningTime([&]() { doc.ParseStream(ss); });
  fprintf(stdout, "it takes %ld ms to parse cdb\n", parse_elapsed);
#else  // SP_WITH_MMAP
  fprintf(stdout,
          "mmap not found, using rapidjson FileReadStream to parse cdb file\n");
#ifdef _WIN32
  FILE* fp = fopen(cdb_file.c_str(), "rb");
#else
  FILE* fp = fopen(cdb_file.c_str(), "r");
#endif

  char read_buf[2 << 15];
  rapidjson::FileReadStream is(fp, read_buf, sizeof(read_buf));

  rapidjson::Document doc;
  const auto parse_elapsed = MeasureRunningTime([&]() { doc.ParseStream(is); });
  fprintf(stdout, "it takes %ld ms to parse cdb\n", parse_elapsed);

#endif  // end of SP_WITH_MMAP
#endif  // end of SP_USE_OS_API

  assert(doc.IsArray() && "schema of compile_commands.json should be an array");

  const auto& command_arr = doc.GetArray();
  fprintf(stdout, "size of compile commands array is %u\n", command_arr.Size());

  //   auto sort_elapsed = MeasureRunningTime([&]() {
  //     std::sort(std::execution::par_unseq, command_arr.Begin(),
  //     command_arr.End(),
  //               [](const auto& a, const auto& b) {
  //                 return std::string_view(a["file"].GetString()) <
  //                        std::string_view(b["file"].GetString());
  //               });
  //   });
  //   printf("it takes %ld ms to sort cdb arr\n", sort_elapsed);

  std::unordered_map<std::string, rapidjson::Value> commands_map;

  auto processing_elapsed = MeasureRunningTime([&]() {
    for (auto& command : command_arr) {
      //   printf("processing command: %s\n", command["file"].GetString());
      const auto prefix = trie.SearchPrefix(command["file"].GetString());
      fs::path commands_dest = fs::path(dest_folder) / prefix;
      const auto commands_dest_str = commands_dest.string();
      if (commands_map.find(commands_dest_str) == commands_map.end()) {
        commands_map[commands_dest_str] =
            rapidjson::Value(rapidjson::kArrayType);
      }
      commands_map[commands_dest_str].PushBack(command.Move(),
                                               doc.GetAllocator());
    }
  });
  fprintf(stdout, "it takes %ld ms to process cdb\n", processing_elapsed);

  auto write_elapsed = MeasureRunningTime([&]() {
    oneapi::tbb::parallel_for_each(
        commands_map.begin(), commands_map.end(), [&](const auto& file) {
          const auto& [dest_folder, commands] = file;
          fs::create_directories(dest_folder);

          fs::path cdb_dest = fs::path(dest_folder) / "compile_commands.json";
#ifdef _WIN32
          FILE* fp = fopen(cdb_dest.string().c_str(), "wb");
#else
          FILE* fp = fopen(cdb_dest.string().c_str(), "w");
#endif
          char write_buf[2 << 15];
          rapidjson::FileWriteStream os(fp, write_buf, sizeof(write_buf));
          rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
          commands.Accept(writer);
          fclose(fp);
        });
  });
  fprintf(stdout, "it takes %ld ms to write output\n", write_elapsed);

#ifdef SP_USE_OS_API
  if (munmap(buffer, file_size) == -1) {
    fprintf(stderr, "error unmapping memory");
    return ErrUnknown;
  }
#endif

  return 0;
}

}  // end of namespace splitter