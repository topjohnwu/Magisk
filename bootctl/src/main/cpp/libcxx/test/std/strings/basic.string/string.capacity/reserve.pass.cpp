//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// Split into two calls for C++20
// void reserve();
// void reserve(size_type res_arg);

#include <string>
#include <stdexcept>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s)
{
    typename S::size_type old_cap = s.capacity();
    S s0 = s;
    s.reserve();
    LIBCPP_ASSERT(s.__invariants());
    assert(s == s0);
    assert(s.capacity() <= old_cap);
    assert(s.capacity() >= s.size());
}

template <class S>
void
test(S s, typename S::size_type res_arg)
{
    typename S::size_type old_cap = s.capacity();
    ((void)old_cap); // Prevent unused warning
    S s0 = s;
    if (res_arg <= s.max_size())
    {
        s.reserve(res_arg);
        assert(s == s0);
        assert(s.capacity() >= res_arg);
        assert(s.capacity() >= s.size());
#if TEST_STD_VER > 17
        assert(s.capacity() >= old_cap); // resize never shrinks as of P0966
#endif
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.reserve(res_arg);
            assert(false);
        }
        catch (std::length_error&)
        {
            assert(res_arg > s.max_size());
        }
    }
#endif
}

int main()
{
    {
    typedef std::string S;
    {
    S s;
    test(s);

    s.assign(10, 'a');
    s.erase(5);
    test(s);

    s.assign(100, 'a');
    s.erase(50);
    test(s);
    }
    {
    S s;
    test(s, 5);
    test(s, 10);
    test(s, 50);
    }
    {
    S s(100, 'a');
    s.erase(50);
    test(s, 5);
    test(s, 10);
    test(s, 50);
    test(s, 100);
    test(s, 1000);
    test(s, S::npos);
    }
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    {
    S s;
    test(s);

    s.assign(10, 'a');
    s.erase(5);
    test(s);

    s.assign(100, 'a');
    s.erase(50);
    test(s);
    }
    {
    S s;
    test(s, 5);
    test(s, 10);
    test(s, 50);
    }
    {
    S s(100, 'a');
    s.erase(50);
    test(s, 5);
    test(s, 10);
    test(s, 50);
    test(s, 100);
    test(s, 1000);
    test(s, S::npos);
    }
    }
#endif
}
