//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// deque(size_type n, const value_type& v, const allocator_type& a);

#include <deque>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"

template <class T, class Allocator>
void
test(unsigned n, const T& x, const Allocator& a)
{
    typedef std::deque<T, Allocator> C;
    typedef typename C::const_iterator const_iterator;
    C d(n, x, a);
    assert(d.get_allocator() == a);
    assert(d.size() == n);
    assert(static_cast<std::size_t>(distance(d.begin(), d.end())) == d.size());
    for (const_iterator i = d.begin(), e = d.end(); i != e; ++i)
        assert(*i == x);
}

int main()
{
    {
    std::allocator<int> a;
    test(0, 5, a);
    test(1, 10, a);
    test(10, 11, a);
    test(1023, -11, a);
    test(1024, 25, a);
    test(1025, 0, a);
    test(2047, 110, a);
    test(2048, -500, a);
    test(2049, 654, a);
    test(4095, 78, a);
    test(4096, 1165, a);
    test(4097, 157, a);
    }
#if TEST_STD_VER >= 11
    {
    min_allocator<int> a;
    test(0, 5, a);
    test(1, 10, a);
    test(10, 11, a);
    test(1023, -11, a);
    test(1024, 25, a);
    test(1025, 0, a);
    test(2047, 110, a);
    test(2048, -500, a);
    test(2049, 654, a);
    test(4095, 78, a);
    test(4096, 1165, a);
    test(4097, 157, a);
    }
#endif
}
