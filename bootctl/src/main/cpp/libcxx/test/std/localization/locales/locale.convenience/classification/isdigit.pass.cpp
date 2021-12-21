//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isdigit (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::isdigit(' ', l));
    assert(!std::isdigit('<', l));
    assert(!std::isdigit('\x8', l));
    assert(!std::isdigit('A', l));
    assert(!std::isdigit('a', l));
    assert(!std::isdigit('z', l));
    assert( std::isdigit('3', l));
    assert(!std::isdigit('.', l));
    assert(!std::isdigit('f', l));
    assert( std::isdigit('9', l));
    assert(!std::isdigit('+', l));
}
