//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

//       iterator begin();
// const_iterator begin() const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(S s)
{
    const S& cs = s;
    typename S::iterator b = s.begin();
    typename S::const_iterator cb = cs.begin();
    if (!s.empty())
    {
        assert(*b == s[0]);
    }
    assert(b == cb);
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
