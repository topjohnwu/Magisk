//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// explicit deque(size_type n);

#include <deque>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"
#include "DefaultOnly.h"
#include "min_allocator.h"

template <class T, class Allocator>
void
test2(unsigned n)
{
#if TEST_STD_VER > 11
    typedef std::deque<T, Allocator> C;
    typedef typename C::const_iterator const_iterator;
    assert(DefaultOnly::count == 0);
    {
    C d(n, Allocator());
    assert(static_cast<unsigned>(DefaultOnly::count) == n);
    assert(d.size() == n);
    assert(static_cast<std::size_t>(distance(d.begin(), d.end())) == d.size());
    for (const_iterator i = d.begin(), e = d.end(); i != e; ++i)
        assert(*i == T());
    }
    assert(DefaultOnly::count == 0);
#else
    ((void)n);
#endif
}

template <class T, class Allocator>
void
test1(unsigned n)
{
    typedef std::deque<T, Allocator> C;
    typedef typename C::const_iterator const_iterator;
    assert(DefaultOnly::count == 0);
    {
    C d(n);
    assert(static_cast<unsigned>(DefaultOnly::count) == n);
    assert(d.size() == n);
    assert(static_cast<std::size_t>(distance(d.begin(), d.end())) == d.size());
#if TEST_STD_VER >= 11
    for (const_iterator i = d.begin(), e = d.end(); i != e; ++i)
        assert(*i == T());
#endif
    }
    assert(DefaultOnly::count == 0);
}

template <class T, class Allocator>
void
test3(unsigned n, Allocator const &alloc = Allocator())
{
#if TEST_STD_VER > 11
    typedef std::deque<T, Allocator> C;
    {
    C d(n, alloc);
    assert(d.size() == n);
    assert(d.get_allocator() == alloc);
    }
#else
    ((void)n);
    ((void)alloc);
#endif
}

template <class T, class Allocator>
void
test(unsigned n)
{
    test1<T, Allocator> ( n );
    test2<T, Allocator> ( n );
}

int main()
{
    test<DefaultOnly, std::allocator<DefaultOnly> >(0);
    test<DefaultOnly, std::allocator<DefaultOnly> >(1);
    test<DefaultOnly, std::allocator<DefaultOnly> >(10);
    test<DefaultOnly, std::allocator<DefaultOnly> >(1023);
    test<DefaultOnly, std::allocator<DefaultOnly> >(1024);
    test<DefaultOnly, std::allocator<DefaultOnly> >(1025);
    test<DefaultOnly, std::allocator<DefaultOnly> >(2047);
    test<DefaultOnly, std::allocator<DefaultOnly> >(2048);
    test<DefaultOnly, std::allocator<DefaultOnly> >(2049);
    test<DefaultOnly, std::allocator<DefaultOnly> >(4095);
    test<DefaultOnly, std::allocator<DefaultOnly> >(4096);
    test<DefaultOnly, std::allocator<DefaultOnly> >(4097);

    LIBCPP_ONLY(test1<DefaultOnly, limited_allocator<DefaultOnly, 4096> >(4095));

#if TEST_STD_VER >= 11
    test<DefaultOnly, min_allocator<DefaultOnly> >(4095);
#endif

#if TEST_STD_VER > 11
    test3<DefaultOnly, std::allocator<DefaultOnly>> (1023);
    test3<int, std::allocator<int>>(1);
    test3<int, min_allocator<int>> (3);
#endif

}
