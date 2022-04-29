#include <cstring>
#include <fstream>
#include <iostream>
#include "util.hpp"
#include "ExitCode.hpp"

size_t lsl::count_char(char const *const str, char const c) {
  size_t count = 0;
  size_t const len = strlen(str);
  for (size_t i = 0; i < len; ++i) {
    if (str[i] == c) {
      ++count;
    }
  }
  return count;
}

void lsl::assert_file(std::ofstream const &file, char const *const name) {
  if (!file.is_open()) {
    std::cerr << "ERROR: failed to open file `" << name << "`\n";
    exit(static_cast<int>(ExitCode::FAILED_TO_OPEN_FILE));
  }
}

void lsl::handle_bad_alloc() {
  std::cerr << "ERROR: not enough memory\n";
  exit(static_cast<int>(ExitCode::NOT_ENOUGH_MEMORY));
}

bool lsl::str_matches(
  char const *const str,
  char const *const testStr1,
  char const *const testStr2
) {
  // return
  //   lsl::strcmp_ce(str, testStr1) == 0 ||
  //   lsl::strcmp_ce(str, testStr2) == 0;
  return
    strcmp(str, testStr1) == 0 ||
    strcmp(str, testStr2) == 0;
}

// constexpr inline // since the built-in strlen isn't constexpr...
// size_t lsl::strlen_ce(char const *const str) {
//   size_t i = 0;
//   while (str[i] != '\0') ++i;
//   return i;
// }

// constexpr inline // since the built-in strcmp isn't constexpr...
// int lsl::strcmp_ce(char const *s1, char const *s2) {
//   // from https://stackoverflow.com/a/34873406/16471560
//   while(*s1 && (*s1 == *s2)) {
//     ++s1;
//     ++s2;
//   }
//   return *(unsigned char const *)s1 - *(unsigned char const *)s2;
// }

// constexpr inline
// char lsl::last_char(char const *const str) {
//   size_t const len = strlen(str);
//   // size_t const len = strlen(str);
//   return len <= 0
//     ? '\0'
//     : str[len - 1];
// }
