#include "archive.h"
#include "error_codes.h"
#include <algorithm>

CorrectingArchive::CorrectingArchive(std::string archive_path)
    : archive_path(archive_path) {
    archive = std::ofstream();
    getNumberOfFiles();
}

CorrectingArchive::~CorrectingArchive() {
    if (archive.is_open()) {
        archive_stream.close();
        setNumberOfFiles();
        archive.close();
    }
}

void CorrectingArchive::invalidArchive() {
    std::cerr << "Invalid archive" << '\n';
    exit(ERROR_INVALID_ARHIVE);
}

void CorrectingArchive::invalidFile(const std::string& file_path) {
    std::cerr << "Invalid file " << file_path << '\n';
    exit(ERROR_INVALID_FILE_PATH);
}

uint8_t CorrectingArchive::getPaddingBitsAmount(uint64_t file_size, uint16_t chunk_size) {
    uint32_t encoded_chunk_size = HammingCode::getEncodedChunkSize(chunk_size);
    chunk_size /= BITS_IN_BYTE;
    uint64_t chunk_amount = file_size / chunk_size + 1 * static_cast<bool>(file_size % chunk_size);
    uint8_t padding_bits_amount = (BITS_IN_BYTE - ((chunk_amount * encoded_chunk_size) % BITS_IN_BYTE)) % BITS_IN_BYTE;
    return padding_bits_amount;
}

void CorrectingArchive::createEmptyArchive() {
    archive.open(archive_path, std::ios::out | std::ios::binary);
    if (!archive.is_open()) {
        invalidArchive();
    }

    files_number = 0;
    bits header(EXTENDED_HEADER_SIZE); // Zero files in empty archive
    archive_stream.write(header);

    archive_stream.close();
}

void CorrectingArchive::getNumberOfFiles() {
    if (!archiveExists()) {
        files_number = 0;
        return;
    }

    // TODO: extract opening to read to function and add class field read_archive_stream
    std::ifstream read_archive(archive_path, std::ios::in | std::ios::binary);
    if (!read_archive.is_open()) {
        invalidArchive();
    }

    bitReader read_stream(read_archive);

    bits header = read_stream.read(HEADER_SIZE);
    read_archive.close();

    HammingCode::decodeChunk(header, FILES_NUMBER_CONTROL_BITS);
    uint16_t new_files_number = fromBits<uint16_t>(header);

    files_number = new_files_number;
}

void CorrectingArchive::setNumberOfFiles() {
//    archive.seekp(0); // Go to header
    archive_stream.close(); // Go to header
    bits number = toBits(files_number);
    HammingCode::encodeChunk(number);
    archive_stream.write(number);
    archive_stream.close();
}
void CorrectingArchive::printBits(bits& bts, std::string message) {
    for (auto bit : bts) {
        std::cerr << bit;
    }
    std::cerr << ' ' << message << '\n';
}

void CorrectingArchive::appendFile(std::string& file_path, uint16_t chunk_size) {
    openArchiveToWrite();
    struct FileHeader file_header = writeFileHeader(file_path, chunk_size);

    std::ifstream file;
    file.open(file_path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        invalidFile(file_path);
    }
    bitReader file_stream(file);
    bits chunk;
    while (!file_stream.eof()) {
        chunk = file_stream.read(chunk_size);
        HammingCode::encodeChunk(chunk);
        archive_stream.write(chunk);
    }
    file.close();

    archive_stream.write(bits(file_header.padding));

    files_number++;
}

std::vector<std::string> CorrectingArchive::listFiles() {
    getNumberOfFiles();
    // TODO: extract opening to read to function and add class field read_archive_stream
    std::ifstream read_archive(archive_path, std::ios::in | std::ios::binary);
    if (!read_archive.is_open()) {
        invalidArchive();
    }

    bitReader read_stream(read_archive);

    read_stream.skip(EXTENDED_HEADER_SIZE);
    std::vector<std::string> files;
    for (uint16_t i = 0; i < files_number; i++) {
        struct FileHeader file_header = readFileHeader(read_stream);
        files.push_back(file_header.file_name);
        nextFile(file_header, read_stream);
    }
    return files;
}

void CorrectingArchive::extractFile(bitReader& read_stream, struct FileHeader file_header, std::string& path) {
    std::ofstream file;
    file.open(path + '/' + file_header.file_name, std::ios::in | std::ios::trunc | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Invalid path\n";
        exit(4);
    }
    bitWriter file_stream(file);
    uint64_t file_length = getEncodedFileLength(file_header) - file_header.padding;
    uint32_t encoded_chunk_size = HammingCode::getEncodedChunkSize(file_header.chunk_size);
    bits chunk;
    while (file_length) {
        chunk = readChunk(read_stream,
                          encoded_chunk_size,
                          HammingCode::getControlBitsAmount(file_header.chunk_size));
        file_length -= encoded_chunk_size;
        file_stream.write(chunk);
    }
    read_stream.skip(file_header.padding);
    file_stream.close();
    file.close();
}

uint64_t CorrectingArchive::getEncodedFileLength(const FileHeader& file_header) {
    uint8_t control_bits_per_chunk = HammingCode::getControlBitsAmount(file_header.chunk_size);
    uint64_t file_length =
        file_header.file_size / file_header.chunk_size * control_bits_per_chunk // Amount of all control bits
            + file_header.file_size
            + file_header.padding;
    return file_length;
}

