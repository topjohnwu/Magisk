//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>
// UNSUPPORTED: c++98, c++03, c++11, c++14

// template<class InputIterator>
//     typename iterator_traits<InputIterator>::value_type
//     reduce(InputIterator first, InputIterator last);

#include <numeric>
#include <cassert>

#include "test_iterators.h"

template <class Iter, class T>
void
test(Iter first, Iter last, T x)
{
    static_assert( std::is_same_v<typename std::iterator_traits<decltype(first)>::value_type,
                                decltype(std::reduce(first, last))> );
    assert(std::reduce(first, last) == x);
}

template <class Iter>
void
test()
{
    int ia[] = {1, 2, 3, 4, 5, 6};
    unsigned sa = sizeof(ia) / sizeof(ia[0]);
    test(Iter(ia), Iter(ia), 0);
    test(Iter(ia), Iter(ia+1), 1);
    test(Iter(ia), Iter(ia+2), 3);
    test(Iter(ia), Iter(ia+sa), 21);
}

template <typename T>
void test_return_type()
{
    T *p = nullptr;
    static_assert( std::is_same_v<T, decltype(std::reduce(p, p))> );
}

int main()
{
    test_return_type<char>();
    test_return_type<int>();
    test_return_type<unsigned long>();
    test_return_type<float>();
    test_return_type<double>();

    test<input_iterator<const int*> >();
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
}
