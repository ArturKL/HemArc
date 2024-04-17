#include <algorithm>
#include <cstdint>
#include <fstream>
#include <vector>

namespace Bits {
using bit = bool;
using bits = std::vector<bit>;

constexpr uint8_t BITS_IN_BYTE = 8;
constexpr size_t BITS_IN_CHAR = BITS_IN_BYTE;

template<typename T>
bits toBits(T element) {
    size_t result_size = BITS_IN_BYTE * sizeof(T);
    bits result(result_size);
    for (size_t pos = 0; pos < result_size; pos++) {
        result[result_size - pos - 1] = element & (1ll << pos);
    }
    return result;
}

template<typename T>
T fromBits(const bits& bts) {
    T result = 0;
    size_t result_size = BITS_IN_BYTE * sizeof(T);
    for (std::size_t pos = 0; pos < result_size; ++pos) {
        result |= (static_cast<uint64_t>(bts[result_size - pos - 1]) << pos);
    }
    return result;
}

struct bitReader {
 public:
  explicit bitReader(std::istream&);

  bit readBit();

  void refresh();

  unsigned char readChar();

  bits read(uint32_t);

  void skip(uint64_t);

  bool eof();

// private:
  static const constexpr size_t BUFF_SIZE = 1 << 16;
  unsigned char buff[BUFF_SIZE];
  size_t pos = 0;
  size_t read_chars_amount = 0;
  std::istream& in;
  void fillBuff();
};

struct bitWriter {
 public:
  explicit bitWriter(std::ofstream&);

  void writeBit(bit);

  void write(const bits&);

  void close();

// private:
  static const size_t BUFF_SIZE = 1 << 15;
  unsigned char buff[BUFF_SIZE];
  static const size_t BITS_IN_BUFF = BITS_IN_CHAR * sizeof(buff);
  size_t pos = 0;
  std::ofstream& out;
};
} // namespace Bits
