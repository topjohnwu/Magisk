//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> bool isgraph (charT c, const locale& loc);

#include <locale>
#include <cassert>

int main()
{
    std::locale l;
    assert(!std::isgraph(' ', l));
    assert( std::isgraph('<', l));
    assert(!std::isgraph('\x8', l));
    assert( std::isgraph('A', l));
    assert( std::isgraph('a', l));
    assert( std::isgraph('z', l));
    assert( std::isgraph('3', l));
    assert( std::isgraph('.', l));
    assert( std::isgraph('f', l));
    assert( std::isgraph('9', l));
    assert( std::isgraph('+', l));
}
