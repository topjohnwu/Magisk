//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <queue>

// template <class Alloc>
//     priority_queue(priority_queue&& q, const Alloc& a);

#include <queue>
#include <cassert>

#include "MoveOnly.h"


template <class C>
C
make(int n)
{
    C c;
    for (int i = 0; i < n; ++i)
        c.push_back(MoveOnly(i));
    return c;
}

#include "test_allocator.h"

template <class T>
struct test
    : public std::priority_queue<T, std::vector<T, test_allocator<T> > >
{
    typedef std::priority_queue<T, std::vector<T, test_allocator<T> > > base;
    typedef typename base::container_type container_type;
    typedef typename base::value_compare value_compare;

    explicit test(const test_allocator<int>& a) : base(a) {}
    test(const value_compare& comp, const test_allocator<int>& a)
        : base(comp, c, a) {}
    test(const value_compare& comp, const container_type& c,
        const test_allocator<int>& a) : base(comp, c, a) {}
    test(const value_compare& comp, container_type&& c,
         const test_allocator<int>& a) : base(comp, std::move(c), a) {}
    test(test&& q, const test_allocator<int>& a) : base(std::move(q), a) {}
    test_allocator<int> get_allocator() {return c.get_allocator();}

    using base::c;
};


int main()
{
    test<MoveOnly> qo(std::less<MoveOnly>(),
                      make<std::vector<MoveOnly, test_allocator<MoveOnly> > >(5),
                      test_allocator<MoveOnly>(2));
    test<MoveOnly> q(std::move(qo), test_allocator<MoveOnly>(6));
    assert(q.size() == 5);
    assert(q.c.get_allocator() == test_allocator<MoveOnly>(6));
    assert(q.top() == MoveOnly(4));
}
