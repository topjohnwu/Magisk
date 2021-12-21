//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// iterator insert(const_iterator p, size_type n, charT c);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::difference_type pos, typename S::size_type n,
     typename S::value_type c, S expected)
{
    typename S::const_iterator p = s.cbegin() + pos;
    typename S::iterator i = s.insert(p, n, c);
    LIBCPP_ASSERT(s.__invariants());
    assert(i - s.begin() == pos);
    assert(s == expected);
}

int main()
{
    {
    typedef std::string S;
    test(S(""), 0, 0, '1', S(""));
    test(S(""), 0, 5, '1', S("11111"));
    test(S(""), 0, 10, '1', S("1111111111"));
    test(S(""), 0, 20, '1', S("11111111111111111111"));
    test(S("abcde"), 0, 0, '1', S("abcde"));
    test(S("abcde"), 0, 5, '1', S("11111abcde"));
    test(S("abcde"), 0, 10, '1', S("1111111111abcde"));
    test(S("abcde"), 0, 20, '1', S("11111111111111111111abcde"));
    test(S("abcde"), 1, 0, '1', S("abcde"));
    test(S("abcde"), 1, 5, '1', S("a11111bcde"));
    test(S("abcde"), 1, 10, '1', S("a1111111111bcde"));
    test(S("abcde"), 1, 20, '1', S("a11111111111111111111bcde"));
    test(S("abcde"), 2, 0, '1', S("abcde"));
    test(S("abcde"), 2, 5, '1', S("ab11111cde"));
    test(S("abcde"), 2, 10, '1', S("ab1111111111cde"));
    test(S("abcde"), 2, 20, '1', S("ab11111111111111111111cde"));
    test(S("abcde"), 4, 0, '1', S("abcde"));
    test(S("abcde"), 4, 5, '1', S("abcd11111e"));
    test(S("abcde"), 4, 10, '1', S("abcd1111111111e"));
    test(S("abcde"), 4, 20, '1', S("abcd11111111111111111111e"));
    test(S("abcde"), 5, 0, '1', S("abcde"));
    test(S("abcde"), 5, 5, '1', S("abcde11111"));
    test(S("abcde"), 5, 10, '1', S("abcde1111111111"));
    test(S("abcde"), 5, 20, '1', S("abcde11111111111111111111"));
    test(S("abcdefghij"), 0, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 0, 5, '1', S("11111abcdefghij"));
    test(S("abcdefghij"), 0, 10, '1', S("1111111111abcdefghij"));
    test(S("abcdefghij"), 0, 20, '1', S("11111111111111111111abcdefghij"));
    test(S("abcdefghij"), 1, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 1, 5, '1', S("a11111bcdefghij"));
    test(S("abcdefghij"), 1, 10, '1', S("a1111111111bcdefghij"));
    test(S("abcdefghij"), 1, 20, '1', S("a11111111111111111111bcdefghij"));
    test(S("abcdefghij"), 5, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 5, 5, '1', S("abcde11111fghij"));
    test(S("abcdefghij"), 5, 10, '1', S("abcde1111111111fghij"));
    test(S("abcdefghij"), 5, 20, '1', S("abcde11111111111111111111fghij"));
    test(S("abcdefghij"), 9, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 9, 5, '1', S("abcdefghi11111j"));
    test(S("abcdefghij"), 9, 10, '1', S("abcdefghi1111111111j"));
    test(S("abcdefghij"), 9, 20, '1', S("abcdefghi11111111111111111111j"));
    test(S("abcdefghij"), 10, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 10, 5, '1', S("abcdefghij11111"));
    test(S("abcdefghij"), 10, 10, '1', S("abcdefghij1111111111"));
    test(S("abcdefghij"), 10, 20, '1', S("abcdefghij11111111111111111111"));
    test(S("abcdefghijklmnopqrst"), 0, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 5, '1', S("11111abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, '1', S("1111111111abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 20, '1', S("11111111111111111111abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 5, '1', S("a11111bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 10, '1', S("a1111111111bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 20, '1', S("a11111111111111111111bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, '1', S("abcdefghij11111klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 10, '1', S("abcdefghij1111111111klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 20, '1', S("abcdefghij11111111111111111111klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 5, '1', S("abcdefghijklmnopqrs11111t"));
    test(S("abcdefghijklmnopqrst"), 19, 10, '1', S("abcdefghijklmnopqrs1111111111t"));
    test(S("abcdefghijklmnopqrst"), 19, 20, '1', S("abcdefghijklmnopqrs11111111111111111111t"));
    test(S("abcdefghijklmnopqrst"), 20, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 5, '1', S("abcdefghijklmnopqrst11111"));
    test(S("abcdefghijklmnopqrst"), 20, 10, '1', S("abcdefghijklmnopqrst1111111111"));
    test(S("abcdefghijklmnopqrst"), 20, 20, '1', S("abcdefghijklmnopqrst11111111111111111111"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), 0, 0, '1', S(""));
    test(S(""), 0, 5, '1', S("11111"));
    test(S(""), 0, 10, '1', S("1111111111"));
    test(S(""), 0, 20, '1', S("11111111111111111111"));
    test(S("abcde"), 0, 0, '1', S("abcde"));
    test(S("abcde"), 0, 5, '1', S("11111abcde"));
    test(S("abcde"), 0, 10, '1', S("1111111111abcde"));
    test(S("abcde"), 0, 20, '1', S("11111111111111111111abcde"));
    test(S("abcde"), 1, 0, '1', S("abcde"));
    test(S("abcde"), 1, 5, '1', S("a11111bcde"));
    test(S("abcde"), 1, 10, '1', S("a1111111111bcde"));
    test(S("abcde"), 1, 20, '1', S("a11111111111111111111bcde"));
    test(S("abcde"), 2, 0, '1', S("abcde"));
    test(S("abcde"), 2, 5, '1', S("ab11111cde"));
    test(S("abcde"), 2, 10, '1', S("ab1111111111cde"));
    test(S("abcde"), 2, 20, '1', S("ab11111111111111111111cde"));
    test(S("abcde"), 4, 0, '1', S("abcde"));
    test(S("abcde"), 4, 5, '1', S("abcd11111e"));
    test(S("abcde"), 4, 10, '1', S("abcd1111111111e"));
    test(S("abcde"), 4, 20, '1', S("abcd11111111111111111111e"));
    test(S("abcde"), 5, 0, '1', S("abcde"));
    test(S("abcde"), 5, 5, '1', S("abcde11111"));
    test(S("abcde"), 5, 10, '1', S("abcde1111111111"));
    test(S("abcde"), 5, 20, '1', S("abcde11111111111111111111"));
    test(S("abcdefghij"), 0, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 0, 5, '1', S("11111abcdefghij"));
    test(S("abcdefghij"), 0, 10, '1', S("1111111111abcdefghij"));
    test(S("abcdefghij"), 0, 20, '1', S("11111111111111111111abcdefghij"));
    test(S("abcdefghij"), 1, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 1, 5, '1', S("a11111bcdefghij"));
    test(S("abcdefghij"), 1, 10, '1', S("a1111111111bcdefghij"));
    test(S("abcdefghij"), 1, 20, '1', S("a11111111111111111111bcdefghij"));
    test(S("abcdefghij"), 5, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 5, 5, '1', S("abcde11111fghij"));
    test(S("abcdefghij"), 5, 10, '1', S("abcde1111111111fghij"));
    test(S("abcdefghij"), 5, 20, '1', S("abcde11111111111111111111fghij"));
    test(S("abcdefghij"), 9, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 9, 5, '1', S("abcdefghi11111j"));
    test(S("abcdefghij"), 9, 10, '1', S("abcdefghi1111111111j"));
    test(S("abcdefghij"), 9, 20, '1', S("abcdefghi11111111111111111111j"));
    test(S("abcdefghij"), 10, 0, '1', S("abcdefghij"));
    test(S("abcdefghij"), 10, 5, '1', S("abcdefghij11111"));
    test(S("abcdefghij"), 10, 10, '1', S("abcdefghij1111111111"));
    test(S("abcdefghij"), 10, 20, '1', S("abcdefghij11111111111111111111"));
    test(S("abcdefghijklmnopqrst"), 0, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 5, '1', S("11111abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, '1', S("1111111111abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 20, '1', S("11111111111111111111abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 5, '1', S("a11111bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 10, '1', S("a1111111111bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 20, '1', S("a11111111111111111111bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, '1', S("abcdefghij11111klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 10, '1', S("abcdefghij1111111111klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 20, '1', S("abcdefghij11111111111111111111klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 5, '1', S("abcdefghijklmnopqrs11111t"));
    test(S("abcdefghijklmnopqrst"), 19, 10, '1', S("abcdefghijklmnopqrs1111111111t"));
    test(S("abcdefghijklmnopqrst"), 19, 20, '1', S("abcdefghijklmnopqrs11111111111111111111t"));
    test(S("abcdefghijklmnopqrst"), 20, 0, '1', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 5, '1', S("abcdefghijklmnopqrst11111"));
    test(S("abcdefghijklmnopqrst"), 20, 10, '1', S("abcdefghijklmnopqrst1111111111"));
    test(S("abcdefghijklmnopqrst"), 20, 20, '1', S("abcdefghijklmnopqrst11111111111111111111"));
    }
#endif
}
