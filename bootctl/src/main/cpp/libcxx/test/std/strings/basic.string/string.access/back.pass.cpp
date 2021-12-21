//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// const charT& back() const;
//       charT& back();

#ifdef _LIBCPP_DEBUG
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(S s)
{
    const S& cs = s;
    assert(&cs.back() == &cs[cs.size()-1]);
    assert(&s.back() == &s[cs.size()-1]);
    s.back() = typename S::value_type('z');
    assert(s.back() == typename S::value_type('z'));
}

int main()
{
    {
    typedef std::string S;
    test(S("1"));
    test(S("1234567890123456789012345678901234567890"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S("1"));
    test(S("1234567890123456789012345678901234567890"));
    }
#endif
#ifdef _LIBCPP_DEBUG
    {
        std::string s;
        char c = s.back();
        assert(false);
    }
#endif
}
