#pragma once

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

#include "result.hpp"

namespace osrp {

inline std::string_view TrimWhitespace(const std::string_view& str) {
  auto begin = str.begin();
  auto end = str.end();
  while (isspace(*begin) && begin <= end) begin++;
  while (isspace(*(end - 1)) && begin <= end) end--;
  return std::string_view(begin, static_cast<size_t>(end - begin));
}

template <typename T>
inline Result<T> ParseString(const std::string_view& str) {
  std::string_view trimmed = TrimWhitespace(str);
  T value;
  auto [ptr, errc] =
      std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), value);
  if (errc == std::errc::invalid_argument ||
      errc == std::errc::result_out_of_range) {
    return Result<T>(std::make_error_code(errc));
  } else {
    return Result<T>(value);
  }
}

template <>
inline Result<std::string_view> ParseString(const std::string_view& str) {
  return Result<std::string_view>(str);
}

// gcc-10 doesn't support std::from_chars for floating-point types
template<>
inline Result<float> ParseString(const std::string_view& str) {
  return Result<float>(std::stof(std::string(TrimWhitespace(str))));
}

template <typename T>
std::string ToString(T value) {
  return std::to_string(value);
}

inline std::optional<std::string_view> RemovePrefix(
    const std::string_view& str, const std::string_view& prefix) {
  auto prefixLength = prefix.size();
  if (str.substr(0, prefixLength) == prefix) {
    return str.substr(prefixLength);
  } else {
    return std::nullopt;
  }
}

template <typename Func, typename = std::enable_if_t<
                             std::is_invocable_v<Func, std::string_view>>>
inline void Split(const std::string_view& str, char delim, Func func) {
  size_t begin = 0;
  size_t indexOfDelim;

  while ((indexOfDelim = str.find(delim, begin)) != std::string_view::npos) {
    func(str.substr(begin, indexOfDelim - begin));
    begin = indexOfDelim + 1;
  }

  func(str.substr(begin));
}

}  // namespace osrp
