//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string<charT,traits,Allocator>& operator=(basic_string_view<charT, traits> sv);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S, class SV>
void
test(S s1, SV sv)
{
    typedef typename S::traits_type T;
    s1 = sv;
    LIBCPP_ASSERT(s1.__invariants());
    assert(s1.size() == sv.size());
    assert(T::compare(s1.data(), sv.data(), s1.size()) == 0);
    assert(s1.capacity() >= s1.size());
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test(S(), SV(""));
    test(S("1"), SV(""));
    test(S(), SV("1"));
    test(S("1"), SV("2"));
    test(S("1"), SV("2"));

    test(S(),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    test(S("123456789"),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"
           "1234567890123456789012345678901234567890123456789012345678901234567890"),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::string_view SV;
    test(S(), SV(""));
    test(S("1"), SV(""));
    test(S(), SV("1"));
    test(S("1"), SV("2"));
    test(S("1"), SV("2"));

    test(S(),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    test(S("123456789"),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"
           "1234567890123456789012345678901234567890123456789012345678901234567890"),
         SV("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
    }
#endif
}
