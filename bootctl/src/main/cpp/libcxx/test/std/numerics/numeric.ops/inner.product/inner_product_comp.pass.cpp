//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>

// template <InputIterator Iter1, InputIterator Iter2, MoveConstructible T,
//           class BinaryOperation1,
//           Callable<auto, Iter1::reference, Iter2::reference> BinaryOperation2>
//   requires Callable<BinaryOperation1, const T&, BinaryOperation2::result_type>
//         && HasAssign<T, BinaryOperation1::result_type>
//         && CopyConstructible<BinaryOperation1>
//         && CopyConstructible<BinaryOperation2>
//   T
//   inner_product(Iter1 first1, Iter1 last1, Iter2 first2,
//                 T init, BinaryOperation1 binary_op1, BinaryOperation2 binary_op2);

#include <numeric>
#include <functional>
#include <cassert>

#include "test_iterators.h"

template <class Iter1, class Iter2, class T>
void
test(Iter1 first1, Iter1 last1, Iter2 first2, T init, T x)
{
    assert(std::inner_product(first1, last1, first2, init,
           std::multiplies<int>(), std::plus<int>()) == x);
}

template <class Iter1, class Iter2>
void
test()
{
    int a[] = {1, 2, 3, 4, 5, 6};
    int b[] = {6, 5, 4, 3, 2, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    test(Iter1(a), Iter1(a), Iter2(b), 1, 1);
    test(Iter1(a), Iter1(a), Iter2(b), 10, 10);
    test(Iter1(a), Iter1(a+1), Iter2(b), 1, 7);
    test(Iter1(a), Iter1(a+1), Iter2(b), 10, 70);
    test(Iter1(a), Iter1(a+2), Iter2(b), 1, 49);
    test(Iter1(a), Iter1(a+2), Iter2(b), 10, 490);
    test(Iter1(a), Iter1(a+sa), Iter2(b), 1, 117649);
    test(Iter1(a), Iter1(a+sa), Iter2(b), 10, 1176490);
}

int main()
{
    test<input_iterator<const int*>, input_iterator<const int*> >();
    test<input_iterator<const int*>, forward_iterator<const int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<input_iterator<const int*>, random_access_iterator<const int*> >();
    test<input_iterator<const int*>, const int*>();

    test<forward_iterator<const int*>, input_iterator<const int*> >();
    test<forward_iterator<const int*>, forward_iterator<const int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<forward_iterator<const int*>, random_access_iterator<const int*> >();
    test<forward_iterator<const int*>, const int*>();

    test<bidirectional_iterator<const int*>, input_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, const int*>();

    test<random_access_iterator<const int*>, input_iterator<const int*> >();
    test<random_access_iterator<const int*>, forward_iterator<const int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<const int*> >();
    test<random_access_iterator<const int*>, const int*>();

    test<const int*, input_iterator<const int*> >();
    test<const int*, forward_iterator<const int*> >();
    test<const int*, bidirectional_iterator<const int*> >();
    test<const int*, random_access_iterator<const int*> >();
    test<const int*, const int*>();
}
