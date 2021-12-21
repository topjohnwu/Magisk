//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>
// UNSUPPORTED: libcpp-no-exceptions
// UNSUPPORTED: c++98, c++03

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_match(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// Throw exception after spent too many cycles with respect to the length of the input string.

#include <regex>
#include <cassert>

int main() {
  for (std::regex_constants::syntax_option_type op :
       {std::regex::ECMAScript, std::regex::extended, std::regex::egrep,
        std::regex::awk}) {
    try {
      std::regex_match(
          "aaaaaaaaaaaaaaaaaaaa",
          std::regex(
              "a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?aaaaaaaaaaaaaaaaaaaa",
              op));
      assert(false);
    } catch (const std::regex_error &e) {
      assert(e.code() == std::regex_constants::error_complexity);
    }
  }
  std::string s(100000, 'a');
  for (std::regex_constants::syntax_option_type op :
       {std::regex::ECMAScript, std::regex::extended, std::regex::egrep,
        std::regex::awk}) {
    assert(std::regex_match(s, std::regex("a*", op)));
  }
  return 0;
}
