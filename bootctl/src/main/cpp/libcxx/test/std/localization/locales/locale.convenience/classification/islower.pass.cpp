//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool islower (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::islower(' ', l));
    assert(!std::islower('<', l));
    assert(!std::islower('\x8', l));
    assert(!std::islower('A', l));
    assert( std::islower('a', l));
    assert( std::islower('z', l));
    assert(!std::islower('3', l));
    assert(!std::islower('.', l));
    assert( std::islower('f', l));
    assert(!std::islower('9', l));
    assert(!std::islower('+', l));
}
