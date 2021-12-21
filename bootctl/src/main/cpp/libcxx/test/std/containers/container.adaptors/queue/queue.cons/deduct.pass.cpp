//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-5, apple-clang-9
// UNSUPPORTED: libcpp-no-deduction-guides
// Clang 5 will generate bad implicit deduction guides
//  Specifically, for the copy constructor.

// template<class Container>
//   queue(Container) -> queue<typename Container::value_type, Container>;
//
// template<class Container, class Allocator>
//   queue(Container, Allocator) -> queue<typename Container::value_type, Container>;


#include <queue>
#include <list>
#include <iterator>
#include <cassert>
#include <cstddef>
#include <climits> // INT_MAX

#include "test_macros.h"
#include "test_iterators.h"
#include "test_allocator.h"

struct A {};

int main()
{

//  Test the explicit deduction guides
    {
    std::list<int> l{0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::queue que(l);

    static_assert(std::is_same_v<decltype(que), std::queue<int, std::list<int>>>, "");
    assert(que.size() == l.size());
    assert(que.back() == l.back());
    }

    {
    std::list<long, test_allocator<long>> l{10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    std::queue que(l, test_allocator<long>(0,2)); // different allocator
    static_assert(std::is_same_v<decltype(que)::container_type, std::list<long, test_allocator<long>>>, "");
    static_assert(std::is_same_v<decltype(que)::value_type, long>, "");
    assert(que.size() == 10);
    assert(que.back() == 19);
//  I'd like to assert that we've gotten the right allocator in the queue, but
//  I don't know how to get at the underlying container.
    }

//  Test the implicit deduction guides
    {
//  We don't expect this one to work - no way to implicitly get value_type
//  std::queue que(std::allocator<int>()); // queue (allocator &)
    }

    {
    std::queue<A> source;
    std::queue que(source); // queue(queue &)
    static_assert(std::is_same_v<decltype(que)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(que)::container_type, std::deque<A>>, "");
    assert(que.size() == 0);
    }

    {
//  This one is odd - you can pass an allocator in to use, but the allocator
//  has to match the type of the one used by the underlying container
    typedef short T;
    typedef test_allocator<T> A;
    typedef std::deque<T, A> C;

    C c{0,1,2,3};
    std::queue<T, C> source(c);
    std::queue que(source, A(2)); // queue(queue &, allocator)
    static_assert(std::is_same_v<decltype(que)::value_type, T>, "");
    static_assert(std::is_same_v<decltype(que)::container_type, C>, "");
    assert(que.size() == 4);
    assert(que.back() == 3);
    }

}
