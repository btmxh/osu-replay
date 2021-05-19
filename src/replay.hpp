#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay.hpp"
#include "io.hpp"
#include "stdfloat.hpp"

namespace osrp {
struct Replay {
 public:
  struct Frame {
    float32_t x, y;
    int64_t time;
    uint32_t keys;
  };

  explicit Replay(const fs::path& path);

  GameMode mode;
  int32_t version;
  std::string mapMd5, playerName, replayMd5;
  int16_t no_300s, no_100s, no_50s, no_gekis, no_katus, no_misses;
  int32_t score;
  int16_t maxCombo;
  int8_t fullCombo;
  int32_t mods;
  std::string lifeBar;
  int64_t timestamp;
  std::vector<Frame> replayData;
  int64_t scoreID;
  float64_t additionalModInfo;
  int32_t replaySeed;
};

}  // namespace osrp

