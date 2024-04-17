#include "error_codes.h"
#include "hamming.h"

void HammingCode::encodeChunk(bits& chunk) {
    uint32_t encoded_chunk_size = getEncodedChunkSize(chunk.size());
    uint8_t control_bits_amount = encoded_chunk_size - chunk.size();

    bits encoded_chunk(encoded_chunk_size);
    uint32_t offset = 2;
    // Placed value bits
    for (uint32_t i = 2; i < encoded_chunk_size; i++) {
        if (isPowerOfTwo(i + 1)) {
            offset++;
            continue;
        }
        encoded_chunk[i] = chunk[i - offset];
    }

    bits control_bits = calculateControlBits(encoded_chunk, control_bits_amount);

    // Set control bits
    for (uint8_t control_bit = 0; control_bit < control_bits_amount; control_bit++) {
        uint32_t control_pos = (1 << control_bit) - 1; // 2^(control_bit) - 1
        encoded_chunk[control_pos] = control_bits[control_bit];
    }

    chunk = encoded_chunk;
}

// Decodes encoded_chunk and returns true if decoded successfully, otherwise returns false
bool HammingCode::decodeChunk(bits& encoded_chunk, uint8_t control_bits_amount) {
    bits decoded_chunk(encoded_chunk.size() - control_bits_amount);
    bits control_bits(control_bits_amount);

    bits recalculated_control_bits = calculateControlBits(encoded_chunk, control_bits_amount);

    // If control bits are different and there's an error
    if (!compareControlBits(control_bits, recalculated_control_bits)) {
        uint16_t error_pos = 0;
        // Find error position
        for (uint8_t i = 0; i < control_bits_amount; i++) {
            error_pos += static_cast<uint16_t>(recalculated_control_bits[i]) << i;
        }
        encoded_chunk[error_pos - 1] = !encoded_chunk[error_pos - 1]; // Correct error
    }

    // Check if there's still an error
    recalculated_control_bits = calculateControlBits(encoded_chunk, control_bits_amount);

    if (!compareControlBits(control_bits, recalculated_control_bits)) {
        return false;
    }

    // Remove control bits from encoded_chunk
    for (uint8_t control_bit = 0; control_bit < control_bits_amount; control_bit++) {
        uint32_t control_pos = (1 << control_bit) - 1;
        encoded_chunk.erase(encoded_chunk.begin() + control_pos - control_bit);
    }

    return true;
}

// Encodes char by char
bits HammingCode::encodeString(const std::string& s) {
    bits result;
    for (char c : s) {
        bits char_bits = toBits(c);
        encodeChunk(char_bits);
        extendBits(result, char_bits);
    }
    return result;
}

std::string HammingCode::decodeString(const bits& encoded) {
    std::string s;
    for (int i = 0; i < encoded.size(); i += BITS_IN_ENCODED_BYTE) {
        bits char_chunk = std::vector<bool>(encoded.begin() + i, encoded.begin() + i + BITS_IN_ENCODED_BYTE);
        bool correct = decodeChunk(char_chunk, 4);
        if (!correct) {
            return std::string();
        }
        s += fromBits<char>(char_chunk);
    }
    return s;
}

bits HammingCode::calculateControlBits(const bits& extended_chunk,
                                       uint8_t control_bits_amount) {
    bits control_bits(control_bits_amount);

    for (uint8_t control_bit = 0; control_bit < control_bits_amount; control_bit++) {
        uint32_t control_pos = (1 << control_bit) - 1; // 2^(control_bit) - 1
        for (uint32_t i = control_pos; i < extended_chunk.size(); i += 2 * control_pos + 2) {
            for (uint32_t j = 0; j < control_pos + 1; j++) {
                control_bits[control_bit] = control_bits[control_bit] != extended_chunk[i + j]; // XOR
            }
        }
    }

    return control_bits;
}

bool HammingCode::compareControlBits(const bits& first, const bits& second) {
    return std::equal(first.begin(), first.end(), second.begin());
}

uint8_t HammingCode::getControlBitsAmount(uint16_t chunk_size) {
    return getLog2(chunk_size) + 1;
}

uint32_t HammingCode::getEncodedChunkSize(uint16_t chunk_size) {
    uint8_t control_bits_amount = getControlBitsAmount(chunk_size); // always <= 17
    uint32_t encoded_chunk_size = chunk_size + control_bits_amount;
    return encoded_chunk_size;
}

void HammingCode::extendBits(bits& a, const bits& b) {
    for (bool el : b) {
        a.push_back(el);
    }
}

bool HammingCode::isPowerOfTwo(uint32_t x) {
    return x && !(x & (x - 1));
}

uint8_t HammingCode::getLog2(uint16_t x) {
    uint8_t log = 0;
    while (x) {
        x >>= 1;
        log++;
    }
    return log - 1;
}
