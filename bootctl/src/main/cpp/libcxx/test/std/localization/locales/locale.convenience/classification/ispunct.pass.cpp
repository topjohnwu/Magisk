//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool ispunct (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::ispunct(' ', l));
    assert( std::ispunct('<', l));
    assert(!std::ispunct('\x8', l));
    assert(!std::ispunct('A', l));
    assert(!std::ispunct('a', l));
    assert(!std::ispunct('z', l));
    assert(!std::ispunct('3', l));
    assert( std::ispunct('.', l));
    assert(!std::ispunct('f', l));
    assert(!std::ispunct('9', l));
    assert( std::ispunct('+', l));
}