void CorrectingArchive::extractFiles(std::vector<std::string>& file_names, std::string& path) {
    std::ifstream read_archive(archive_path, std::ios::in | std::ios::binary);
    if (!read_archive.is_open()) {
        invalidArchive();
    }

    bitReader read_stream(read_archive);
    std::ofstream file;
    read_stream.skip(EXTENDED_HEADER_SIZE);
    for (uint16_t i = 0; i < files_number; i++) {
        struct FileHeader file_header = readFileHeader(read_stream);
        if (std::find(file_names.begin(), file_names.end(), file_header.file_name) != file_names.end()) {
            extractFile(read_stream, file_header, path);
        } else {
            nextFile(file_header, read_stream);
        }
    }
}

void CorrectingArchive::extractAllFiles(std::string& path) {
    std::ifstream read_archive(archive_path, std::ios::in | std::ios::binary);
    if (!read_archive.is_open()) {
        invalidArchive();
    }

    bitReader read_stream(read_archive);
    std::ofstream file;
    read_stream.skip(EXTENDED_HEADER_SIZE);
    for (uint16_t i = 0; i < files_number; i++) {
        struct FileHeader file_header = readFileHeader(read_stream);
        extractFile(read_stream, file_header, path);
    }
}

bool CorrectingArchive::archiveExists() {
    std::ifstream arch(archive_path);
    if (arch.good()) {
        arch.close();
        return true;
    }
    return false;
}

void CorrectingArchive::openArchiveToWrite() {
    if (archive.is_open()) {
        archive.close();
    } else if (!archiveExists()) {
        createEmptyArchive();
    }
    archive.open(archive_path, std::ios::ate | std::ios::binary | std::ios::in);
}

struct FileHeader CorrectingArchive::writeFileHeader(const std::string& file_path, uint16_t chunk_size) {
    std::filesystem::path p = file_path;
    if (!std::filesystem::exists(p)) {
        invalidFile(file_path);
    }
    uint64_t file_size = std::filesystem::file_size(p) * BITS_IN_BYTE;

    bits chunk_size_block = toBits(chunk_size);
    HammingCode::encodeChunk(chunk_size_block);
    archive_stream.write(chunk_size_block);

    bits file_size_block = toBits(file_size);
    HammingCode::encodeChunk(file_size_block);
    archive_stream.write(file_size_block);

    uint8_t padding_bits_amount = getPaddingBitsAmount(file_size, chunk_size);
    bits padding_amount_block = toBits(padding_bits_amount);
    HammingCode::encodeChunk(padding_amount_block);
    archive_stream.write(padding_amount_block);

    std::string file_name = file_path.substr(file_path.find_last_of("/\\") + 1);
    file_name.resize(150, static_cast<char>(0));
    bits encoded_file_name = HammingCode::encodeString(file_name);
    archive_stream.write(encoded_file_name);

    struct FileHeader file_header;
    file_header.chunk_size = chunk_size;
    file_header.file_size = file_size;
    file_header.padding = padding_bits_amount;
    file_header.file_name = file_name;
    return file_header;
}

struct FileHeader CorrectingArchive::readFileHeader(bitReader& read_stream) const {
    bits chunk_size_bits = readChunk(read_stream, CHUNK_SIZE_INFO, CHUNK_SIZE_CONTROL_BITS);
    auto chunk_size = fromBits<uint16_t>(chunk_size_bits);

    bits file_size_bits = readChunk(read_stream, FILE_SIZE_INFO, FILE_SIZE_CONTROL_BITS);
    auto file_size = fromBits<uint64_t>(file_size_bits);

    bits padding_bits = readChunk(read_stream, PADDING_INFO, PADDING_CONTROL_BITS);

    auto padding = fromBits<uint8_t>(padding_bits);

    std::string file_name = readString(read_stream, FILE_NAME_INFO);

    struct FileHeader file_header = {chunk_size, file_size, padding, file_name};
    return file_header;
}

void CorrectingArchive::nextFile(struct FileHeader& file_header, bitReader& read_stream) {
    uint64_t file_length = getEncodedFileLength(file_header);
    read_stream.skip(file_length);
}

bits CorrectingArchive::readChunk(bitReader& read_stream,
                                  uint32_t encoded_chunk_size,
                                  uint8_t control_bits_amount) {
    bits chunk = read_stream.read(encoded_chunk_size);
    bool correct = HammingCode::decodeChunk(chunk, control_bits_amount);
    if (!correct) {
        invalidArchive();
    }
    return chunk;
}

std::string CorrectingArchive::readString(bitReader& read_stream, uint32_t string_size) {
    std::string s = HammingCode::decodeString(read_stream.read(string_size));
    if (s.empty()) {
        invalidArchive();
    }
    auto it = s.find(static_cast<char>(0)); // Strip \0 chars at the end
    s.erase(s.begin() + it, s.end());
    return s;
}

uint32_t CorrectingArchive::getStringControlBitsAmount(uint32_t string_size) {
    return string_size * HammingCode::getControlBitsAmount(BITS_IN_BYTE);
}
