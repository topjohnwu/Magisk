//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string(const charT* s, const Allocator& a = Allocator());

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class charT>
void
test(const charT* s)
{
    typedef std::basic_string<charT, std::char_traits<charT>, test_allocator<charT> > S;
    typedef typename S::traits_type T;
    typedef typename S::allocator_type A;
    std::size_t n = T::length(s);
    S s2(s);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == n);
    assert(T::compare(s2.data(), s, n) == 0);
    assert(s2.get_allocator() == A());
    assert(s2.capacity() >= s2.size());
}

template <class charT, class A>
void
test(const charT* s, const A& a)
{
    typedef std::basic_string<charT, std::char_traits<charT>, A> S;
    typedef typename S::traits_type T;
    std::size_t n = T::length(s);
    S s2(s, a);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == n);
    assert(T::compare(s2.data(), s, n) == 0);
    assert(s2.get_allocator() == a);
    assert(s2.capacity() >= s2.size());
}

int main()
{
    {
    typedef test_allocator<char> A;

    test("");
    test("", A(2));

    test("1");
    test("1", A(2));

    test("1234567980");
    test("1234567980", A(2));

    test("123456798012345679801234567980123456798012345679801234567980");
    test("123456798012345679801234567980123456798012345679801234567980", A(2));
    }
#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;

    test("");
    test("", A());

    test("1");
    test("1", A());

    test("1234567980");
    test("1234567980", A());

    test("123456798012345679801234567980123456798012345679801234567980");
    test("123456798012345679801234567980123456798012345679801234567980", A());
    }
#endif
}
