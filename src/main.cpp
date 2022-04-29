#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <sstream>
#include <chrono>
#include <ctime>
#include "ExitCode.hpp"
#include "FileRanker.hpp"
#include "util.hpp"
#include "defs.hpp"

namespace fs = std::filesystem;
using lsl::ExitCode, lsl::str_matches;

int main(int const argc, char *const *const argv) {
  if (argc <= 1) {
    std::cout
      << "   Usage: <root> [<option> [<value>]]...\n"
      << " Options: -v, --version                       prints program version\n"
      << "          -r, --rank-limit  [1, 1,000,000)    how many files to list [default: 10]\n"
      << "          -m, --max-size    [1, UINTMAX_MAX]  max file size (in bytes) to consider [default: UINTMAX_MAX]\n"
      << "          -e, --extensions  CSV               which file extensions to consider [default: all]\n"
      << "          -s, --save        pathname          file to write output into [default: nil]\n"
      << "          -c, --console     0|1               print file rankings to console\n";
    return static_cast<int>(ExitCode::NO_ARGS_SPECIFIED);
  }

  // handle version flag
  for (int i = 1; i < argc; ++i) {
    if (str_matches(argv[i], "-v", "--version")) {
      std::cout << "lslargest version " << lsl::VERSION << '\n';
      return static_cast<int>(ExitCode::SUCCESS);
    }
  }

  // <root> arg
  std::string rootPathname(argv[1]);
  if (rootPathname.back() != fs::path::preferred_separator)
    rootPathname += fs::path::preferred_separator;
  // validate
  if (!fs::exists(rootPathname)) {
    std::cerr << "ERROR: root `" << rootPathname << "` does not exist\n";
    return static_cast<int>(ExitCode::ROOT_DOES_NOT_EXIST);
  }
  if (!fs::is_directory(rootPathname)) {
    std::cerr << "ERROR: root `" << rootPathname << "` is not a directory\n";
    return static_cast<int>(ExitCode::ROOT_NOT_DIRECTORY);
  }
  if (fs::is_empty(rootPathname)) {
    std::cerr << "ERROR: root `" << rootPathname << "` is empty\n";
    return static_cast<int>(ExitCode::ROOT_EMPTY);
  }

  // option variables
  uintmax_t rankLimit = 10, maxSize = UINTMAX_MAX;
  std::vector<std::string> extensions{};
  char const *savePathname = nullptr;
  bool console = true;

  { // process options
    auto const parseNumericOpt = [](
      char const *const optName,
      char const *const optVal,
      uintmax_t &parsedVal,
      uintmax_t const min,
      uintmax_t const max
    ) {
      try {
        parsedVal = std::stoull(optVal);
      } catch (...) {
        std::cerr << "ERROR: failed to parse option `" << optName << "` value\n";
        exit(static_cast<int>(ExitCode::BAD_OPTION_VALUE));
      }

      if (parsedVal < min || parsedVal > max) {
        std::cerr << "ERROR: option `" << optName << "` value must be in range ["
          << min << ", " << max << "]\n";
        exit(static_cast<int>(ExitCode::BAD_OPTION_VALUE));
      }
    };

    for (int i = 2; i < argc; i += 2) {
      char const *const optName = argv[i];
      {
        bool const isOptValMissing = (i == argc - 1);
        if (isOptValMissing) {
          std::cerr << "ERROR: option `" << optName << "` is missing value\n";
          return static_cast<int>(ExitCode::OPTION_VALUE_MISSING);
        }
      }
      char *const optVal = argv[i + 1];

      if (
        str_matches(optName, "-n", "--num-files")
      ) {
        parseNumericOpt(optName, optVal, rankLimit, 1, lsl::MAX_RANK);
      } else if (
        str_matches(optName, "-m", "--max-size")
      ) {
        parseNumericOpt(optName, optVal, maxSize, 1, UINTMAX_MAX);
      } else if (
        str_matches(optName, "-e", "--extensions")
      ) {
        try {
          size_t const extCount = lsl::count_char(optVal, ',') + 1;
          extensions.reserve(extCount); // allocate enough to avoid resizing
        } catch (...) {
          lsl::handle_bad_alloc();
        }
        char const *const delim = ",";

        char const *piece = strtok(optVal, delim);
        while (piece != nullptr) {
          extensions.emplace_back(piece);
          piece = strtok(nullptr, delim);
        }
      } else if (
        str_matches(optName, "-s", "--save")
      ) {
        savePathname = optVal;
      } else if (
        str_matches(optName, "-c", "--console")
      ) {
        uintmax_t parsedVal;
        parseNumericOpt(optName, optVal, parsedVal, 0, 1);
        console = static_cast<bool>(parsedVal);
      } else {
        std::cerr << "ERROR: unknown option `" << optName << "`\n";
        return static_cast<int>(ExitCode::UNKNOWN_OPTION);
      }
    }
  }

  size_t filesFound = 0;
  lsl::FileRanker fileTracker(rootPathname.size(), rankLimit);
  auto const start = std::chrono::system_clock::now();
  std::cout << "Searching... ";

  /*
    STRATEGY:
    iterate through each file in <root>, for each file:
      1. if fileTracker is not full, binary insert the file
      2. else if file size > than smallest file in fileTracker,
        remove smallest file from fileTracker and binary insert the file
  */

  for (
    auto const &entry :
    fs::recursive_directory_iterator(
      rootPathname,
      fs::directory_options::skip_permission_denied
    )
  ) {
    if (entry.is_directory()) {
      continue;
    }

    ++filesFound;

    uintmax_t const size = fs::file_size(entry);

    bool const doesExtensionMatch =
      extensions.empty() ||
      ([&extensions, &entry](){
        std::string const entryExt = entry.path().extension().string();

        for (auto const &ext : extensions) {
          // `entryExt` starts with . but `ext` doesn't

          if (entryExt.size() != (ext.size() + 1)) {
            continue;
          }
          bool match = true;
          for (size_t i = 0; i < ext.size(); ++i) {
            if (entryExt[i + 1] != ext[i]) {
              match = false;
              break;
            }
          }
          if (match) {
            return true;
          }
        }

        return false;
      })();

    if (
      size <= maxSize &&
      doesExtensionMatch &&
      (!fileTracker.is_full() || size > fileTracker.smallest_size())
    ) {
      lsl::File file(entry, size);
      fileTracker.insert(entry, size);
    }
  }

  auto const end = std::chrono::system_clock::now();
  std::chrono::duration<double> const secondsElapsed = end - start;

  {
    auto const printResult = [
      secondsElapsed,
      &fileTracker,
      filesFound
    ](std::ostream &os, bool printRankings){
      os
        << filesFound << " files processed in "
        << std::setprecision(2) << secondsElapsed.count() << "s\n";
      if (printRankings) {
        fileTracker.print(os);
      }
    };

    printResult(std::cout, console);
    std::cout << '\n';

    if (savePathname != nullptr) {
      std::ofstream file(savePathname);
      lsl::assert_file(file, savePathname);

      for (int i = 0; i < argc; ++i)
        file << argv[i] << ' ';
      file << "\n\n";

      file << "Version - " << lsl::VERSION << '\n';

      {
        auto const
          startTime = std::chrono::system_clock::to_time_t(start),
          endTime = std::chrono::system_clock::to_time_t(end);
        file
          << "Started - " << std::ctime(&startTime)
          << "Ended   - " << std::ctime(&endTime)
          << '\n';
      }

      printResult(file, true);
    }
  }

  return static_cast<int>(ExitCode::SUCCESS);
}
