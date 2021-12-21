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
// UNSUPPORTED: libcpp-no-deduction-guides

// template<class Compare, class Container>
// priority_queue(Compare, Container)
//     -> priority_queue<typename Container::value_type, Container, Compare>;
//
// template<class InputIterator,
//          class Compare = less<typename iterator_traits<InputIterator>::value_type>,
//          class Container = vector<typename iterator_traits<InputIterator>::value_type>>
// priority_queue(InputIterator, InputIterator, Compare = Compare(), Container = Container())
//     -> priority_queue<typename iterator_traits<InputIterator>::value_type, Container, Compare>;
//
// template<class Compare, class Container, class Allocator>
// priority_queue(Compare, Container, Allocator)
//     -> priority_queue<typename Container::value_type, Container, Compare>;


#include <queue>
#include <vector>
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
    std::priority_queue pri(std::greater<int>(), v); // priority_queue(Compare, Container)

    static_assert(std::is_same_v<decltype(pri), std::priority_queue<int, std::vector<int>, std::greater<int>>>, "");
    assert(pri.size() == v.size());
    assert(pri.top() == 0);
    }

    {
    std::vector<long, test_allocator<long>> v{10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    std::priority_queue pri(std::greater<long>(), v, test_allocator<long>(2)); // priority_queue(Compare, Container, Allocator)

    static_assert(std::is_same_v<decltype(pri),
                                 std::priority_queue<long, std::vector<long, test_allocator<long>>, std::greater<long>>>, "");
    assert(pri.size() == v.size());
    assert(pri.top() == 10);
    }

    {
    std::vector<short> v{10, 11, 12, 13, 14, 15, 28, 17, 18, 19 };
    std::priority_queue pri(v.begin(), v.end()); // priority_queue(Iter, Iter)

    static_assert(std::is_same_v<decltype(pri), std::priority_queue<short>>, "");
    assert(pri.size() == v.size());
    assert(pri.top() == 28);
    }

    {
    std::vector<double> v{10, 11, 12, 13, 6, 15, 28, 17, 18, 19 };
    std::priority_queue pri(v.begin(), v.end(), std::greater<double>()); // priority_queue(Iter, Iter, Comp)

    static_assert(std::is_same_v<decltype(pri), std::priority_queue<double, std::vector<double>, std::greater<double>>>, "");
    assert(pri.size() == v.size());
    assert(pri.top() == 6);
    }

    {
    std::vector<double> v{10, 6, 15, 28, 4, 18, 19 };
    std::deque<double> deq;
    std::priority_queue pri(v.begin(), v.end(), std::greater<double>(), deq); // priority_queue(Iter, Iter, Comp, Container)

    static_assert(std::is_same_v<decltype(pri), std::priority_queue<double, std::deque<double>, std::greater<double>>>, "");
    assert(pri.size() == v.size());
    assert(pri.top() == 4);
    }

//  Test the implicit deduction guides
    {
//  We don't expect this one to work - no way to implicitly get value_type
//  std::priority_queue pri(std::allocator<int>()); // queue (allocator &)
    }

    {
    std::priority_queue<float> source;
    std::priority_queue pri(source); // priority_queue(priority_queue &)
    static_assert(std::is_same_v<decltype(pri)::value_type, float>, "");
    static_assert(std::is_same_v<decltype(pri)::container_type, std::vector<float>>, "");
    assert(pri.size() == 0);
    }

    {
//  This one is odd - you can pass an allocator in to use, but the allocator
//  has to match the type of the one used by the underlying container
    typedef long double T;
    typedef std::greater<T> Comp;
    typedef test_allocator<T> A;
    typedef std::deque<T, A> Cont;

    Cont c{2,3,0,1};
    std::priority_queue<T, Cont, Comp> source(Comp(), c);
    std::priority_queue pri(source, A(2)); // queue(queue &, allocator)
    static_assert(std::is_same_v<decltype(pri)::value_type, T>, "");
    static_assert(std::is_same_v<decltype(pri)::container_type, Cont>, "");
    assert(pri.size() == 4);
    assert(pri.top() == 0);
    }
}
