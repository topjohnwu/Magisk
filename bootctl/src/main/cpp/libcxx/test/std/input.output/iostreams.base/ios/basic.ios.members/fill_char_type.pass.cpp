//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// template <class charT, class traits> class basic_ios

// char_type fill(char_type fillch);

#include <ios>
#include <cassert>

int main()
{
    std::ios ios(0);
    assert(ios.fill() == ' ');
    char c = ios.fill('*');
    assert(c == ' ');
    assert(ios.fill() == '*');
}
