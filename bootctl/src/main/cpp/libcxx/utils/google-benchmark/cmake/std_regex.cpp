#include <regex>
#include <string>
int main() {
  const std::string str = "test0159";
  std::regex re;
  re = std::regex("^[a-z]+[0-9]+$",
       std::regex_constants::extended | std::regex_constants::nosubs);
  return std::regex_search(str, re) ? 0 : -1;
}

