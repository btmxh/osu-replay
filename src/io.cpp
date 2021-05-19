#include "io.hpp"

const bool osrp::IsLittleEndian = []() {
  int32_t i = 0x00000001;
  return reinterpret_cast<char*>(&i)[0];
}();

