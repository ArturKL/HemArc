#include <lib/archive.h>
#include <lib/arguments.h>
#include <lib/error_codes.h>
#include <iostream>
#include <cmath>

void invalidArguments() {
  std::cerr << "Invalid arguments\n";
  exit(ERROR_INVALID_PARARMETER);
}

int main(int argc, char** argv) {
    ArgumentParser ap = ArgumentParser(argc, argv);
    bool create = ap.parseBoolArgument("-c", "--create");
    bool list = ap.parseBoolArgument("-l", "--list");
    bool extract = ap.parseBoolArgument("-x", "--extract");
    bool append = ap.parseBoolArgument("-a", "--append");
    std::string archive_path = ap.parseArgumentByRegex("-f", std::regex("--file=.+"), true);
    if (archive_path[0] == '-') {
        archive_path = archive_path.substr(strlen("--file=")); // Separate archive_path path from flag
    }
    size_t extract_path_arg = ap.parseParameterizedArgument("-p", "--extract-path", 1, false);

    CorrectingArchive archive(archive_path);
    if (create) {
        archive.createEmptyArchive();
        std::vector<size_t> files = ap.getRest();
        for (auto file : files) {
            std::string file_path = argv[file];
            archive.appendFile(file_path, 8);
        }
    } else if (append) {
        std::vector<size_t> files = ap.getRest();
        for (auto file : files) {
            std::string file_path = argv[file];
            archive.appendFile(file_path, 8);
        }
    } else if (list) {
        std::vector<std::string> files = archive.listFiles();
        for (auto file : files) {
            std::cout << file << '\n';
        }
    } else if (extract) {
        std::vector<size_t> file_indices = ap.getRest();
        std::vector<std::string> files;
        for (auto i : file_indices) {
            files.push_back(argv[i]);
        }
        if (extract_path_arg != NULL) {
            std::string extract_path = argv[extract_path_arg];
            if (files.empty()) {
                archive.extractAllFiles(extract_path);
            } else {
                archive.extractFiles(files, extract_path);
            }
        } else {
            invalidArguments();
        }
    }
    return 0;
}
