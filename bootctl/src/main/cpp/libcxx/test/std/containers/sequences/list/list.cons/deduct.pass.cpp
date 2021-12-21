//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides


// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    list(InputIterator, InputIterator, Allocator = Allocator())
//    -> list<typename iterator_traits<InputIterator>::value_type, Allocator>;
//


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
    const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::list lst(std::begin(arr), std::end(arr));

    static_assert(std::is_same_v<decltype(lst), std::list<int>>, "");
    assert(std::equal(lst.begin(), lst.end(), std::begin(arr), std::end(arr)));
    }

    {
    const long arr[] = {INT_MAX, 1L, 2L, 3L };
    std::list lst(std::begin(arr), std::end(arr), std::allocator<long>());
    static_assert(std::is_same_v<decltype(lst)::value_type, long>, "");
    assert(lst.size() == 4);
    auto it = lst.begin();
    assert(*it++ == INT_MAX);
    assert(*it++ == 1L);
    assert(*it++ == 2L);
    }

//  Test the implicit deduction guides

    {
//  We don't expect this one to work.
//  std::list lst(std::allocator<int>()); // list (allocator &)
    }

    {
    std::list lst(1, A{}); // list (size_type, T)
    static_assert(std::is_same_v<decltype(lst)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, std::allocator<A>>, "");
    assert(lst.size() == 1);
    }

    {
    std::list lst(1, A{}, test_allocator<A>()); // list (size_type, T, allocator)
    static_assert(std::is_same_v<decltype(lst)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, test_allocator<A>>, "");
    assert(lst.size() == 1);
    }

    {
    std::list lst{1U, 2U, 3U, 4U, 5U}; // list(initializer-list)
    static_assert(std::is_same_v<decltype(lst)::value_type, unsigned>, "");
    assert(lst.size() == 5);
    auto it = lst.begin();
    std::advance(it, 2);
    assert(*it == 3U);
    }

    {
    std::list lst({1.0, 2.0, 3.0, 4.0}, test_allocator<double>()); // list(initializer-list, allocator)
    static_assert(std::is_same_v<decltype(lst)::value_type, double>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, test_allocator<double>>, "");
    assert(lst.size() == 4);
    auto it = lst.begin();
    std::advance(it, 3);
    assert(*it == 4.0);
    }

    {
    std::list<long double> source;
    std::list lst(source); // list(list &)
    static_assert(std::is_same_v<decltype(lst)::value_type, long double>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, std::allocator<long double>>, "");
    assert(lst.size() == 0);
    }
}
