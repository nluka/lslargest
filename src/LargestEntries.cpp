#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <string.h>
#include "LargestEntries.hpp"

namespace fs = std::filesystem;

Entry::Entry() : path{}, size{} {}

Entry::Entry(std::filesystem::path const & path, uintmax_t const size)
: path{path}, size{size} {}

LargestEntries::LargestEntries(size_t const rootPathLength, size_t const max)
: m_rootPathLength{rootPathLength}, m_max{max}, m_entries{} {
  m_entries.reserve(max);
}

bool LargestEntries::is_full() const {
  return m_entries.size() >= m_max;
}

bool LargestEntries::is_overfilled() const {
  return m_entries.size() > m_max;
}

bool LargestEntries::is_empty() const {
  return m_entries.size() == 0;
}

uintmax_t LargestEntries::smallest_size() const {
  if (is_empty()) {
    return 0;
  }
  return m_entries[m_entries.size() - 1].size;
}

void LargestEntries::insert(fs::path entryPath, uintmax_t entrySize) {
  if (is_empty()) {
    m_entries.emplace_back(entryPath, entrySize);
    return;
  }

  binary_insert(entryPath, entrySize, 0, static_cast<int>(m_entries.size()) - 1);
  if (is_overfilled()) {
    m_entries.pop_back();
  }
}

void LargestEntries::binary_insert(
  fs::path const & entryPath,
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

size_t LargestEntries::size() const {
  return m_entries.size();
}

Entry const & LargestEntries::operator[](size_t index) const {
  return m_entries[index];
}

void format_bytes(uintmax_t bytes, char * out, size_t outSize) {
  char const * sizes[] = { "B", "KB", "MB", "GB", "TB" };
  int i;
  double bytesDbl = static_cast<double>(bytes);
  for (
    i = 0;
    i < 5 && bytes >= 1024;
    ++i, bytes /= 1024
  ) {
    bytesDbl = bytes / 1024.0;
  }
  snprintf(out, outSize, "%.2f", bytesDbl);
  strcat(strcat(out, " "), sizes[i]);
}

void LargestEntries::display(std::ostream &os) const {
  if (m_entries.size() == 0) {
    os << "No matches" << std::endl;
    return;
  }

  os
    << "--------------------------\n"
    << "RANK  SIZE        PATHNAME\n";

  size_t rank = 1;
  for (auto const & entry : m_entries) {
    char formattedBytesStr[11];
    format_bytes(
      entry.size,
      formattedBytesStr,
      sizeof formattedBytesStr
    );

    os
      << std::setw(4) << std::left << rank++ << "  "
      << std::setw((sizeof formattedBytesStr) - 1) << std::left
        << formattedBytesStr << "  "
      << (entry.path.string().c_str() + m_rootPathLength) << '\n';
  }

  os << "--------------------------" << std::endl;
}
