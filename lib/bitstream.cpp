#include "bitstream.h"
#include <cstring>
#include <iostream>

Bits::bitReader::bitReader(std::istream& in) : in(in) {
    std::fill(buff, buff + BUFF_SIZE, 0);
}

Bits::bits Bits::bitReader::read(uint32_t n) {
    bits result(n);
    for (size_t i = 0; i < n; ++i) {
        result[i] = readBit();
    }
    return result;
}

Bits::bit Bits::bitReader::readBit() {
    if (eof()) {
        throw std::out_of_range("Trying to read, when nothing to read.");
    }

    bit result = buff[pos / BITS_IN_CHAR] & (1 << (pos % BITS_IN_CHAR));
    pos++;
    return result;
}

bool Bits::bitReader::eof() {
    fillBuff();
    return pos == read_chars_amount * BITS_IN_CHAR && in.eof();
}

void Bits::bitReader::fillBuff() {
    if (pos == read_chars_amount * BITS_IN_CHAR) {
        in.read(reinterpret_cast<char*>(buff), BUFF_SIZE);
        read_chars_amount = in.gcount();
        pos = 0;
    }
}

void Bits::bitReader::refresh() {
    in.clear();
    in.seekg(0);
    pos = 0;
    read_chars_amount = 0;
    std::memset(buff, 0, sizeof(buff));
}

void Bits::bitReader::skip(uint64_t skip_bits) {
    // Not working fast skip

//    std::streampos tpos = in.tellg(); // Position in file
//    std::streampos sbuff_size = static_cast<std::streampos>(BUFF_SIZE);
//    std::streampos cur_pos = static_cast<std::streampos>(pos / BITS_IN_CHAR) + tpos / sbuff_size; // Position of last read char in file
//    size_t pos_in_char = (pos % BITS_IN_CHAR) + (skip_bits % BITS_IN_CHAR);
//    std::streampos skip_chars = static_cast<std::streampos>(skip_bits / BITS_IN_CHAR + pos_in_char / BITS_IN_CHAR);
//    std::streampos new_pos = cur_pos + skip_chars;
//    in.seekg(new_pos);
//    pos = 0;
//    read_chars_amount = 0;
//    std::memset(buff, 0, sizeof(buff));
//    fillBuff();

    // Working slow skip
    read(skip_bits);

}

Bits::bitWriter::bitWriter(std::ofstream& out) : out(out) {
    std::fill(buff, buff + BUFF_SIZE, 0);
}

void Bits::bitWriter::write(const bits& bs) {
    for (auto b : bs) {
        writeBit(b);
    }
}

void Bits::bitWriter::writeBit(Bits::bit b) {
    buff[pos / BITS_IN_CHAR] |= (b << (pos % BITS_IN_CHAR));
    ++pos;

    if (pos == BITS_IN_BUFF) {
        out.write(reinterpret_cast<char*>(buff), BUFF_SIZE);
        pos = 0;
        std::memset(buff, 0, sizeof(buff));
    }
}

void Bits::bitWriter::close() {
    if (pos) {
        out.write(reinterpret_cast<char*>(buff),
                  (pos + BITS_IN_CHAR - 1) / BITS_IN_CHAR);
    }
    out.flush();
    out.clear();
    out.seekp(0);
    pos = 0;
    std::memset(buff, 0, sizeof(buff));
}
