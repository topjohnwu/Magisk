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

// template<class InputIterator, class T>
//   T reduce(InputIterator first, InputIterator last, T init);

#include <numeric>
#include <cassert>

#include "test_iterators.h"

template <class Iter, class T>
void
test(Iter first, Iter last, T init, T x)
{
    static_assert( std::is_same_v<T, decltype(std::reduce(first, last, init))> );
    assert(std::reduce(first, last, init) == x);
}

template <class Iter>
void
test()
{
    int ia[] = {1, 2, 3, 4, 5, 6};
    unsigned sa = sizeof(ia) / sizeof(ia[0]);
    test(Iter(ia), Iter(ia), 0, 0);
    test(Iter(ia), Iter(ia), 1, 1);
    test(Iter(ia), Iter(ia+1), 0, 1);
    test(Iter(ia), Iter(ia+1), 2, 3);
    test(Iter(ia), Iter(ia+2), 0, 3);
    test(Iter(ia), Iter(ia+2), 3, 6);
    test(Iter(ia), Iter(ia+sa), 0, 21);
    test(Iter(ia), Iter(ia+sa), 4, 25);
}

template <typename T, typename Init>
void test_return_type()
{
    T *p = nullptr;
    static_assert( std::is_same_v<Init, decltype(std::reduce(p, p, Init{}))> );
}

int main()
{
    test_return_type<char, int>();
    test_return_type<int, int>();
    test_return_type<int, unsigned long>();
    test_return_type<float, int>();
    test_return_type<short, float>();
    test_return_type<double, char>();
    test_return_type<char, double>();

    test<input_iterator<const int*> >();
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
}
