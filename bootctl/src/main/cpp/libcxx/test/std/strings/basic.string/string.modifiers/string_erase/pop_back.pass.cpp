//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// void pop_back();

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, S expected)
{
    s.pop_back();
    LIBCPP_ASSERT(s.__invariants());
    assert(s[s.size()] == typename S::value_type());
    assert(s == expected);
}

int main()
{
    {
    typedef std::string S;
    test(S("abcde"), S("abcd"));
    test(S("abcdefghij"), S("abcdefghi"));
    test(S("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrs"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S("abcde"), S("abcd"));
    test(S("abcdefghij"), S("abcdefghi"));
    test(S("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrs"));
    }
#endif
}
