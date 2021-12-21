//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isupper (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::isupper(' ', l));
    assert(!std::isupper('<', l));
    assert(!std::isupper('\x8', l));
    assert( std::isupper('A', l));
    assert(!std::isupper('a', l));
    assert(!std::isupper('z', l));
    assert(!std::isupper('3', l));
    assert(!std::isupper('.', l));
    assert(!std::isupper('f', l));
    assert(!std::isupper('9', l));
    assert(!std::isupper('+', l));
}
