//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> charT tolower(charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(std::tolower(' ', l) == ' ');
    assert(std::tolower('<', l) == '<');
    assert(std::tolower('\x8', l) == '\x8');
    assert(std::tolower('A', l) == 'a');
    assert(std::tolower('a', l) == 'a');
    assert(std::tolower('z', l) == 'z');
    assert(std::tolower('3', l) == '3');
    assert(std::tolower('.', l) == '.');
    assert(std::tolower('f', l) == 'f');
    assert(std::tolower('9', l) == '9');
    assert(std::tolower('+', l) == '+');
}
