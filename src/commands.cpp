#include "splitter/commands.h"

#include <args.hxx>
#include <cassert>

#include "splitter/consts.h"
#include "splitter/filesystem.h"
#include "splitter/util.hpp"

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
        fprintf(stderr, "error parsing repo manifest file: %s dosn't exist",
                manifest.c_str());
        exit(splitter::ErrNotExist);
      }

      if (!EndsWith(manifest, ".xml")) {
        fprintf(stderr, "error paring repo manifest file: %s is not a xml file",
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
              "folder",
              manifest_folder->c_str());
      exit(splitter::ErrNotValid);
    }

    for (const auto& entry : fs::directory_iterator(folder)) {
      if (entry.is_regular_file() && entry.path().extension() == ".xml") {
        manifest_files.emplace_back(entry.path().c_str());
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

  fprintf(stdout, "parsing split subcommand...");
  std::vector<std::string> manifest_files =
      _details::parseManifestFiles(manifests, manifest_folder);
  fprintf(stdout, "manifestt files vector size: %lu, first one: %s",
          manifest_files.size(), manifest_files[0].c_str());

  const std::string cdb_file = fs::path(args::get(CDB_path));
  if (!fs::exists(cdb_file) || !fs::is_regular_file(cdb_file) ||
      fs::path(cdb_file).extension() != ".json") {
    fprintf(stderr, "error paring cdb path: %s is illegal", cdb_file.c_str());
    exit(splitter::ErrNotValid);
  }
  fprintf(stdout, "cdb_file path: %s", cdb_file.c_str());

  const std::string dest_folder = fs::path(args::get(dest));
  if (!fs::is_directory(dest_folder)) {
    fprintf(stderr, "error paring destination folder: %s is illegal",
            dest_folder.c_str());
    exit(splitter::ErrNotValid);
  }
  if (!fs::is_empty(dest_folder)) {
    fs::remove_all(dest_folder);
  }
  if (!fs::exists(dest_folder)) {
    fs::create_directories(dest_folder);
  }
  fprintf(stdout, "dest_folder path: %s", dest_folder.c_str());

  int res = splitter::DoSplitCommand(manifest_files, cdb_file, dest_folder);

  if (res != 0) {
    fprintf(stderr, "error occurs doing split command");
    exit(res);
  }
}
}  // end of namespace commands

int DoSplitCommand(const std::vector<std::string>& manifest_files,
                   const std::string& cdb_file,
                   const std::string& dest_folder) {
  return 0;
}

}  // end of namespace splitter