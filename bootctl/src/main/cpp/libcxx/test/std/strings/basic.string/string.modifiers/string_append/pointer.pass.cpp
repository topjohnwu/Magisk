//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string<charT,traits,Allocator>& append(const charT* s);

#include <string>
#include <stdexcept>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, const typename S::value_type* str, S expected)
{
    s.append(str);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

int main()
{
    {
    typedef std::string S;
    test(S(), "", S());
    test(S(), "12345", S("12345"));
    test(S(), "12345678901234567890", S("12345678901234567890"));

    test(S("12345"), "", S("12345"));
    test(S("12345"), "12345", S("1234512345"));
    test(S("12345"), "1234567890", S("123451234567890"));

    test(S("12345678901234567890"), "", S("12345678901234567890"));
    test(S("12345678901234567890"), "12345", S("1234567890123456789012345"));
    test(S("12345678901234567890"), "12345678901234567890",
         S("1234567890123456789012345678901234567890"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), "", S());
    test(S(), "12345", S("12345"));
    test(S(), "12345678901234567890", S("12345678901234567890"));

    test(S("12345"), "", S("12345"));
    test(S("12345"), "12345", S("1234512345"));
    test(S("12345"), "1234567890", S("123451234567890"));

    test(S("12345678901234567890"), "", S("12345678901234567890"));
    test(S("12345678901234567890"), "12345", S("1234567890123456789012345"));
    test(S("12345678901234567890"), "12345678901234567890",
         S("1234567890123456789012345678901234567890"));
    }
#endif

    { // test appending to self
    typedef std::string S;
    S s_short = "123/";
    S s_long  = "Lorem ipsum dolor sit amet, consectetur/";

    s_short.append(s_short.c_str());
    assert(s_short == "123/123/");
    s_short.append(s_short.c_str());
    assert(s_short == "123/123/123/123/");
    s_short.append(s_short.c_str());
    assert(s_short == "123/123/123/123/123/123/123/123/");

    s_long.append(s_long.c_str());
    assert(s_long == "Lorem ipsum dolor sit amet, consectetur/Lorem ipsum dolor sit amet, consectetur/");
    }
}
