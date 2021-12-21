//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isspace (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert( std::isspace(' ', l));
    assert(!std::isspace('<', l));
    assert(!std::isspace('\x8', l));
    assert(!std::isspace('A', l));
    assert(!std::isspace('a', l));
    assert(!std::isspace('z', l));
    assert(!std::isspace('3', l));
    assert(!std::isspace('.', l));
    assert(!std::isspace('f', l));
    assert(!std::isspace('9', l));
    assert(!std::isspace('+', l));
}
