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
// from_chars_result from_chars(const char* first, const char* last,
//                              Integral& value, int base = 10)
//
// Integral cannot be bool.

#include <charconv>

int main()
{
    using std::from_chars;
    char buf[] = "01001";
    bool lv;

    from_chars(buf, buf + sizeof(buf), lv);      // expected-error {{call to deleted function}}
    from_chars(buf, buf + sizeof(buf), lv, 16);  // expected-error {{call to deleted function}}
}
