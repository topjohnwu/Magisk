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
//   erase(size_type pos = 0, size_type n = npos);

#include <string>
#include <stdexcept>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::size_type pos, typename S::size_type n, S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos <= old_size)
    {
        s.erase(pos, n);
        LIBCPP_ASSERT(s.__invariants());
        assert(s[s.size()] == typename S::value_type());
        assert(s == expected);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.erase(pos, n);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > old_size);
            assert(s == s0);
        }
    }
#endif
}

template <class S>
void
test(S s, typename S::size_type pos, S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos <= old_size)
    {
        s.erase(pos);
        LIBCPP_ASSERT(s.__invariants());
        assert(s[s.size()] == typename S::value_type());
        assert(s == expected);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.erase(pos);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > old_size);
            assert(s == s0);
        }
    }
#endif
}

template <class S>
void
test(S s, S expected)
{
    s.erase();
    LIBCPP_ASSERT(s.__invariants());
    assert(s[s.size()] == typename S::value_type());
    assert(s == expected);
}

int main()
{
    {
    typedef std::string S;
    test(S(""), 0, 0, S(""));
    test(S(""), 0, 1, S(""));
    test(S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 0, 0, S("abcde"));
    test(S("abcde"), 0, 1, S("bcde"));
    test(S("abcde"), 0, 2, S("cde"));
    test(S("abcde"), 0, 4, S("e"));
    test(S("abcde"), 0, 5, S(""));
    test(S("abcde"), 0, 6, S(""));
    test(S("abcde"), 1, 0, S("abcde"));
    test(S("abcde"), 1, 1, S("acde"));
    test(S("abcde"), 1, 2, S("ade"));
    test(S("abcde"), 1, 3, S("ae"));
    test(S("abcde"), 1, 4, S("a"));
    test(S("abcde"), 1, 5, S("a"));
    test(S("abcde"), 2, 0, S("abcde"));
    test(S("abcde"), 2, 1, S("abde"));
    test(S("abcde"), 2, 2, S("abe"));
    test(S("abcde"), 2, 3, S("ab"));
    test(S("abcde"), 2, 4, S("ab"));
    test(S("abcde"), 4, 0, S("abcde"));
    test(S("abcde"), 4, 1, S("abcd"));
    test(S("abcde"), 4, 2, S("abcd"));
    test(S("abcde"), 5, 0, S("abcde"));
    test(S("abcde"), 5, 1, S("abcde"));
    test(S("abcde"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, 1, S("bcdefghij"));
    test(S("abcdefghij"), 0, 5, S("fghij"));
    test(S("abcdefghij"), 0, 9, S("j"));
    test(S("abcdefghij"), 0, 10, S(""));
    test(S("abcdefghij"), 0, 11, S(""));
    test(S("abcdefghij"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, 1, S("acdefghij"));
    test(S("abcdefghij"), 1, 4, S("afghij"));
    test(S("abcdefghij"), 1, 8, S("aj"));
    test(S("abcdefghij"), 1, 9, S("a"));
    test(S("abcdefghij"), 1, 10, S("a"));
    test(S("abcdefghij"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, 1, S("abcdeghij"));
    test(S("abcdefghij"), 5, 2, S("abcdehij"));
    test(S("abcdefghij"), 5, 4, S("abcdej"));
    test(S("abcdefghij"), 5, 5, S("abcde"));
    test(S("abcdefghij"), 5, 6, S("abcde"));
    test(S("abcdefghij"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, 1, S("abcdefghi"));
    test(S("abcdefghij"), 9, 2, S("abcdefghi"));
    test(S("abcdefghij"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, S("bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, S("klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 19, S("t"));
    test(S("abcdefghijklmnopqrst"), 0, 20, S(""));
    test(S("abcdefghijklmnopqrst"), 0, 21, S(""));
    test(S("abcdefghijklmnopqrst"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, S("acdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, S("aklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 18, S("at"));
    test(S("abcdefghijklmnopqrst"), 1, 19, S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 20, S("a"));
    test(S("abcdefghijklmnopqrst"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, S("abcdefghijlmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, S("abcdefghijpqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 9, S("abcdefghijt"));
    test(S("abcdefghijklmnopqrst"), 10, 10, S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 11, S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 1, S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 2, S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 21, 0, S("can't happen"));

    test(S(""), 0, S(""));
    test(S(""), 1, S("can't happen"));
    test(S("abcde"), 0, S(""));
    test(S("abcde"), 1, S("a"));
    test(S("abcde"), 2, S("ab"));
    test(S("abcde"), 4, S("abcd"));
    test(S("abcde"), 5, S("abcde"));
    test(S("abcde"), 6, S("can't happen"));
    test(S("abcdefghij"), 0, S(""));
    test(S("abcdefghij"), 1, S("a"));
    test(S("abcdefghij"), 5, S("abcde"));
    test(S("abcdefghij"), 9, S("abcdefghi"));
    test(S("abcdefghij"), 10, S("abcdefghij"));
    test(S("abcdefghij"), 11, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, S(""));
    test(S("abcdefghijklmnopqrst"), 1, S("a"));
    test(S("abcdefghijklmnopqrst"), 10, S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 19, S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 20, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 21, S("can't happen"));

    test(S(""), S(""));
    test(S("abcde"), S(""));
    test(S("abcdefghij"), S(""));
    test(S("abcdefghijklmnopqrst"), S(""));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), 0, 0, S(""));
    test(S(""), 0, 1, S(""));
    test(S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 0, 0, S("abcde"));
    test(S("abcde"), 0, 1, S("bcde"));
    test(S("abcde"), 0, 2, S("cde"));
    test(S("abcde"), 0, 4, S("e"));
    test(S("abcde"), 0, 5, S(""));
    test(S("abcde"), 0, 6, S(""));
    test(S("abcde"), 1, 0, S("abcde"));
    test(S("abcde"), 1, 1, S("acde"));
    test(S("abcde"), 1, 2, S("ade"));
    test(S("abcde"), 1, 3, S("ae"));
    test(S("abcde"), 1, 4, S("a"));
    test(S("abcde"), 1, 5, S("a"));
    test(S("abcde"), 2, 0, S("abcde"));
    test(S("abcde"), 2, 1, S("abde"));
    test(S("abcde"), 2, 2, S("abe"));
    test(S("abcde"), 2, 3, S("ab"));
    test(S("abcde"), 2, 4, S("ab"));
    test(S("abcde"), 4, 0, S("abcde"));
    test(S("abcde"), 4, 1, S("abcd"));
    test(S("abcde"), 4, 2, S("abcd"));
    test(S("abcde"), 5, 0, S("abcde"));
    test(S("abcde"), 5, 1, S("abcde"));
    test(S("abcde"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, 1, S("bcdefghij"));
    test(S("abcdefghij"), 0, 5, S("fghij"));
    test(S("abcdefghij"), 0, 9, S("j"));
    test(S("abcdefghij"), 0, 10, S(""));
    test(S("abcdefghij"), 0, 11, S(""));
    test(S("abcdefghij"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, 1, S("acdefghij"));
    test(S("abcdefghij"), 1, 4, S("afghij"));
    test(S("abcdefghij"), 1, 8, S("aj"));
    test(S("abcdefghij"), 1, 9, S("a"));
    test(S("abcdefghij"), 1, 10, S("a"));
    test(S("abcdefghij"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, 1, S("abcdeghij"));
    test(S("abcdefghij"), 5, 2, S("abcdehij"));
    test(S("abcdefghij"), 5, 4, S("abcdej"));
    test(S("abcdefghij"), 5, 5, S("abcde"));
    test(S("abcdefghij"), 5, 6, S("abcde"));
    test(S("abcdefghij"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, 1, S("abcdefghi"));
    test(S("abcdefghij"), 9, 2, S("abcdefghi"));
    test(S("abcdefghij"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, S("bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, S("klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 19, S("t"));
    test(S("abcdefghijklmnopqrst"), 0, 20, S(""));
    test(S("abcdefghijklmnopqrst"), 0, 21, S(""));
    test(S("abcdefghijklmnopqrst"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, S("acdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, S("aklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 18, S("at"));
    test(S("abcdefghijklmnopqrst"), 1, 19, S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 20, S("a"));
    test(S("abcdefghijklmnopqrst"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, S("abcdefghijlmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, S("abcdefghijpqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 9, S("abcdefghijt"));
    test(S("abcdefghijklmnopqrst"), 10, 10, S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 11, S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 1, S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 2, S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 21, 0, S("can't happen"));

    test(S(""), 0, S(""));
    test(S(""), 1, S("can't happen"));
    test(S("abcde"), 0, S(""));
    test(S("abcde"), 1, S("a"));
    test(S("abcde"), 2, S("ab"));
    test(S("abcde"), 4, S("abcd"));
    test(S("abcde"), 5, S("abcde"));
    test(S("abcde"), 6, S("can't happen"));
    test(S("abcdefghij"), 0, S(""));
    test(S("abcdefghij"), 1, S("a"));
    test(S("abcdefghij"), 5, S("abcde"));
    test(S("abcdefghij"), 9, S("abcdefghi"));
    test(S("abcdefghij"), 10, S("abcdefghij"));
    test(S("abcdefghij"), 11, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, S(""));
    test(S("abcdefghijklmnopqrst"), 1, S("a"));
    test(S("abcdefghijklmnopqrst"), 10, S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 19, S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 20, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 21, S("can't happen"));

    test(S(""), S(""));
    test(S("abcde"), S(""));
    test(S("abcdefghij"), S(""));
    test(S("abcdefghijklmnopqrst"), S(""));
    }
#endif
}
