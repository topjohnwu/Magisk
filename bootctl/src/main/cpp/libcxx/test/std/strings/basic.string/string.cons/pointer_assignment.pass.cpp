//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string<charT,traits,Allocator>&
//   operator=(const charT* s);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s1, const typename S::value_type* s2)
{
    typedef typename S::traits_type T;
    s1 = s2;
    LIBCPP_ASSERT(s1.__invariants());
    assert(s1.size() == T::length(s2));
    assert(T::compare(s1.data(), s2, s1.size()) == 0);
    assert(s1.capacity() >= s1.size());
}

int main()
{
    {
    typedef std::string S;
    test(S(), "");
    test(S("1"), "");
    test(S(), "1");
    test(S("1"), "2");
    test(S("1"), "2");

    test(S(),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    test(S("123456789"),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"
           "1234567890123456789012345678901234567890123456789012345678901234567890"),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), "");
    test(S("1"), "");
    test(S(), "1");
    test(S("1"), "2");
    test(S("1"), "2");

    test(S(),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    test(S("123456789"),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"
           "1234567890123456789012345678901234567890123456789012345678901234567890"),
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    }
#endif
}
