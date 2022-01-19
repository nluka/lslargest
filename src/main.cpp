#include <iostream>

enum class ExitCode {
  Success = 0
};

int main(const int argc, char const *const *const argv) {
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      std::cout
        << "Usage: DIRECTORY [OPTION]...\n"
        << "Options:\n"
        << "  -h, --help         prints out this info\n"
        << "  -v, --version      prints out program version\n"
        << "  -n, --number       specifies how many entries to list\n"
        << "  -s, --save-output  specifies the pathname of the file to save the output to\n"
        << std::endl;
      return static_cast<int>(ExitCode::Success);
    } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      std::cout << "Version 1.0.0\n" << std::endl;
      return static_cast<int>(ExitCode::Success);
    }
  }

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

  return static_cast<int>(ExitCode::Success);
}
