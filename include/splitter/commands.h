#ifndef _SP_COMMANDS_HPP_
#define _SP_COMMANDS_HPP_

#include <string>
#include <vector>

namespace args {
class Subparser;
}

namespace splitter {
namespace commands {
void SplitCommand(args::Subparser& parser);
}  // end of namespace commands

int DoSplitCommand(const std::vector<std::string>& manifest_files,
                   const std::string& cdb_file, const std::string& dest_folder,
                   const std::string& aosp_base_path);

}  // end of namespace splitter

#endif
