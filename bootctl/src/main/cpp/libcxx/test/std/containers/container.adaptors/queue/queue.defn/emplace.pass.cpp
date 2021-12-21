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

// template <class... Args> decltype(auto) emplace(Args&&... args);
// return type is 'decltype(auto)' in C++17; 'void' before
// whatever the return type of the underlying container's emplace_back() returns.


#include <queue>
#include <cassert>
#include <list>

#include "test_macros.h"

#include "../../../Emplaceable.h"

template <typename Queue>
void test_return_type() {
    typedef typename Queue::container_type Container;
    typedef typename Container::value_type value_type;
    typedef decltype(std::declval<Queue>().emplace(std::declval<value_type &>())) queue_return_type;

#if TEST_STD_VER > 14
    typedef decltype(std::declval<Container>().emplace_back(std::declval<value_type>())) container_return_type;
    static_assert(std::is_same<queue_return_type, container_return_type>::value, "");
#else
    static_assert(std::is_same<queue_return_type, void>::value, "");
#endif
}

int main()
{
    test_return_type<std::queue<int> > ();
    test_return_type<std::queue<int, std::list<int> > > ();

    std::queue<Emplaceable> q;
#if TEST_STD_VER > 14
    typedef Emplaceable T;
    T& r1 = q.emplace(1, 2.5);
    assert(&r1 == &q.back());
    T& r2 = q.emplace(2, 3.5);
    assert(&r2 == &q.back());
    T& r3 = q.emplace(3, 4.5);
    assert(&r3 == &q.back());
    assert(&r1 == &q.front());
#else
    q.emplace(1, 2.5);
    q.emplace(2, 3.5);
    q.emplace(3, 4.5);
#endif

    assert(q.size() == 3);
    assert(q.front() == Emplaceable(1, 2.5));
    assert(q.back() == Emplaceable(3, 4.5));
}
