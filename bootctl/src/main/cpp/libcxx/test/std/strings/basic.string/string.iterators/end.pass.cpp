//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

//       iterator end();
// const_iterator end() const;

#include <string>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"

template <class S>
void
test(S s)
{
    const S& cs = s;
    typename S::iterator e = s.end();
    typename S::const_iterator ce = cs.end();
    if (s.empty())
    {
        assert(e == s.begin());
        assert(ce == cs.begin());
    }
    assert(static_cast<std::size_t>(e - s.begin()) == s.size());
    assert(static_cast<std::size_t>(ce - cs.begin()) == cs.size());
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
