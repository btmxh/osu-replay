#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "io.hpp"
#include "result.hpp"
#include "strings.hpp"

namespace osrp {

enum class KeyValueSection { GENERAL, EDITOR, METADATA, DIFFICULTY, COLORS };

enum class CommaSeparatedSection { EVENTS, TIMING_POINTS, HIT_OBJECTS };

class Beatmap {
 public:
  explicit Beatmap(const fs::path& path);

  template <typename T = std::string_view>
  Result<T> GetProperty(KeyValueSection section, const std::string_view& key) {
    auto string = GetPropertyString(section, key);
    return string.Map(
        [](const std::string_view& str) { return ParseString<T>(str); });
  }

  template <typename T>
  void SetProperty(KeyValueSection section, const std::string_view& key,
                   T value) {
    SetPropertyString(section, key, ToString(value));
  }

  const std::vector<std::vector<std::string>>& GetCommaSeparatedValues(
      CommaSeparatedSection section) const;

 private:
  fs::path path;
  std::string version;

  std::map<KeyValueSection, std::map<std::string, std::string>>
      keyValueSections;
  std::map<CommaSeparatedSection, std::vector<std::vector<std::string>>>
      commaSeparatedSections;

  Result<std::string_view> GetPropertyString(KeyValueSection section,
                                             const std::string_view& key) const;
  void SetPropertyString(KeyValueSection section, const std::string_view& key,
                         const std::string_view& value);
};
}  // namespace osrp
