#include <algorithm>
#include <regex>
#include <string>
#include <vector>

class ArgumentParser {
 public:
  ArgumentParser(int, char**);

  size_t parseParameterizedArgument(char* short_flag, char* long_flag, uint16_t parameter_amount, bool is_required);

  std::string parseArgumentByRegex(char* short_flag, std::regex expression, bool is_required);

  bool parseBoolArgument(char*, char*);

  std::vector<size_t> getRest();

 private:
  std::vector<std::string> arguments;

  std::vector<bool> parsed;

  bool isValid(size_t, bool, uint16_t = 0);

  void invalidArguemntError();

  size_t findFlag(char*, char*);
};
