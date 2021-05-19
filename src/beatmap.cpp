#include "beatmap.hpp"

#include <iostream>

namespace osrp {

Beatmap::Beatmap(const fs::path& path) : path(path) {
  enum class SectionType { NO_SECTION, KEY_VALUE, COMMA_SEPARATED };

  SectionType currentSection = SectionType::NO_SECTION;
  KeyValueSection currentKVSection = KeyValueSection::GENERAL;
  CommaSeparatedSection currentCSSection = CommaSeparatedSection::EVENTS;
  std::optional<std::string> mapVersion;

  ReadLines(path, [&](const std::string_view& line) {
    const auto trimmed = TrimWhitespace(line);
    if (trimmed.empty()) return;
    if (trimmed[0] == '[') {
      const auto section = trimmed.substr(1, trimmed.size() - 2);
      auto setKV = [&](KeyValueSection s) {
        currentSection = SectionType::KEY_VALUE;
        currentKVSection = s;
      };
      auto setCS = [&](CommaSeparatedSection s) {
        currentSection = SectionType::COMMA_SEPARATED;
        currentCSSection = s;
      };
      if (section == "General") {
        setKV(KeyValueSection::GENERAL);
      } else if (section == "Editor") {
        setKV(KeyValueSection::EDITOR);
      } else if (section == "Metadata") {
        setKV(KeyValueSection::METADATA);
      } else if (section == "Difficulty") {
        setKV(KeyValueSection::DIFFICULTY);
      } else if (section == "Colours") {
        setKV(KeyValueSection::COLORS);
      } else if (section == "Events") {
        setCS(CommaSeparatedSection::EVENTS);
      } else if (section == "TimingPoints") {
        setCS(CommaSeparatedSection::TIMING_POINTS);
      } else if (section == "HitObjects") {
        setCS(CommaSeparatedSection::HIT_OBJECTS);
      } else {
        std::cerr << "Invalid section found: " << section << std::endl;
      }
    } else {
      switch (currentSection) {
        case SectionType::NO_SECTION: {
          auto version = RemovePrefix(trimmed, "osu file format");
          if (version.has_value()) {
            mapVersion = version.value();
          }
          break;
        }
        case SectionType::KEY_VALUE: {
          auto indexOfFirstColon = trimmed.find(':');
          if (indexOfFirstColon != std::string_view::npos) {
            auto key = TrimWhitespace(trimmed.substr(0, indexOfFirstColon));
            auto value = TrimWhitespace(trimmed.substr(indexOfFirstColon + 1));
            SetPropertyString(currentKVSection, key, value);
          }
          break;
        }
        case SectionType::COMMA_SEPARATED: {
          size_t begin = 0;
          size_t indexOfComma;
          std::vector<std::string> v;
          while ((indexOfComma = trimmed.find(',', begin)) !=
                 std::string_view::npos) {
            v.push_back(std::string(trimmed.substr(begin, indexOfComma)));
            begin = indexOfComma + 1;
          }
          v.push_back(std::string(trimmed.substr(begin)));
          commaSeparatedSections[currentCSSection].push_back(v);
        }
      }
    }
  });

  if (!mapVersion.has_value()) {
    version = "14";
    std::cerr << "Beatmap version not found. Assumed to be .osu v14"
              << std::endl;
  } else {
    version = mapVersion.value();
  }
}

struct BeatmapPropertyNotFound : public std::error_category {
  const char* name() const noexcept override {
    return "beatmap_property_not_found";
  }
  std::string message(int err) const override { return name(); }
};

const BeatmapPropertyNotFound bpnfCategory;

Result<std::string_view> Beatmap::GetPropertyString(
    KeyValueSection section, const std::string_view& key) const {
  auto it = keyValueSections.find(section);
  if (it == keyValueSections.end()) {
    throw std::logic_error("invalid section");
  } else {
    const auto& map = it->second;
    auto it2 = map.find(std::string(key));
    if (it2 == map.end()) {
      return Result<std::string_view>(std::error_code(1, bpnfCategory));
    } else {
      return Result<std::string_view>(it2->second);
    }
  }
}

void Beatmap::SetPropertyString(KeyValueSection section,
                                const std::string_view& key,
                                const std::string_view& value) {
  keyValueSections[section][std::string(key)] = value;
}

const std::vector<std::vector<std::string>>& Beatmap::GetCommaSeparatedValues(
    CommaSeparatedSection section) const {
  auto it = commaSeparatedSections.find(section);
  if (it == commaSeparatedSections.end()) {
    throw std::logic_error("invalid section");
  } else {
    return it->second;
  }
}

}  // namespace osrp

