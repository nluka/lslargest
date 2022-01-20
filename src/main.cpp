#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include "FileSystemEntries.hpp"

namespace fs = std::filesystem;

enum class ExitCode {
  Success = 0,
  NoArgsSpecified,
  InputPathDoesNotExist,
  InputPathNotDirectory,
  UnknownArg,
  InvalidArgValue,
  NoEntriesFound
};

int main(int const argc, char const *const *const argv) {
  if (argc <= 1) {
    std::cout
      << "Error: no arguments specified, use -h or --help for usage info"
      << std::endl;
    return static_cast<int>(ExitCode::NoArgsSpecified);
  }

  for (int i = 1; i < argc; ++i) {
    if (
      strcmp(argv[i], "-h") == 0 ||
      strcmp(argv[i], "--help") == 0
    ) {
      std::cout
        << "Usage: DIRECTORY [OPTION]...\n"
        << "Options:\n"
        << "  -h, --help     prints out this info\n"
        << "  -v, --version  prints out program version\n"
        << "  -n, --number   specifies how many entries to list (defaults to 10)\n"
        // << "  -s, --save-output  specifies the pathname of the file to save the output to\n"
        // << "  -m, --max-size     specifies the max file size to consider\n"
        << std::endl;
      return static_cast<int>(ExitCode::Success);
    } else if (
      strcmp(argv[i], "-v") == 0 ||
      strcmp(argv[i], "--version") == 0
    ) {
      std::cout << "Version 1.0.0\n" << std::endl;
      return static_cast<int>(ExitCode::Success);
    }
  }

  auto const inputPath = argv[1];
  if (!fs::exists(inputPath)) {
    std::cout << "Error: " << inputPath << " does not exist\n" << std::endl;
    return static_cast<int>(ExitCode::InputPathDoesNotExist);
  }
  if (!fs::is_directory(inputPath)) {
    std::cout << "Error: " << inputPath << " is not a directory\n" << std::endl;
    return static_cast<int>(ExitCode::InputPathNotDirectory);
  }
  if (fs::is_empty(inputPath)) {
    std::cout << "Error: " << inputPath << " is empty\n" << std::endl;
    return static_cast<int>(ExitCode::NoEntriesFound);
  }

  size_t n = 10;

  // process optional arguments
  for (int i = 2; i < argc; ++i) {
    if (
      strcmp(argv[i], "-n") == 0 ||
      strcmp(argv[i], "--number") == 0
    ) {
      try {
        n = std::stoull(argv[i + 1]);
      } catch (...) {
        std::cout
          << "Error: failed to parse value `"
          << argv[i + 1]
          << "` for -n/--number option\n"
          << std::endl;
        return static_cast<int>(ExitCode::InvalidArgValue);
      }
      if (n == 0) {
        std::cout << "Error: -n/--number option value must be > 0\n" << std::endl;
        return static_cast<int>(ExitCode::InvalidArgValue);
      }
      ++i; // skip next arg since we used it as the value for this one
    } else {
      std::cout << "Error: unknown option `" << argv[i] << "`\n" << std::endl;
      return static_cast<int>(ExitCode::UnknownArg);
    }
  }

  std::cout << "Searching... ";

  /*
    STRATEGY:

    Variables:
    `largest`: array of top n biggest files (by size) found so far, sorted in ascending order

    Algorithm:
    recursively iterate through each file in DIRECTORY
    for each `file`:
      1. if `largest` list is not full, binary insert `file` to `largest`
      2. else if `file` size > than smallest entry in `largest`,
        remove smallest entry from `largest` and binary insert `file`
  */

  size_t entriesVisited = 0;
  FileSystemEntries largest(n);
  for (auto const &entryPath : fs::recursive_directory_iterator(inputPath)) {
    ++entriesVisited;
    if (fs::is_directory(entryPath)) {
      continue;
    }
    auto const entrySize = fs::file_size(entryPath);
    if (largest.is_full() && entrySize < largest.smallest_size()) {
      continue;
    }
    largest.insert(entryPath, entrySize);
  }

  auto const format_bytes = [](
    uintmax_t bytes,
    char *str,
    size_t bufCount
  ) {
    char const *sizes[] = { "B", "KB", "MB", "GB", "TB" };
    int i;
    double bytesDbl = static_cast<double>(bytes);
    for (
      i = 0;
      i < 5 && bytes >= 1024;
      ++i, bytes /= 1024
    ) {
      bytesDbl = bytes / 1024.0;
    }
    snprintf(str, bufCount, "%.2f", bytesDbl);
    strcat(strcat(str, " "), sizes[i]);
  };

  auto const format_path_str = [](std::string &str) {
    for (size_t i = 0; i < str.length(); ++i) {
      char const ch = str[i];
      if ((ch == '/' || ch == '\\') && ch != fs::path::preferred_separator) {
        str[i] = fs::path::preferred_separator;
      }
    }
  };

  std::cout
    << entriesVisited << " entries found\n"
    << "--------------------------\n"
    << "RANK  SIZE        PATHNAME\n";
  for (size_t i = 0; i < largest.size(); ++i) {
    auto const &entry = largest[i];
    char formattedBytesStr[11];
    format_bytes(
      entry.size,
      formattedBytesStr,
      sizeof formattedBytesStr
    );
    std::string entryPathStr = entry.path.u8string();
    format_path_str(entryPathStr);
    std::cout
      << std::setw(4) << std::left << (i + 1) << "  "
      << std::setw((sizeof formattedBytesStr) - 1) << std::left
        << formattedBytesStr << "  "
      << entryPathStr << '\n';
  }
  std::cout << std::endl;

  return static_cast<int>(ExitCode::Success);
}
