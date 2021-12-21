//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string(size_type n, charT c, const Allocator& a = Allocator());

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
test(unsigned n, charT c)
{
    typedef std::basic_string<charT, std::char_traits<charT>, test_allocator<charT> > S;
    typedef typename S::allocator_type A;
    S s2(n, c);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == n);
    for (unsigned i = 0; i < n; ++i)
        assert(s2[i] == c);
    assert(s2.get_allocator() == A());
    assert(s2.capacity() >= s2.size());
}

template <class charT, class A>
void
test(unsigned n, charT c, const A& a)
{
    typedef std::basic_string<charT, std::char_traits<charT>, A> S;
    S s2(n, c, a);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == n);
    for (unsigned i = 0; i < n; ++i)
        assert(s2[i] == c);
    assert(s2.get_allocator() == a);
    assert(s2.capacity() >= s2.size());
}

template <class Tp>
void
test(Tp n, Tp c)
{
    typedef char charT;
    typedef std::basic_string<charT, std::char_traits<charT>, test_allocator<charT> > S;
    typedef typename S::allocator_type A;
    S s2(n, c);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
        assert(s2[i] == c);
    assert(s2.get_allocator() == A());
    assert(s2.capacity() >= s2.size());
}

template <class Tp, class A>
void
test(Tp n, Tp c, const A& a)
{
    typedef char charT;
    typedef std::basic_string<charT, std::char_traits<charT>, A> S;
    S s2(n, c, a);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
        assert(s2[i] == c);
    assert(s2.get_allocator() == a);
    assert(s2.capacity() >= s2.size());
}

int main()
{
    {
    typedef test_allocator<char> A;

    test(0, 'a');
    test(0, 'a', A(2));

    test(1, 'a');
    test(1, 'a', A(2));

    test(10, 'a');
    test(10, 'a', A(2));

    test(100, 'a');
    test(100, 'a', A(2));

    test(static_cast<char>(100), static_cast<char>(65));
    test(static_cast<char>(100), static_cast<char>(65), A(3));
    }
#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;

    test(0, 'a');
    test(0, 'a', A());

    test(1, 'a');
    test(1, 'a', A());

    test(10, 'a');
    test(10, 'a', A());

    test(100, 'a');
    test(100, 'a', A());

    test(static_cast<char>(100), static_cast<char>(65));
    test(static_cast<char>(100), static_cast<char>(65), A());
    }
#endif
}
