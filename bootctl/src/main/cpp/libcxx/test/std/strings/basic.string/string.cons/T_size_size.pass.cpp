//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class _Tp>
//   basic_string(const _Tp& __t, size_type __pos, size_type __n,
//                const allocator_type& __a = allocator_type());
//
//  Mostly we're testing string_view here

#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class S, class SV>
void
test(SV sv, std::size_t pos, std::size_t n)
{
    typedef typename S::traits_type T;
    typedef typename S::allocator_type A;
    typedef typename S::size_type Size;
    if (pos <= sv.size())
    {
        S s2(sv, static_cast<Size>(pos), static_cast<Size>(n));
        LIBCPP_ASSERT(s2.__invariants());
        assert(pos <= sv.size());
        std::size_t rlen = std::min(sv.size() - pos, n);
        assert(s2.size() == rlen);
        assert(T::compare(s2.data(), sv.data() + pos, rlen) == 0);
        assert(s2.get_allocator() == A());
        assert(s2.capacity() >= s2.size());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            S s2(sv, static_cast<Size>(pos), static_cast<Size>(n));
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > sv.size());
        }
    }
#endif
}

template <class S, class SV>
void
test(SV sv, std::size_t pos, std::size_t n, const typename S::allocator_type& a)
{
    typedef typename S::traits_type T;
    typedef typename S::size_type Size;
    if (pos <= sv.size())
    {
        S s2(sv, static_cast<Size>(pos), static_cast<Size>(n), a);
        LIBCPP_ASSERT(s2.__invariants());
        assert(pos <= sv.size());
        std::size_t rlen = std::min(sv.size() - pos, n);
        assert(s2.size() == rlen);
        assert(T::compare(s2.data(), sv.data() + pos, rlen) == 0);
        assert(s2.get_allocator() == a);
        assert(s2.capacity() >= s2.size());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            S s2(sv, static_cast<Size>(pos), static_cast<Size>(n), a);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > sv.size());
        }
    }
#endif
}

int main()
{

    {
    typedef test_allocator<char> A;
    typedef std::basic_string_view<char, std::char_traits<char> > SV;
    typedef std::basic_string     <char, std::char_traits<char>, A> S;

    test<S,SV>(SV(), 0, 0);
    test<S,SV>(SV(), 0, 1);
    test<S,SV>(SV(), 1, 0);
    test<S,SV>(SV(), 1, 1);
    test<S,SV>(SV(), 1, 2);
    test<S,SV>(SV("1"), 0, 0);
    test<S,SV>(SV("1"), 0, 1);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50,   0);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50,   1);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50,  10);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 100);

    test<S,SV>(SV(), 0, 0, A(4));
    test<S,SV>(SV(), 0, 1, A(4));
    test<S,SV>(SV(), 1, 0, A(4));
    test<S,SV>(SV(), 1, 1, A(4));
    test<S,SV>(SV(), 1, 2, A(4));
    test<S,SV>(SV("1"), 0, 0, A(6));
    test<S,SV>(SV("1"), 0, 1, A(6));
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50,   0, A(8));
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50,   1, A(8));
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50,  10, A(8));
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 100, A(8));
    }

#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;
    typedef std::basic_string_view<char, std::char_traits<char> > SV;
    typedef std::basic_string     <char, std::char_traits<char>, A> S;

    test<S,SV>(SV(), 0, 0);
    test<S,SV>(SV(), 0, 1);
    test<S,SV>(SV(), 1, 0);
    test<S,SV>(SV(), 1, 1);
    test<S,SV>(SV(), 1, 2);
    test<S,SV>(SV("1"), 0, 0);
    test<S,SV>(SV("1"), 0, 1);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 0);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 1);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 10);
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 100);

    test<S,SV>(SV(), 0, 0, A());
    test<S,SV>(SV(), 0, 1, A());
    test<S,SV>(SV(), 1, 0, A());
    test<S,SV>(SV(), 1, 1, A());
    test<S,SV>(SV(), 1, 2, A());
    test<S,SV>(SV("1"), 0, 0, A());
    test<S,SV>(SV("1"), 0, 1, A());
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 0, A());
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 1, A());
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 10, A());
    test<S,SV>(SV("1234567890123456789012345678901234567890123456789012345678901234567890"), 50, 100, A());
    }
#endif
    {
    typedef std::string S;
    typedef std::string_view SV;
    S s = "ABCD";
    SV sv = "EFGH";
    char arr[] = "IJKL";

    S s1("CDEF", 4);    // calls ctor(const char *, len)
    assert(s1 == "CDEF");

    S s2("QRST", 0, 3); // calls ctor(string("QRST", pos, len)
    assert(s2 == "QRS");

    S s3(sv, 0, std::string::npos);   // calls ctor(T, pos, npos)
    assert(s3 == sv);

    S s4(sv, 0, 3);   // calls ctor(T, pos, len)
    assert(s4 == "EFG");

    S s5(arr, 0, 2);     // calls ctor(const char *, len)
    assert(s5 == "IJ");

    S s6(arr, 0);     // calls ctor(const char *, len)
    assert(s6 == "");

    S s7(s.data(), 2);     // calls ctor(const char *, len)
    assert(s7 == "AB");
    }
}
