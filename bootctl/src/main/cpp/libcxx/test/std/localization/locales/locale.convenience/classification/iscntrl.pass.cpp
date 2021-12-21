//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool iscntrl (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::iscntrl(' ', l));
    assert(!std::iscntrl('<', l));
    assert( std::iscntrl('\x8', l));
    assert(!std::iscntrl('A', l));
    assert(!std::iscntrl('a', l));
    assert(!std::iscntrl('z', l));
    assert(!std::iscntrl('3', l));
    assert(!std::iscntrl('.', l));
    assert(!std::iscntrl('f', l));
    assert(!std::iscntrl('9', l));
    assert(!std::iscntrl('+', l));
}
