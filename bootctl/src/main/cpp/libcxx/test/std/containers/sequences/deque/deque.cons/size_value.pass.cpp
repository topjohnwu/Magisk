//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// deque(size_type n, const value_type& v);

#include <deque>
#include <cassert>
#include <cstddef>

#include "test_allocator.h"
#include "min_allocator.h"

template <class T, class Allocator>
void
test(unsigned n, const T& x)
{
    typedef std::deque<T, Allocator> C;
    typedef typename C::const_iterator const_iterator;
    C d(n, x);
    assert(d.size() == n);
    assert(static_cast<std::size_t>(distance(d.begin(), d.end())) == d.size());
    for (const_iterator i = d.begin(), e = d.end(); i != e; ++i)
        assert(*i == x);
}

int main()
{
    test<int, std::allocator<int> >(0, 5);
    test<int, std::allocator<int> >(1, 10);
    test<int, std::allocator<int> >(10, 11);
    test<int, std::allocator<int> >(1023, -11);
    test<int, std::allocator<int> >(1024, 25);
    test<int, std::allocator<int> >(1025, 0);
    test<int, std::allocator<int> >(2047, 110);
    test<int, std::allocator<int> >(2048, -500);
    test<int, std::allocator<int> >(2049, 654);
    test<int, std::allocator<int> >(4095, 78);
    test<int, std::allocator<int> >(4096, 1165);
    test<int, std::allocator<int> >(4097, 157);
    LIBCPP_ONLY(test<int, limited_allocator<int, 4096> >(4095, 90));
#if TEST_STD_VER >= 11
    test<int, min_allocator<int> >(4095, 90);
#endif
}
