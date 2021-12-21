//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isalnum (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::isalnum(' ', l));
    assert(!std::isalnum('<', l));
    assert(!std::isalnum('\x8', l));
    assert( std::isalnum('A', l));
    assert( std::isalnum('a', l));
    assert( std::isalnum('z', l));
    assert( std::isalnum('3', l));
    assert(!std::isalnum('.', l));
    assert( std::isalnum('f', l));
    assert( std::isalnum('9', l));
    assert(!std::isalnum('+', l));
}
