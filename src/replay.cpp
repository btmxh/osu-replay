#include "replay.hpp"

#include <iostream>
#include <memory>

#include "../lzma/LzmaDec.h"
#include "strings.hpp"

void* SzAlloc(ISzAllocPtr p, size_t size) {
  p = p;
  return malloc(size);
}
void SzFree(ISzAllocPtr p, void* address) {
  p = p;
  free(address);
}
ISzAlloc alloc = {SzAlloc, SzFree};

std::unique_ptr<uint8_t[]> decompressLZMA(uint8_t* src, size_t src_size,
                                          size_t& dst_size) {
  if (src_size < 13) throw std::runtime_error("invalid LZMA header");

  UInt64 size = 0;
  for (int i = 0; i < 8; i++) {
    size |= (src[5 + i] << (i * 8));
  }

  auto dst = std::make_unique<uint8_t[]>(size);

  ELzmaStatus lzmaStatus;
  SizeT procOutSize = size, procInSize = src_size - 13;
  auto status = LzmaDecode(dst.get(), &procOutSize, &src[13], &procInSize, src,
                           5, LZMA_FINISH_END, &lzmaStatus, &alloc);

  if (status == SZ_OK && procOutSize == size) {
    dst_size = size;
    return dst;
  }

  throw std::runtime_error("An error occurred while decompressing LZMA data");
}
namespace osrp {
Result<Replay::Frame> ParseFrame(const std::string_view& str) {
  Replay::Frame frame;
  int tokenIndex = 0;
  std::error_code err{};
  Split(str, '|', [&](const auto& token) {
#define PARSE(x)                                           \
  if (auto result = ParseString<decltype(frame.x)>(token); \
      result.HasValue()) {                                 \
    frame.x = result.Value();                              \
  } else {                                                 \
    err = result.Error();                                  \
  }
    switch (tokenIndex++) {
      case 0:
        PARSE(time);
      case 1:
        PARSE(x);
      case 2:
        PARSE(y);
      case 3:
        PARSE(keys);
    }
  });

  return err == std::error_code() ? Result<Replay::Frame>(frame)
                                  : Result<Replay::Frame>(err);
}

Replay::Replay(const fs::path& path) {
  std::ifstream input;
  Open(input, path, std::ios::in | std::ios::binary);

#define READ(x) x = ReadBinary<decltype(x)>(input)

  mode = static_cast<GameMode>(ReadBinary<uint8_t>(input));
  READ(version);
  READ(mapMd5);
  READ(playerName);
  READ(replayMd5);
  READ(no_300s);
  READ(no_100s);
  READ(no_50s);
  READ(no_gekis);
  READ(no_katus);
  READ(no_misses);
  READ(score);
  READ(maxCombo);
  READ(fullCombo);
  READ(mods);
  READ(lifeBar);
  READ(timestamp);

  size_t compressedSize = ReadBinary<int32_t>(input);
  auto compressedData = ReadBytes<uint8_t>(input, compressedSize);

  size_t decompressedSize;
  auto decompressedData =
      decompressLZMA(compressedData.data(), compressedSize, decompressedSize);

  std::string_view replayDataString(
      reinterpret_cast<char*>(decompressedData.get()), decompressedSize);

  Split(replayDataString, ',', [&](const std::string_view& str) {
    if (TrimWhitespace(str).size() <= 0) return;
    if (auto seedString = RemovePrefix(str, "-12345|0|0|");
        seedString.has_value()) {
      auto seedStringValue = seedString.value();
      seedStringValue.remove_suffix(1);
      auto seedResult = ParseString<decltype(replaySeed)>(seedStringValue);
      if (seedResult) {
        replaySeed = seedResult.Value();
      } else {
        std::cerr << "Replay seed parsing unsuccessfully. Error: "
                  << seedResult.Error().message() << std::endl;
      }
    } else {
      if (auto frame = ParseFrame(str); frame.HasValue()) {
        replayData.push_back(frame.Value());
      }
    }
  });
}
}  // namespace osrp

