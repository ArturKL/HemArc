#pragma once

#include "hamming.h"
#include <fstream>
#include <iostream>
#include <string>

using Bits::bitReader;
using Bits::bitWriter;

/*
    Archive starts with the number of files stored in archive (2 bytes)
    Before each file next information is stored:
        chunk size: 2 bytes
        file size: 8 bytes
        padding bits amount: 1 byte
        file name: max 100 characters (100 bytes)
*/
class CorrectingArchive {
 public:
  explicit CorrectingArchive(std::string archive_path);
  ~CorrectingArchive();

  void createEmptyArchive();

  void appendFile(std::string& file_path, uint16_t chunk_size);

  void extractAllFiles(std::string& path);

  void extractFiles(std::vector<std::string>& file_names, std::string& path);

  std::vector<std::string> listFiles();

  void deleteFile(std::string& file_name);

 private:
  std::string const archive_path;
  std::ofstream archive;
  Bits::bitWriter archive_stream = Bits::bitWriter(archive);

  uint16_t files_number;
  const uint8_t FILES_NUMBER_SIZE = 2 * BITS_IN_BYTE;
  const uint8_t FILES_NUMBER_CONTROL_BITS = HammingCode::getControlBitsAmount(FILES_NUMBER_SIZE);

  const uint8_t HEADER_SIZE = FILES_NUMBER_SIZE + FILES_NUMBER_CONTROL_BITS;

  const uint8_t ARCHIVE_HEADER_PADDING = BITS_IN_BYTE - (FILES_NUMBER_SIZE + FILES_NUMBER_CONTROL_BITS) % BITS_IN_BYTE;
  const uint8_t EXTENDED_HEADER_SIZE = FILES_NUMBER_SIZE + FILES_NUMBER_CONTROL_BITS + ARCHIVE_HEADER_PADDING;


  const uint8_t CHUNK_SIZE_BITS = 2 * BITS_IN_BYTE;
  const uint8_t CHUNK_SIZE_CONTROL_BITS = HammingCode::getControlBitsAmount(CHUNK_SIZE_BITS);
  const uint8_t CHUNK_SIZE_INFO = CHUNK_SIZE_BITS + CHUNK_SIZE_CONTROL_BITS;

  const uint8_t FILE_SIZE_BITS = 8 * BITS_IN_BYTE;
  const uint8_t FILE_SIZE_CONTROL_BITS = HammingCode::getControlBitsAmount(FILE_SIZE_BITS);
  const uint8_t FILE_SIZE_INFO = FILE_SIZE_BITS + FILE_SIZE_CONTROL_BITS;

  const uint8_t PADDING_BITS = 1 * BITS_IN_BYTE;
  const uint8_t PADDING_CONTROL_BITS = HammingCode::getControlBitsAmount(PADDING_BITS);
  const uint8_t PADDING_INFO = PADDING_BITS + PADDING_CONTROL_BITS;

  const uint16_t FILE_NAME_BITS = 150 * BITS_IN_BYTE;
  const uint16_t FILE_NAME_CONTROL_BITS = getStringControlBitsAmount(FILE_NAME_BITS / BITS_IN_BYTE);
  const uint16_t FILE_NAME_INFO = FILE_NAME_BITS + FILE_NAME_CONTROL_BITS;

  const uint16_t FILE_HEADER_SIZE = CHUNK_SIZE_INFO + FILE_SIZE_INFO + PADDING_INFO + FILE_NAME_INFO;

  void openArchiveToWrite();

  void getNumberOfFiles();

  void setNumberOfFiles();

  struct FileHeader writeFileHeader(const std::string& file_path, uint16_t chunk_size);

  struct FileHeader readFileHeader(bitReader& read_stream) const;

  void nextFile(struct FileHeader& file_header, bitReader& read_stream);

  void extractFile(bitReader& read_stream, struct FileHeader file_header, std::string& path);

  bool archiveExists();

  static void invalidArchive();

  static void invalidFile(const std::string&);

  static bits readChunk(bitReader& read_stream, uint32_t encoded_chunk_size, uint8_t control_bits_amount);

  static std::string readString(bitReader& read_stream, uint32_t string_size);

  static uint8_t getPaddingBitsAmount(uint64_t file_size, uint16_t chunk_size);

  static uint32_t getStringControlBitsAmount(uint32_t);

  static void printBits(bits& bts, std::string message);

  static uint64_t getEncodedFileLength(const FileHeader& file_header);
};

struct FileHeader {
  uint16_t chunk_size;
  uint64_t file_size;
  uint8_t padding;
  std::string file_name;
};