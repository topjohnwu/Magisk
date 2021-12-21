//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>

// template <class ForwardIterator, class T>
//     void iota(ForwardIterator first, ForwardIterator last, T value);

#include <numeric>
#include <cassert>

#include "test_iterators.h"

template <class InIter>
void
test()
{
    int ia[] = {1, 2, 3, 4, 5};
    int ir[] = {5, 6, 7, 8, 9};
    const unsigned s = sizeof(ia) / sizeof(ia[0]);
    std::iota(InIter(ia), InIter(ia+s), 5);
    for (unsigned i = 0; i < s; ++i)
        assert(ia[i] == ir[i]);
}

int main()
{
    test<forward_iterator<int*> >();
    test<bidirectional_iterator<int*> >();
    test<random_access_iterator<int*> >();
    test<int*>();
}
