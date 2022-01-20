#ifndef LSLARGEST_LARGEST_ENTRIES_HPP
#define LSLARGEST_LARGEST_ENTRIES_HPP

#include <vector>
#include <filesystem>
#include <ostream>

struct Entry {
  std::filesystem::path path;
  uintmax_t size;
  Entry();
  Entry(std::filesystem::path const & path, uintmax_t const size);
};

class LargestEntries {
  const size_t m_max;
  std::vector<Entry> m_entries; // descending order (by size)

  void binary_insert(
    std::filesystem::path const & entryPath,
    uintmax_t const entrySize,
    int const first,
    int const last
  );

public:
  LargestEntries(size_t max);
  bool is_full() const;
  bool is_overfilled() const;
  bool is_empty() const;
  uintmax_t smallest_size() const;
  void insert(std::filesystem::path entryPath, uintmax_t entrySize);
  size_t size() const;
  Entry const & operator[](size_t index) const;
  void display(std::ostream & os) const;
};

#endif // LSLARGEST_LARGEST_ENTRIES_HPP
