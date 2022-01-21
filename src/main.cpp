#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <fstream>
#include "LargestEntries.hpp"

namespace fs = std::filesystem;

enum class ExitCode {
  Success = 0,
  NoArgsSpecified,
  SearchPathDoesNotExist,
  SearchPathNotDirectory,
  SearchPathEmpty,
  UnknownOption,
  OptionValueMissing,
  InvalidOptionValue,
  FailedToOpenFile
};

void validate_option_value_exists(
  int argc,
  int optionIndex,
  char const * flagOrName
);
void process_numeric_option(
  char const * flagOrName,
  uintmax_t & optVal,
  char const * cliVal,
  uintmax_t min,
  uintmax_t max
);
void assert_file(std::ofstream const & file, char const * name);

int main(int const argc, char const * const * const argv) {
  if (argc <= 1) {
    std::cout
      << "Error: no arguments specified, use -h or --help for usage info"
      << std::endl;
    exit(static_cast<int>(ExitCode::NoArgsSpecified));
  }

  // handle help/version options
  for (int i = 1; i < argc; ++i) {
    char const * const arg = argv[i];
    if (
      strcmp(arg, "-h") == 0 ||
      strcmp(arg, "--help") == 0
    ) {
      std::cout
        << "Usage: SEARCHPATH [OPTION]...\n"
        << "Options:\n"
        << "  -h, --help         prints this info\n"
        << "  -v, --version      prints program version\n"
        << "  -n, --number       specifies how many files to list [default: 10]\n"
        << "  -m, --max-size     specifies max file size (in bytes) to consider [default: any size]\n"
        << "  -e, --extension    specifies which file extension to consider [default: any extension]\n"
        << "  -s, --save-output  specifies pathname of file to save output to"
        << std::endl;
      exit(static_cast<int>(ExitCode::Success));
    } else if (
      strcmp(arg, "-v") == 0 ||
      strcmp(arg, "--version") == 0
    ) {
      std::cout << "lslargest version 1.0.0" << std::endl;
      exit(static_cast<int>(ExitCode::Success));
    }
  }

  std::string searchPath(argv[1]);
  if (searchPath.back() != fs::path::preferred_separator) {
    searchPath += fs::path::preferred_separator;
  }

  { // validate SEARCHPATH argument
    if (!fs::exists(searchPath)) {
      std::cout << "Error: " << searchPath << " does not exist" << std::endl;
      exit(static_cast<int>(ExitCode::SearchPathDoesNotExist));
    }
    if (!fs::is_directory(searchPath)) {
      std::cout << "Error: " << searchPath << " is not a directory" << std::endl;
      exit(static_cast<int>(ExitCode::SearchPathNotDirectory));
    }
    if (fs::is_empty(searchPath)) {
      std::cout << "Error: " << searchPath << " is empty" << std::endl;
      exit(static_cast<int>(ExitCode::SearchPathEmpty));
    }
  }

  uintmax_t number = 10, maxSize = static_cast<uintmax_t>(-1);
  char const * saveOutput = nullptr;
  char const * extension = nullptr;

  // process options
  for (int i = 2; i < argc; ++i) {
    char const * const option = argv[i];
    if (
      strcmp(option, "-n") == 0 ||
      strcmp(option, "--number") == 0
    ) {
      validate_option_value_exists(argc, i, option);
      process_numeric_option(option, number, argv[i + 1], 1, 1000);
      ++i; // skip next arg since we used it as value for this one
    } else if (
      strcmp(option, "-m") == 0 ||
      strcmp(option, "--max-size") == 0
    ) {
      validate_option_value_exists(argc, i, option);
      process_numeric_option(option, maxSize, argv[i + 1], 1, UINTMAX_MAX);
      ++i;
    } else if (
      strcmp(option, "-e") == 0 ||
      strcmp(option, "--extension") == 0
    ) {
      validate_option_value_exists(argc, i, option);
      extension = argv[i + 1];
      ++i;
    } else if (
      strcmp(option, "-s") == 0 ||
      strcmp(option, "--save-output") == 0
    ) {
      validate_option_value_exists(argc, i, option);
      saveOutput = argv[i + 1];
      // std::ofstream file(saveOutput);
      // assert_file(file, saveOutput);
      ++i;
    } else {
      std::cout << "Error: unknown option " << option << std::endl;
      exit(static_cast<int>(ExitCode::UnknownOption));
    }
  }

  size_t filesFound = 0;
  LargestEntries largestEntries(searchPath.size(), number);

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

    for (
      auto const & entryPath :
      fs::recursive_directory_iterator(searchPath)
    ) {
      if (entryPath.is_directory()) {
        continue;
      }
      ++filesFound;
      uintmax_t const entrySize = fs::file_size(entryPath);
      if (
        entrySize > maxSize ||
        (extension != nullptr &&
          entryPath.path().extension().string() != extension) ||
        (largestEntries.is_full() &&
          entrySize < largestEntries.smallest_size())
      ) {
        continue;
      }
      largestEntries.insert(entryPath, entrySize);
    }
  }

  { // display result
    std::cout << filesFound << " files found\n";
    largestEntries.display(std::cout);
  }

  if (saveOutput == nullptr) {
    exit(static_cast<int>(ExitCode::Success));
  }

  { // write result to output file
    std::ofstream file(saveOutput);
    assert_file(file, saveOutput);
    file
      << "Found " << filesFound << " files in "
      << fs::absolute(searchPath).string() << '\n';
    largestEntries.display(file);
  }

  exit(static_cast<int>(ExitCode::Success));
}

void validate_option_value_exists(
  int const argc,
  int const optionIndex,
  char const * const flagOrName
) {
  bool const isOptionValueMissing = optionIndex == argc - 1;
  if (isOptionValueMissing) {
    std::cout
      << "Error: " << flagOrName << " is missing value" << std::endl;
    exit(static_cast<int>(ExitCode::OptionValueMissing));
  }
}

void process_numeric_option(
  char const * const flagOrName,
  uintmax_t & optVal,
  char const * const cliVal,
  uintmax_t const min,
  uintmax_t const max
) {
  try {
    optVal = std::stoull(cliVal);
  } catch (...) {
    std::cout
      << "Error: failed to parse " << flagOrName << " value" << std::endl;
    exit(static_cast<int>(ExitCode::InvalidOptionValue));
  }
  if (optVal < min || optVal > max) {
    std::cout
      << "Error: " << flagOrName << " value must be in range [" << min << ", "
      << max << ']' << std::endl;
    exit(static_cast<int>(ExitCode::InvalidOptionValue));
  }
}

void assert_file(std::ofstream const & file, char const * const name) {
  if (!file.is_open()) {
    std::cout << "Error: failed to open file " << name << std::endl;
    exit(static_cast<int>(ExitCode::FailedToOpenFile));
  }
}
