//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isalpha (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::isalpha(' ', l));
    assert(!std::isalpha('<', l));
    assert(!std::isalpha('\x8', l));
    assert( std::isalpha('A', l));
    assert( std::isalpha('a', l));
    assert( std::isalpha('z', l));
    assert(!std::isalpha('3', l));
    assert(!std::isalpha('.', l));
    assert( std::isalpha('f', l));
    assert(!std::isalpha('9', l));
    assert(!std::isalpha('+', l));
}
