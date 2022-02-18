#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <string.h>
#include "FileTracker.hpp"

namespace fs = std::filesystem;

File::File() : fsPath{}, size{} {}

File::File(std::filesystem::path const &fsPath, uintmax_t const size)
: fsPath{fsPath}, size{size} {}

FileTracker::FileTracker(size_t const rootPathLength, size_t const max)
: m_rootPathLength{rootPathLength}, m_max{max}, m_entries{} {
  m_entries.reserve(max);
}

bool FileTracker::is_full() const {
  return m_entries.size() >= m_max;
}

bool FileTracker::is_overfilled() const {
  return m_entries.size() > m_max;
}

bool FileTracker::is_empty() const {
  return m_entries.size() == 0;
}

uintmax_t FileTracker::smallest_size() const {
  return is_empty() ? 0 : m_entries.back().size;
}

void FileTracker::insert(fs::path const &entryPath, uintmax_t const entrySize) {
  if (is_empty()) {
    m_entries.emplace_back(entryPath, entrySize);
    return;
  }

  binary_insert(entryPath, entrySize, 0, static_cast<int>(m_entries.size()) - 1);
  if (is_overfilled()) {
    m_entries.pop_back();
  }
}

void FileTracker::binary_insert(
  fs::path const &fsPath,
  uintmax_t const size,
  int first,
  int last
) {
  while (last - first > 1) {
    int const middle = (first + last) / 2;
    if (size < m_entries[middle].size) {
      first = middle;
    } else {
      last = middle;
    }
  }
  if (size > m_entries[first].size) {
    m_entries.emplace(m_entries.begin() + first, fsPath, size);
  } else if (size < m_entries[last].size) {
    m_entries.emplace(m_entries.begin() + last + 1, fsPath, size);
  } else {
    m_entries.emplace(m_entries.begin() + last, fsPath, size);
  }
}

size_t FileTracker::size() const {
  return m_entries.size();
}

File const &FileTracker::operator[](size_t const index) const {
  return m_entries[index];
}

void format_bytes(uintmax_t bytes, char *const out, size_t const outSize) {
  char const *sizes[] = { "B", "KB", "MB", "GB", "TB" };
  int i;
  double bytesDbl = static_cast<double>(bytes);
  for (i = 0; i < 5 && bytes >= 1024; ++i, bytes /= 1024) {
    bytesDbl = bytes / 1024.0;
  }
  snprintf(out, outSize, "%.2f", bytesDbl);
  strcat(strcat(out, " "), sizes[i]);
}

void FileTracker::printContents(std::ostream &os) const {
  if (m_entries.size() == 0) {
    os << "No matches\n";
    return;
  }

  os
    << "-----------------------------------\n"
    << "Rank  Size        Relative Pathname\n"
    << "----  ----------  -----------------\n";

  size_t rank = 1;
  for (auto const &entry : m_entries) {
    char formattedBytesStr[11];

    format_bytes(
      entry.size,
      formattedBytesStr,
      sizeof formattedBytesStr
    );

    os << std::setw(4) << std::left << rank++ << "  "
      << std::setw(10) << std::left << formattedBytesStr
      << "  " << (entry.fsPath.string().c_str() + m_rootPathLength) << '\n';
  }

  os << "-----------------------------------\n";
}
