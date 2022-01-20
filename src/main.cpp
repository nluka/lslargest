#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include "LargestEntries.hpp"

namespace fs = std::filesystem;

enum class ExitCode {
  Success = 0,
  NoArgsSpecified,
  SearchPathDoesNotExist,
  SearchPathNotDirectory,
  SearchPathEmpty,
  UnknownOption,
  InvalidOptionValue
};

int main(int const argc, char const * const * const argv) {
  if (argc <= 1) {
    std::cout
      << "Error: no arguments specified, use -h or --help for usage info"
      << std::endl;
    return static_cast<int>(ExitCode::NoArgsSpecified);
  }

  // handle help/version options
  for (int i = 1; i < argc; ++i) {
    if (
      strcmp(argv[i], "-h") == 0 ||
      strcmp(argv[i], "--help") == 0
    ) {
      std::cout
        << "Usage: SEARCHPATH [OPTION]...\n"
        << "Options:\n"
        << "  -h, --help     prints out this info\n"
        << "  -v, --version  prints out program version\n"
        << "  -n, --number   specifies how many entries to list (defaults to 10)\n"
        // << "  -s, --save-output  specifies the pathname of the file to save the output to\n"
        // << "  -m, --max-size     specifies the max file size to consider\n"
        << std::endl;
      exit(static_cast<int>(ExitCode::Success));
    } else if (
      strcmp(argv[i], "-v") == 0 ||
      strcmp(argv[i], "--version") == 0
    ) {
      std::cout << "Version 1.0.0\n" << std::endl;
      exit(static_cast<int>(ExitCode::Success));
    }
  }

  char const * const searchPath = argv[1];

  { // validate SEARCHPATH argument
    if (!fs::exists(argv[1])) {
      std::cout << "Error: " << searchPath << " does not exist\n" << std::endl;
      exit(static_cast<int>(ExitCode::SearchPathDoesNotExist));
    }
    if (!fs::is_directory(searchPath)) {
      std::cout << "Error: " << searchPath << " is not a directory\n" << std::endl;
      exit(static_cast<int>(ExitCode::SearchPathNotDirectory));
    }
    if (fs::is_empty(searchPath)) {
      std::cout << "Error: " << searchPath << " is empty\n" << std::endl;
      exit(static_cast<int>(ExitCode::SearchPathEmpty));
    }
  }

  size_t optionNumber = 10;

  // process options
  for (int i = 2; i < argc; ++i) {
    if (
      strcmp(argv[i], "-n") == 0 ||
      strcmp(argv[i], "--number") == 0
    ) {
      try {
        optionNumber = std::stoull(argv[i + 1]);
      } catch (...) {
        std::cout
          << "Error: failed to parse value `"
          << argv[i + 1]
          << "` for -n/--number option\n"
          << std::endl;
        exit(static_cast<int>(ExitCode::InvalidOptionValue));
      }
      if (optionNumber == 0) {
        std::cout << "Error: -n/--number option value must be > 0\n" << std::endl;
        exit(static_cast<int>(ExitCode::InvalidOptionValue));
      }
      ++i; // skip next arg since we used it as the value for this one
    } else {
      std::cout << "Error: unknown option `" << argv[i] << "`\n" << std::endl;
      exit(static_cast<int>(ExitCode::UnknownOption));
    }
  }

  LargestEntries largestEntries(optionNumber);

  { // search through entries within SEARCHPATH
    /*
    STRATEGY:
    recursively iterate through each file in SEARCHPATH
    for each `file`:
      1. if `largest` list is not full, binary insert `file` to `largest`
      2. else if `file` size > than smallest entry in `largest`,
        remove smallest entry from `largest` and binary insert `file`
    */

    std::cout << "Searching... ";

    size_t entriesFound = 0;
    for (
      auto const & entryPath :
      fs::recursive_directory_iterator(searchPath)
    ) {
      ++entriesFound;

      if (fs::is_directory(entryPath)) {
        continue;
      }

      uintmax_t const entrySize = fs::file_size(entryPath);

      if (
        largestEntries.is_full() &&
        entrySize < largestEntries.smallest_size()
      ) {
        continue;
      }

      largestEntries.insert(entryPath, entrySize);
    }

    std::cout << entriesFound << " entries found\n";
  }

  largestEntries.display(std::cout);

  exit(static_cast<int>(ExitCode::Success));
}
