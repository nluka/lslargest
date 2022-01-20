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

void process_numeric_option(
  unsigned long long & option,
  char const * flag,
  char const * name,
  char const * value
);

int main(int const argc, char const * const * const argv) {
  if (argc <= 1) {
    std::cout
      << "Error: no arguments specified, use -h or --help for usage info"
      << std::endl;
    exit(static_cast<int>(ExitCode::NoArgsSpecified));
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
        << "  -h, --help      prints out this info\n"
        << "  -v, --version   prints out program version\n"
        << "  -n, --number    specifies how many entries to list [default: 10]\n"
        << "  -m, --max-size  specifies the max file size (in bytes) to consider [default: any size]\n"
        // << "  -s, --save-output  specifies the pathname of the file to save the output to\n"
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
  uintmax_t optionMaxSize = static_cast<uintmax_t>(-1);

  // process options
  for (int i = 2; i < argc; ++i) {
    if (
      strcmp(argv[i], "-n") == 0 ||
      strcmp(argv[i], "--number") == 0
    ) {
      process_numeric_option(optionNumber, "-n", "--number", argv[i + 1]);
      ++i; // skip next arg since we used it as the value for this one
    } else if (
      strcmp(argv[i], "-m") == 0 ||
      strcmp(argv[i], "--max-size") == 0
    ) {
      process_numeric_option(optionMaxSize, "-m", "--max-size", argv[i + 1]);
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
        entrySize > optionMaxSize ||
        (largestEntries.is_full() &&
        entrySize < largestEntries.smallest_size())
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

void process_numeric_option(
  unsigned long long & option,
  char const * const flag,
  char const * const name,
  char const * const value
) {
  try {
    option = std::stoull(value);
  } catch (...) {
    std::cout
      << "Error: failed to parse value `" << value  << "` for " << flag << '/'
      << name << " option\n" << std::endl;
    exit(static_cast<int>(ExitCode::InvalidOptionValue));
  }
  if (option == 0) {
    std::cout
      << "Error: " << flag << '/' << name << " option value must be > 0\n"
      << std::endl;
    exit(static_cast<int>(ExitCode::InvalidOptionValue));
  }
}
