#include <args.hxx>
#include <cstdio>
#include <filesystem>

#include "splitter/commands.h"
#include "splitter/consts.h"

args::Group arguments("arguments");
args::HelpFlag help(arguments, "help", "display global help menu",
                    {'h', "help"});

int main(int argc, char** argv) {
  args::ArgumentParser parser(
      "a program to split CDB of repo managed C/C++ projects");

  args::Group commands(parser, "commands");
  args::Command split(commands, "split", "command to split CDB",
                      &splitter::commands::SplitCommand);

  args::GlobalOptions globals(parser, arguments);
  args::CompletionFlag completion(parser, {"complete"});

  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Completion& e) {
    std::cout << e.what();
    return 0;
  } catch (const args::Help&) {
    std::cout << parser;
    return 0;
  } catch (const args::ParseError& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  return 0;
}