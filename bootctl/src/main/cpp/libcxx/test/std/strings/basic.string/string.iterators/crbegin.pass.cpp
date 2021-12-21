//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// const_reverse_iterator crbegin() const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s)
{
    typename S::const_reverse_iterator cb = s.crbegin();
    if (!s.empty())
    {
        assert(*cb == s.back());
    }
    assert(cb == s.rbegin());
}

int main()
{
    {
    typedef std::string S;
    test(S());
    test(S("123"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S());
    test(S("123"));
    }
#endif
}
