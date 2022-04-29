#ifndef LSLARGEST_UTIL_HPP
#define LSLARGEST_UTIL_HPP

#include <cstddef>

namespace lsl {

size_t count_char(char const *str, char c);

void assert_file(std::ofstream const &file, char const *name);

void handle_bad_alloc();

bool str_matches(
  char const *str,
  char const *testStr1,
  char const *testStr2
);

template<typename NumType>
constexpr inline
size_t count_digits(NumType num) {
  size_t count = 0;
  while (num != 0) {
    ++count;
    num /= 10;
  }
  return count;
}

// constexpr inline // since the regular strlen isn't constexpr...
// size_t strlen_ce(char const *str);

// constexpr inline // since the built-in strcmp isn't constexpr...
// int strcmp_ce(char const *s1, char const *s2);

// constexpr inline
// char last_char(char const *const str);

} // namespace lsl

#endif // LSLARGEST_UTIL_HPP
