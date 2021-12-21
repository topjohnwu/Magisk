#include <regex.h>
#include <string>
int main() {
  std::string str = "test0159";
  regex_t re;
  int ec = regcomp(&re, "^[a-z]+[0-9]+$", REG_EXTENDED | REG_NOSUB);
  if (ec != 0) {
    return ec;
  }
  int ret = regexec(&re, str.c_str(), 0, nullptr, 0) ? -1 : 0;
  regfree(&re);
  return ret;
}

