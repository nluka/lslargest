#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <cstring>
#include "FileRanker.hpp"
#include "util.hpp"
#include "defs.hpp"

namespace fs = std::filesystem;

using lsl::File, lsl::FileRanker;

File::File(std::filesystem::path const &fsPath, uintmax_t const size)
: m_fsPath{fsPath}, m_size{size} {}

FileRanker::FileRanker(size_t const rootPathLength, size_t const max)
: m_rootPathLength{rootPathLength}, m_max{max}, m_entries{} {
  try {
    m_entries.reserve(max);
  } catch (...) {
    lsl::handle_bad_alloc();
  }
}

bool FileRanker::is_full() const {
  return m_entries.size() >= m_max;
}

bool FileRanker::is_overfilled() const {
  return m_entries.size() > m_max;
}

bool FileRanker::is_empty() const {
  return m_entries.size() == 0;
}

uintmax_t FileRanker::smallest_size() const {
  return is_empty() ? 0 : m_entries.back().m_size;
}

void FileRanker::insert(fs::path const &entryPath, uintmax_t const entrySize) {
  if (is_empty()) {
    m_entries.emplace_back(entryPath, entrySize);
    return;
  }

  binary_insert(entryPath, entrySize, 0, static_cast<int>(m_entries.size()) - 1);
  if (is_overfilled()) {
    m_entries.pop_back();
  }
}

void FileRanker::binary_insert(
  fs::path const &fsPath,
  uintmax_t const size,
  int first,
  int last
) {
  while (last - first > 1) {
    int const middle = (first + last) / 2;
    if (size < m_entries[middle].m_size) {
      first = middle;
    } else {
      last = middle;
    }
  }
  if (size > m_entries[first].m_size) {
    m_entries.emplace(m_entries.begin() + first, fsPath, size);
  } else if (size < m_entries[last].m_size) {
    m_entries.emplace(m_entries.begin() + last + 1, fsPath, size);
  } else {
    m_entries.emplace(m_entries.begin() + last, fsPath, size);
  }
}

size_t FileRanker::size() const {
  return m_entries.size();
}

File const &FileRanker::operator[](size_t const index) const {
  return m_entries[index];
}

void FileRanker::print(std::ostream &os) const {
  if (m_entries.size() == 0) {
    os << "No matches\n";
    return;
  }

  using std::left, std::setw;

  os << '\n'
    << "Rank     Size        Pathname\n"
    << "-------  ----------  --------\n";

  size_t rank = 1;
  for (auto const &entry : m_entries) {
    char formattedSize[11];
    ([&formattedSize](uintmax_t bytes){
      char const *sizes[] = { "B", "KB", "MB", "GB", "TB" };
      int sizeIdx;
      double bytesDbl = static_cast<double>(bytes);
      for (
        sizeIdx = 0;
        sizeIdx < 5 && bytes >= 1024;
        ++sizeIdx, bytes /= 1024
      ) bytesDbl = bytes / 1024.0;
      snprintf(formattedSize, sizeof formattedSize, "%.2f", bytesDbl);
      strcat(strcat(formattedSize, " "), sizes[sizeIdx]);
    })(entry.m_size);

    os
      << left << setw(7)  << rank++        << "  "
      << left << setw(10) << formattedSize << "  "
      << (entry.m_fsPath.string().c_str() + m_rootPathLength) << '\n';
  }
}
