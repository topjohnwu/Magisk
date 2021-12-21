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
//     priority_queue(const Compare& comp, container_type&& c,
//                    const Alloc& a);

#include <queue>
#include <cassert>

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

template <class T>
struct test
    : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
{
    typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
    typedef typename base::container_type container_type;
    typedef typename base::value_compare value_compare;

    explicit test(const test_allocator<int>& a) : base(a) {}
    test(const value_compare& comp, const test_allocator<int>& a)
        : base(comp, a) {}
    test(const value_compare& comp, const container_type& c,
        const test_allocator<int>& a) : base(comp, c, a) {}
#if TEST_STD_VER >= 11 // testing rvalue ctor
    test(const value_compare& comp, container_type&& c,
         const test_allocator<int>& a) : base(comp, std::move(c), a) {}
    test(test&& q, const test_allocator<int>& a) : base(std::move(q), a) {}
#endif
    test_allocator<int> get_allocator() {return c.get_allocator();}

    using base::c;
};

int main()
{
    typedef std::vector<int, test_allocator<int> > C;
    test<int> q(std::less<int>(), make<C>(5), test_allocator<int>(3));
    assert(q.c.get_allocator() == test_allocator<int>(3));
    assert(q.size() == 5);
    assert(q.top() == 4);
}
