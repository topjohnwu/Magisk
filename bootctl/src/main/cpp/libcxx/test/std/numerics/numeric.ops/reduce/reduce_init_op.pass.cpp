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

// template<class InputIterator, class T, class BinaryOperation>
//   T reduce(InputIterator first, InputIterator last, T init, BinaryOperation op);

#include <numeric>
#include <cassert>

#include "test_iterators.h"

template <class Iter, class T, class Op>
void
test(Iter first, Iter last, T init, Op op, T x)
{
    static_assert( std::is_same_v<T, decltype(std::reduce(first, last, init, op))>, "" );
    assert(std::reduce(first, last, init, op) == x);
}

template <class Iter>
void
test()
{
    int ia[] = {1, 2, 3, 4, 5, 6};
    unsigned sa = sizeof(ia) / sizeof(ia[0]);
    test(Iter(ia), Iter(ia), 0, std::plus<>(), 0);
    test(Iter(ia), Iter(ia), 1, std::multiplies<>(), 1);
    test(Iter(ia), Iter(ia+1), 0, std::plus<>(), 1);
    test(Iter(ia), Iter(ia+1), 2, std::multiplies<>(), 2);
    test(Iter(ia), Iter(ia+2), 0, std::plus<>(), 3);
    test(Iter(ia), Iter(ia+2), 3, std::multiplies<>(), 6);
    test(Iter(ia), Iter(ia+sa), 0, std::plus<>(), 21);
    test(Iter(ia), Iter(ia+sa), 4, std::multiplies<>(), 2880);
}

template <typename T, typename Init>
void test_return_type()
{
    T *p = nullptr;
    static_assert( std::is_same_v<Init, decltype(std::reduce(p, p, Init{}, std::plus<>()))>, "" );
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

//  Make sure the math is done using the correct type
    {
    auto v = {1, 2, 3, 4, 5, 6, 7, 8};
    unsigned res = std::reduce(v.begin(), v.end(), 1U, std::multiplies<>());
    assert(res == 40320);       // 8! will not fit into a char
    }
}
