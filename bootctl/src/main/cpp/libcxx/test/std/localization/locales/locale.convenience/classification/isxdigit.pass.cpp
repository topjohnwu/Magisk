//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isxdigit (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::isxdigit(' ', l));
    assert(!std::isxdigit('<', l));
    assert(!std::isxdigit('\x8', l));
    assert( std::isxdigit('A', l));
    assert( std::isxdigit('a', l));
    assert(!std::isxdigit('z', l));
    assert( std::isxdigit('3', l));
    assert(!std::isxdigit('.', l));
    assert( std::isxdigit('f', l));
    assert( std::isxdigit('9', l));
    assert(!std::isxdigit('+', l));
}
