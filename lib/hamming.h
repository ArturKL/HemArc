#pragma once

#include "bitstream.h"
#include <string>

using Bits::bits;
using Bits::BITS_IN_BYTE;
using Bits::toBits;
using Bits::fromBits;

class HammingCode {
 public:
  static void encodeChunk(bits&);

  static bool decodeChunk(bits& encoded_chunk, uint8_t);

  static bits encodeString(const std::string& s);

  static std::string decodeString(const bits& encoded);

  static uint8_t getControlBitsAmount(uint16_t chunk_size);

  static uint32_t getEncodedChunkSize(uint16_t chunk_size);

 private:

  static const uint8_t BITS_IN_ENCODED_BYTE = 12; // char is 8 bits + 4 control bits

  [[nodiscard]] static bits calculateControlBits(const bits& chunk, uint8_t);

  static bool compareControlBits(const bits& first, const bits& second);

  static void extendBits(bits&, const bits&);

  static uint8_t getLog2(uint16_t);

  static bool isPowerOfTwo(uint32_t);
};
