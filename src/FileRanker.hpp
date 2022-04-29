#ifndef LSLARGEST_LARGEST_ENTRIES_HPP
#define LSLARGEST_LARGEST_ENTRIES_HPP

#include <vector>
#include <filesystem>
#include <ostream>

namespace lsl {

struct File {
  std::filesystem::path m_fsPath;
  uintmax_t m_size;

  File(std::filesystem::path const &fsPath, uintmax_t size);
};

class FileRanker {
  size_t const m_rootPathLength;
  size_t const m_max;
  std::vector<File> m_entries; // descending order (by size)

  void binary_insert(
    std::filesystem::path const &entryPath,
    uintmax_t const entrySize,
    int const first,
    int const last
  );

public:
  FileRanker(size_t rootPathLength, size_t max);
  bool        is_full() const;
  bool        is_overfilled() const;
  bool        is_empty() const;
  uintmax_t   smallest_size() const;
  void        insert(std::filesystem::path const &fsPath, uintmax_t size);
  size_t      size() const;
  File const &operator[](size_t index) const;
  void        print(std::ostream &os) const;
};

} // namespace lsl

#endif // LSLARGEST_LARGEST_ENTRIES_HPP
