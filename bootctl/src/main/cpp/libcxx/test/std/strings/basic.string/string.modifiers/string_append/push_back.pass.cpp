//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// void push_back(charT c)

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

struct veryLarge
{
  long long a;
  char b;
};

template <class S>
void
test(S s, typename S::value_type c, S expected)
{
    s.push_back(c);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

int main()
{
    {
    typedef std::string S;
    test(S(), 'a', S(1, 'a'));
    test(S("12345"), 'a', S("12345a"));
    test(S("12345678901234567890"), 'a', S("12345678901234567890a"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), 'a', S(1, 'a'));
    test(S("12345"), 'a', S("12345a"));
    test(S("12345678901234567890"), 'a', S("12345678901234567890a"));
    }
#endif

    {
// https://bugs.llvm.org/show_bug.cgi?id=31454
    std::basic_string<veryLarge> s;
    veryLarge vl = {};
    s.push_back(vl);
    s.push_back(vl);
    s.push_back(vl);
    }
}
