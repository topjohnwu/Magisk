//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>

// template <InputIterator Iter, MoveConstructible T>
//   requires HasPlus<T, Iter::reference>
//         && HasAssign<T, HasPlus<T, Iter::reference>::result_type>
//   T
//   accumulate(Iter first, Iter last, T init);

#include <numeric>
#include <cassert>

#include "test_iterators.h"

template <class Iter, class T>
void
test(Iter first, Iter last, T init, T x)
{
    assert(std::accumulate(first, last, init) == x);
}

template <class Iter>
void
test()
{
    int ia[] = {1, 2, 3, 4, 5, 6};
    unsigned sa = sizeof(ia) / sizeof(ia[0]);
    test(Iter(ia), Iter(ia), 0, 0);
    test(Iter(ia), Iter(ia), 10, 10);
    test(Iter(ia), Iter(ia+1), 0, 1);
    test(Iter(ia), Iter(ia+1), 10, 11);
    test(Iter(ia), Iter(ia+2), 0, 3);
    test(Iter(ia), Iter(ia+2), 10, 13);
    test(Iter(ia), Iter(ia+sa), 0, 21);
    test(Iter(ia), Iter(ia+sa), 10, 31);
}

int main()
{
    test<input_iterator<const int*> >();
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
}
