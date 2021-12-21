//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// template <class Alloc>
//   queue(const container_type& c, const Alloc& a);

#include <queue>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"

template <class C>
C
make(int n)
{
    C c;
    for (int i = 0; i < n; ++i)
        c.push_back(i);
    return c;
}

typedef std::deque<int, test_allocator<int> > C;

struct test
    : public std::queue<int, C>
{
    typedef std::queue<int, C> base;

    explicit test(const test_allocator<int>& a) : base(a) {}
    test(const container_type& c, const test_allocator<int>& a) : base(c, a) {}
#if TEST_STD_VER >= 11
    test(container_type&& c, const test_allocator<int>& a) : base(std::move(c), a) {}
    test(test&& q, const test_allocator<int>& a) : base(std::move(q), a) {}
#endif
    test_allocator<int> get_allocator() {return c.get_allocator();}
};

int main()
{
    C d = make<C>(5);
    test q(d, test_allocator<int>(4));
    assert(q.get_allocator() == test_allocator<int>(4));
    assert(q.size() == 5);
    for (C::size_type i = 0; i < d.size(); ++i)
    {
        assert(q.front() == d[i]);
        q.pop();
    }
}
