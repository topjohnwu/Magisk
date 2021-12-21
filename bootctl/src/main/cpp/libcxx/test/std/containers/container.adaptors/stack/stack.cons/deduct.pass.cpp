//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-5, apple-clang-9
// UNSUPPORTED: libcpp-no-deduction-guides
// Clang 5 will generate bad implicit deduction guides
//	Specifically, for the copy constructor.


// template<class Container>
//   stack(Container) -> stack<typename Container::value_type, Container>;
//
// template<class Container, class Allocator>
//   stack(Container, Allocator) -> stack<typename Container::value_type, Container>;


#include <stack>
#include <vector>
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
    std::vector<int> v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::stack stk(v);

    static_assert(std::is_same_v<decltype(stk), std::stack<int, std::vector<int>>>, "");
    assert(stk.size() == v.size());
    assert(stk.top() == v.back());
    }

    {
    std::list<long, test_allocator<long>> l{10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    std::stack stk(l, test_allocator<long>(0,2)); // different allocator
    static_assert(std::is_same_v<decltype(stk)::container_type, std::list<long, test_allocator<long>>>, "");
    static_assert(std::is_same_v<decltype(stk)::value_type, long>, "");
    assert(stk.size() == 10);
    assert(stk.top() == 19);
//  I'd like to assert that we've gotten the right allocator in the stack, but
//  I don't know how to get at the underlying container.
    }

//  Test the implicit deduction guides

    {
//  We don't expect this one to work - no way to implicitly get value_type
//  std::stack stk(std::allocator<int>()); // stack (allocator &)
    }

    {
    std::stack<A> source;
    std::stack stk(source); // stack(stack &)
    static_assert(std::is_same_v<decltype(stk)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(stk)::container_type, std::deque<A>>, "");
    assert(stk.size() == 0);
    }

    {
//  This one is odd - you can pass an allocator in to use, but the allocator
//  has to match the type of the one used by the underlying container
    typedef short T;
    typedef test_allocator<T> A;
    typedef std::deque<T, A> C;

    C c{0,1,2,3};
    std::stack<T, C> source(c);
    std::stack stk(source, A(2)); // stack(stack &, allocator)
    static_assert(std::is_same_v<decltype(stk)::value_type, T>, "");
    static_assert(std::is_same_v<decltype(stk)::container_type, C>, "");
    assert(stk.size() == 4);
    assert(stk.top() == 3);
    }

}
