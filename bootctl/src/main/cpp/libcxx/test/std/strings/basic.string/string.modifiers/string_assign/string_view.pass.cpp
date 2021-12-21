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
//   assign(basic_string_view<charT,traits> sv);

#include <string>
#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "test_allocator.h"

template <class S, class SV>
void
test(S s, SV sv, S expected)
{
    s.assign(sv);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

template <class S, class SV>
void
testAlloc(S s, SV sv, const typename S::allocator_type& a)
{
    s.assign(sv);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == sv);
    assert(s.get_allocator() == a);
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test(S(), SV(), S());
    test(S(), SV("12345"), S("12345"));
    test(S(), SV("1234567890"), S("1234567890"));
    test(S(), SV("12345678901234567890"), S("12345678901234567890"));

    test(S("12345"), SV(), S());
    test(S("12345"), SV("12345"), S("12345"));
    test(S("12345"), SV("1234567890"), S("1234567890"));
    test(S("12345"), SV("12345678901234567890"), S("12345678901234567890"));

    test(S("1234567890"), SV(), S());
    test(S("1234567890"), SV("12345"), S("12345"));
    test(S("1234567890"), SV("1234567890"), S("1234567890"));
    test(S("1234567890"), SV("12345678901234567890"), S("12345678901234567890"));

    test(S("12345678901234567890"), SV(), S());
    test(S("12345678901234567890"), SV("12345"), S("12345"));
    test(S("12345678901234567890"), SV("1234567890"), S("1234567890"));
    test(S("12345678901234567890"), SV("12345678901234567890"),
         S("12345678901234567890"));

    testAlloc(S(), SV(), std::allocator<char>());
    testAlloc(S(), SV("12345"), std::allocator<char>());
    testAlloc(S(), SV("1234567890"), std::allocator<char>());
    testAlloc(S(), SV("12345678901234567890"), std::allocator<char>());
    }

#if TEST_STD_VER >= 11
    {
    typedef std::basic_string     <char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::basic_string_view<char, std::char_traits<char> > SV;
    test(S(), SV(), S());
    test(S(), SV("12345"), S("12345"));
    test(S(), SV("1234567890"), S("1234567890"));
    test(S(), SV("12345678901234567890"), S("12345678901234567890"));

    test(S("12345"), SV(), S());
    test(S("12345"), SV("12345"), S("12345"));
    test(S("12345"), SV("1234567890"), S("1234567890"));
    test(S("12345"), SV("12345678901234567890"), S("12345678901234567890"));

    test(S("1234567890"), SV(), S());
    test(S("1234567890"), SV("12345"), S("12345"));
    test(S("1234567890"), SV("1234567890"), S("1234567890"));
    test(S("1234567890"), SV("12345678901234567890"), S("12345678901234567890"));

    test(S("12345678901234567890"), SV(), S());
    test(S("12345678901234567890"), SV("12345"), S("12345"));
    test(S("12345678901234567890"), SV("1234567890"), S("1234567890"));
    test(S("12345678901234567890"), SV("12345678901234567890"),
         S("12345678901234567890"));

    testAlloc(S(), SV(), min_allocator<char>());
    testAlloc(S(), SV("12345"), min_allocator<char>());
    testAlloc(S(), SV("1234567890"), min_allocator<char>());
    testAlloc(S(), SV("12345678901234567890"), min_allocator<char>());
    }
#endif
}
