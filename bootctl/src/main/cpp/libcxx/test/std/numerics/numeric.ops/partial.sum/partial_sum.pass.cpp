//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>

// template <InputIterator InIter, OutputIterator<auto, const InIter::value_type&> OutIter>
//   requires HasPlus<InIter::value_type, InIter::reference>
//         && HasAssign<InIter::value_type,
//                      HasPlus<InIter::value_type, InIter::reference>::result_type>
//         && Constructible<InIter::value_type, InIter::reference>
//   OutIter
//   partial_sum(InIter first, InIter last, OutIter result);

#include <numeric>
#include <cassert>

#include "test_iterators.h"

template <class InIter, class OutIter>
void
test()
{
    int ia[] = {1, 2, 3, 4, 5};
    int ir[] = {1, 3, 6, 10, 15};
    const unsigned s = sizeof(ia) / sizeof(ia[0]);
    int ib[s] = {0};
    OutIter r = std::partial_sum(InIter(ia), InIter(ia+s), OutIter(ib));
    assert(base(r) == ib + s);
    for (unsigned i = 0; i < s; ++i)
        assert(ib[i] == ir[i]);
}

int main()
{
    test<input_iterator<const int*>, output_iterator<int*> >();
    test<input_iterator<const int*>, forward_iterator<int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<int*> >();
    test<input_iterator<const int*>, random_access_iterator<int*> >();
    test<input_iterator<const int*>, int*>();

    test<forward_iterator<const int*>, output_iterator<int*> >();
    test<forward_iterator<const int*>, forward_iterator<int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<int*> >();
    test<forward_iterator<const int*>, random_access_iterator<int*> >();
    test<forward_iterator<const int*>, int*>();

    test<bidirectional_iterator<const int*>, output_iterator<int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<const int*>, int*>();

    test<random_access_iterator<const int*>, output_iterator<int*> >();
    test<random_access_iterator<const int*>, forward_iterator<int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<int*> >();
    test<random_access_iterator<const int*>, int*>();

    test<const int*, output_iterator<int*> >();
    test<const int*, forward_iterator<int*> >();
    test<const int*, bidirectional_iterator<int*> >();
    test<const int*, random_access_iterator<int*> >();
    test<const int*, int*>();
}
