//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// const charT* c_str() const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s)
{
    typedef typename S::traits_type T;
    const typename S::value_type* str = s.c_str();
    if (s.size() > 0)
    {
        assert(T::compare(str, &s[0], s.size()) == 0);
        assert(T::eq(str[s.size()], typename S::value_type()));
    }
    else
        assert(T::eq(str[0], typename S::value_type()));
}

int main()
{
    {
    typedef std::string S;
    test(S(""));
    test(S("abcde"));
    test(S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""));
    test(S("abcde"));
    test(S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"));
    }
#endif
}
