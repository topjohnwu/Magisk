//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>
//   The strings's value type must be the same as the traits's char_type

#include <string>

int main()
{
    std::basic_string<char, std::char_traits<wchar_t>> s;
}
