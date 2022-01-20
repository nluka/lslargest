#include "FileSystemEntries.hpp"

namespace fs = std::filesystem;

Entry::Entry() : path{}, size{} {}

Entry::Entry(std::filesystem::path path, uintmax_t const size)
: path{path}, size{size} {}

FileSystemEntries::FileSystemEntries(size_t max)
: m_max{max}, m_entries{} {
  m_entries.reserve(max);
}

bool FileSystemEntries::is_full() const {
  return m_entries.size() >= m_max;
}

bool FileSystemEntries::is_overfilled() const {
  return m_entries.size() > m_max;
}

bool FileSystemEntries::is_empty() const {
  return m_entries.size() == 0;
}

uintmax_t FileSystemEntries::smallest_size() const {
  if (is_empty()) {
    return 0;
  }
  return m_entries[m_entries.size() - 1].size;
}

void FileSystemEntries::insert(fs::path entryPath, uintmax_t entrySize) {
  if (is_empty()) {
    m_entries.emplace_back(entryPath, entrySize);
  } else {
    binary_insert(entryPath, entrySize, 0, static_cast<int>(m_entries.size()) - 1);
    if (is_overfilled()) {
      m_entries.pop_back();
    }
  }
}

void FileSystemEntries::binary_insert(
  fs::path const &entryPath,
  uintmax_t const entrySize,
  int first,
  int last
) {
  while (last - first > 1) {
    int const middle = (first + last) / 2;
    if (entrySize < m_entries[middle].size) {
      first = middle;
    } else {
      last = middle;
    }
  }
  if (entrySize > m_entries[first].size) {
    m_entries.emplace(m_entries.begin() + first, entryPath, entrySize);
  } else if (entrySize < m_entries[last].size) {
    m_entries.emplace(m_entries.begin() + last + 1, entryPath, entrySize);
  } else {
    m_entries.emplace(m_entries.begin() + last, entryPath, entrySize);
  }
}

size_t FileSystemEntries::size() const {
  return m_entries.size();
}

Entry const &FileSystemEntries::operator[](size_t index) {
  return m_entries[index];
}
