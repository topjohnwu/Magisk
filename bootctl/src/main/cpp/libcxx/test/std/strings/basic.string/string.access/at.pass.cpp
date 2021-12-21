//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// const_reference at(size_type pos) const;
//       reference at(size_type pos);

#include <string>
#include <stdexcept>
#include <cassert>

#include "min_allocator.h"

#include "test_macros.h"

template <class S>
void
test(S s, typename S::size_type pos)
{
    const S& cs = s;
    if (pos < s.size())
    {
        assert(s.at(pos) == s[pos]);
        assert(cs.at(pos) == cs[pos]);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            TEST_IGNORE_NODISCARD s.at(pos);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos >= s.size());
        }
        try
        {
            TEST_IGNORE_NODISCARD cs.at(pos);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos >= s.size());
        }
    }
#endif
}

int main()
{
    {
    typedef std::string S;
    test(S(), 0);
    test(S("123"), 0);
    test(S("123"), 1);
    test(S("123"), 2);
    test(S("123"), 3);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), 0);
    test(S("123"), 0);
    test(S("123"), 1);
    test(S("123"), 2);
    test(S("123"), 3);
    }
#endif
}
