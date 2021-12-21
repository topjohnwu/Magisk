//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type size() const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s, typename S::size_type c)
{
    assert(s.size() == c);
}

int main()
{
    {
    typedef std::string S;
    test(S(), 0);
    test(S("123"), 3);
    test(S("12345678901234567890123456789012345678901234567890"), 50);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), 0);
    test(S("123"), 3);
    test(S("12345678901234567890123456789012345678901234567890"), 50);
    }
#endif
}
