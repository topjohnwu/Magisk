//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// explicit basic_string(basic_string_view<CharT, traits> sv, const Allocator& a = Allocator());

#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class charT>
void
test(std::basic_string_view<charT> sv)
{
    typedef std::basic_string<charT, std::char_traits<charT>, test_allocator<charT> > S;
    typedef typename S::traits_type T;
    typedef typename S::allocator_type A;
  {
    S s2(sv);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == sv.size());
    assert(T::compare(s2.data(), sv.data(), sv.size()) == 0);
    assert(s2.get_allocator() == A());
    assert(s2.capacity() >= s2.size());
  }
  {
    S s2;
    s2 = sv;
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == sv.size());
    assert(T::compare(s2.data(), sv.data(), sv.size()) == 0);
    assert(s2.get_allocator() == A());
    assert(s2.capacity() >= s2.size());
  }
}

template <class charT, class A>
void
test(std::basic_string_view<charT> sv, const A& a)
{
    typedef std::basic_string<charT, std::char_traits<charT>, A> S;
    typedef typename S::traits_type T;
  {
    S s2(sv, a);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == sv.size());
    assert(T::compare(s2.data(), sv.data(), sv.size()) == 0);
    assert(s2.get_allocator() == a);
    assert(s2.capacity() >= s2.size());
  }
  {
    S s2(a);
    s2 = sv;
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == sv.size());
    assert(T::compare(s2.data(), sv.data(), sv.size()) == 0);
    assert(s2.get_allocator() == a);
    assert(s2.capacity() >= s2.size());
  }
}

int main()
{
    {
    typedef test_allocator<char> A;
    typedef std::basic_string_view<char, std::char_traits<char> > SV;

    test(SV(""));
    test(SV(""), A(2));

    test(SV("1"));
    test(SV("1") ,A(2));

    test(SV("1234567980"));
    test(SV("1234567980"), A(2));

    test(SV("123456798012345679801234567980123456798012345679801234567980"));
    test(SV("123456798012345679801234567980123456798012345679801234567980"), A(2));
    }
#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;
    typedef std::basic_string_view<char, std::char_traits<char> > SV;

    test(SV(""));
    test(SV(""), A());

    test(SV("1"));
    test(SV("1") ,A());

    test(SV("1234567980"));
    test(SV("1234567980"), A());

    test(SV("123456798012345679801234567980123456798012345679801234567980"));
    test(SV("123456798012345679801234567980123456798012345679801234567980"), A());
    }
#endif
}
