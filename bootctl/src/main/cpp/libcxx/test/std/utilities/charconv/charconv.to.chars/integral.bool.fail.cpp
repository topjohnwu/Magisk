//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <charconv>

// In
//
// to_chars_result to_chars(char* first, char* last, Integral value,
//                          int base = 10)
//
// Integral cannot be bool.

#include <charconv>

int main()
{
    using std::to_chars;
    char buf[10];
    bool lv = true;

    to_chars(buf, buf + sizeof(buf), false);   // expected-error {{call to deleted function}}
    to_chars(buf, buf + sizeof(buf), lv, 16);  // expected-error {{call to deleted function}}
}
