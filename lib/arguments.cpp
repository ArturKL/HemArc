#include "arguments.h"
#include "error_codes.h"
#include <algorithm>
#include <iostream>

ArgumentParser::ArgumentParser(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        arguments.push_back(std::string(argv[i]));
    }
    parsed = std::vector<bool>(argc);
}

size_t ArgumentParser::parseParameterizedArgument(char* short_flag, char* long_flag, uint16_t parameter_amount, bool is_required) {
    size_t flag_index = findFlag(short_flag, long_flag);
    if (!isValid(flag_index, true, parameter_amount)) {
        if (is_required) {
            invalidArguemntError();
        }
        return NULL;
    }

    for (int i = 0; i < parameter_amount + 1; i++) {
        parsed[flag_index + i] = 1;
    }

    return flag_index + 1;
}

std::string ArgumentParser::parseArgumentByRegex(char* short_flag, std::regex expression, bool is_required) {
    auto arguemnt_iterator =
        std::find(arguments.begin(), arguments.end(), static_cast<std::string>(short_flag));
    size_t flag_index = arguemnt_iterator - arguments.begin();
    if (isValid(flag_index, true, 1)) {
        parsed[flag_index] = 1;
        parsed[flag_index + 1] = 1;
        return arguments[flag_index + 1];
    } else if (isValid(flag_index, false)) { // If found short flag but not arguemnt value
        invalidArguemntError();
    }

    // Find by regex
    auto predicate = [expression](std::string s) { // Lambda function that checks if string matches regex
      return std::regex_match(s, expression);
    };
    arguemnt_iterator = std::find_if(arguments.begin(), arguments.end(), predicate);

    flag_index = arguemnt_iterator - arguments.begin();
    if (!isValid(flag_index, false, 0)) {
        if (is_required) {
            invalidArguemntError();
        }
        return NULL;
    }

    parsed[flag_index] = 1;

    return arguments[flag_index];
}

bool ArgumentParser::parseBoolArgument(char* short_flag, char* long_flag) {
    size_t flag_index = findFlag(short_flag, long_flag);
    if (!isValid(flag_index, false)) {
        return false;
    }
    parsed[flag_index] = 1;
    
    return true;
}

std::vector<size_t> ArgumentParser::getRest() {
    std::vector<size_t> rest;
    for (size_t i = 1; i < arguments.size(); i++) {
        if (!parsed[i]) {
            rest.push_back(i);
        }
    }
    return rest;
}

void ArgumentParser::invalidArguemntError() {
    std::cerr << "Invalid command line arguments\n";
    exit(ERROR_INVALID_PARARMETER);
}

size_t ArgumentParser::findFlag(char* short_flag, char* long_flag) {
    auto arguemnt_iterator =
        std::find(arguments.begin(), arguments.end(), static_cast<std::string>(short_flag));
    if (arguemnt_iterator == arguments.end()) {
        arguemnt_iterator =
            std::find(arguments.begin(), arguments.end(), static_cast<std::string>(long_flag));
    }

    return arguemnt_iterator - arguments.begin();
}

bool ArgumentParser::isValid(size_t flag_index, bool is_parameterized, uint16_t parameter_amount) {
    if (is_parameterized) {
        return flag_index < arguments.size() - parameter_amount; // If flag and its parameters in arguments
    }
    return flag_index != arguments.size();
}
