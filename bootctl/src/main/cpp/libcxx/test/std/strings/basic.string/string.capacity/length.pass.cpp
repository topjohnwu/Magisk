//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type length() const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s)
{
    assert(s.length() == s.size());
}

int main()
{
    {
    typedef std::string S;
    test(S());
    test(S("123"));
    test(S("12345678901234567890123456789012345678901234567890"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S());
    test(S("123"));
    test(S("12345678901234567890123456789012345678901234567890"));
    }
#endif
}
