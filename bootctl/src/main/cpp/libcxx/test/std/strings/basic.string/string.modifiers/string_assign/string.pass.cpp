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
//   assign(const basic_string<charT,traits>& str);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "test_allocator.h"

template <class S>
void
test(S s, S str, S expected)
{
    s.assign(str);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

template <class S>
void
testAlloc(S s, S str, const typename S::allocator_type& a)
{
    s.assign(str);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == str);
    assert(s.get_allocator() == a);
}

int main()
{
    {
    typedef std::string S;
    test(S(), S(), S());
    test(S(), S("12345"), S("12345"));
    test(S(), S("1234567890"), S("1234567890"));
    test(S(), S("12345678901234567890"), S("12345678901234567890"));

    test(S("12345"), S(), S());
    test(S("12345"), S("12345"), S("12345"));
    test(S("12345"), S("1234567890"), S("1234567890"));
    test(S("12345"), S("12345678901234567890"), S("12345678901234567890"));

    test(S("1234567890"), S(), S());
    test(S("1234567890"), S("12345"), S("12345"));
    test(S("1234567890"), S("1234567890"), S("1234567890"));
    test(S("1234567890"), S("12345678901234567890"), S("12345678901234567890"));

    test(S("12345678901234567890"), S(), S());
    test(S("12345678901234567890"), S("12345"), S("12345"));
    test(S("12345678901234567890"), S("1234567890"), S("1234567890"));
    test(S("12345678901234567890"), S("12345678901234567890"),
         S("12345678901234567890"));

    testAlloc(S(), S(), std::allocator<char>());
    testAlloc(S(), S("12345"), std::allocator<char>());
    testAlloc(S(), S("1234567890"), std::allocator<char>());
    testAlloc(S(), S("12345678901234567890"), std::allocator<char>());
    }

    { //  LWG#5579 make sure assign takes the allocators where appropriate
    typedef other_allocator<char> A;  // has POCCA --> true
    typedef std::basic_string<char, std::char_traits<char>, A> S;
    testAlloc(S(A(5)), S(A(3)), A(3));
    testAlloc(S(A(5)), S("1"), A());
    testAlloc(S(A(5)), S("1", A(7)), A(7));
    testAlloc(S(A(5)), S("1234567890123456789012345678901234567890123456789012345678901234567890", A(7)), A(7));
    }

#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), S(), S());
    test(S(), S("12345"), S("12345"));
    test(S(), S("1234567890"), S("1234567890"));
    test(S(), S("12345678901234567890"), S("12345678901234567890"));

    test(S("12345"), S(), S());
    test(S("12345"), S("12345"), S("12345"));
    test(S("12345"), S("1234567890"), S("1234567890"));
    test(S("12345"), S("12345678901234567890"), S("12345678901234567890"));

    test(S("1234567890"), S(), S());
    test(S("1234567890"), S("12345"), S("12345"));
    test(S("1234567890"), S("1234567890"), S("1234567890"));
    test(S("1234567890"), S("12345678901234567890"), S("12345678901234567890"));

    test(S("12345678901234567890"), S(), S());
    test(S("12345678901234567890"), S("12345"), S("12345"));
    test(S("12345678901234567890"), S("1234567890"), S("1234567890"));
    test(S("12345678901234567890"), S("12345678901234567890"),
         S("12345678901234567890"));

    testAlloc(S(), S(), min_allocator<char>());
    testAlloc(S(), S("12345"), min_allocator<char>());
    testAlloc(S(), S("1234567890"), min_allocator<char>());
    testAlloc(S(), S("12345678901234567890"), min_allocator<char>());
    }
#endif
#if TEST_STD_VER > 14
    {
    typedef std::string S;
    static_assert(noexcept(S().assign(S())), "");  // LWG#2063
    }
#endif
}
