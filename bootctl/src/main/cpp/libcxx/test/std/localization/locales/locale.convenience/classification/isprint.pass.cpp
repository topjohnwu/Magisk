//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isprint (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert( std::isprint(' ', l));
    assert( std::isprint('<', l));
    assert(!std::isprint('\x8', l));
    assert( std::isprint('A', l));
    assert( std::isprint('a', l));
    assert( std::isprint('z', l));
    assert( std::isprint('3', l));
    assert( std::isprint('.', l));
    assert( std::isprint('f', l));
    assert( std::isprint('9', l));
    assert( std::isprint('+', l));
}
