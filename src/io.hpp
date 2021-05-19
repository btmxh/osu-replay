#pragma once

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace osrp {
namespace fs = std::filesystem;

template <typename FStrType>
inline void Open(FStrType& stream, const fs::path& path,
                 std::ios::openmode mode = std::ios::in | std::ios::out) {
  auto pathstr = path.string();
  stream.open(pathstr, mode);
}

template <typename Func,
          typename = std::enable_if_t<std::is_invocable_v<Func, std::string>>>
inline void ReadLines(const fs::path& path, Func f) {
  std::ifstream ifstr;
  Open(ifstr, path, std::ios::in);

  std::string line;
  while (std::getline(ifstr, line)) {
    f(line);
  }
}

extern const bool IsLittleEndian;

template <typename CharT = char,
          typename = std::enable_if_t<sizeof(CharT) == 1>>
inline std::vector<CharT> ReadBytes(std::istream& stream, size_t size) {
  std::vector<CharT> value;
  value.resize(size);
  stream.read(reinterpret_cast<char*>(value.data()), size);
  return value;
}

template <typename T>
inline T ReadBinary(std::istream& stream) {
  T value;
  char* ptr = reinterpret_cast<char*>(&value);
  stream.read(ptr, sizeof(T));
  if (!IsLittleEndian) {
    std::reverse(ptr, ptr + sizeof(T));
  }
  return value;
}

inline uintmax_t ReadULEB128(std::istream& stream) {
  uint8_t byte;
  uintmax_t value = 0;
  int shift = 0;
  do {
    stream.read(reinterpret_cast<char*>(&byte), 1);
    value |= (byte & 0x7f) << shift;
    shift += 7;
  } while (byte & 0x80);
  return value;
}

template <>
inline std::string ReadBinary(std::istream& stream) {
  auto flag = ReadBinary<uint8_t>(stream);
  if (flag) {
    assert(flag == 0x0b);
    size_t length = ReadULEB128(stream);
    std::string value(length, '\0');
    stream.read(value.data(), length);
    return value;
  } else {
    return "";
  }
}

}  // namespace osrp
