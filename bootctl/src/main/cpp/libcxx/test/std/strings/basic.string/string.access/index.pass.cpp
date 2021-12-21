//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// const_reference operator[](size_type pos) const;
//       reference operator[](size_type pos);

#ifdef _LIBCPP_DEBUG
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <string>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    typedef std::string S;
    S s("0123456789");
    const S& cs = s;
    for (S::size_type i = 0; i < cs.size(); ++i)
    {
        assert(s[i] == static_cast<char>('0' + i));
        assert(cs[i] == s[i]);
    }
    assert(cs[cs.size()] == '\0');
    const S s2 = S();
    assert(s2[0] == '\0');
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    S s("0123456789");
    const S& cs = s;
    for (S::size_type i = 0; i < cs.size(); ++i)
    {
        assert(s[i] == static_cast<char>('0' + i));
        assert(cs[i] == s[i]);
    }
    assert(cs[cs.size()] == '\0');
    const S s2 = S();
    assert(s2[0] == '\0');
    }
#endif
#ifdef _LIBCPP_DEBUG
    {
        std::string s;
        char c = s[0];
        assert(c == '\0');
        c = s[1];
        assert(false);
    }
#endif
}
