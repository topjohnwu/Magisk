//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <functional>

// template<class ForwardIterator, class BinaryPredicate = equal_to<>>
// default_searcher<ForwardIterator, BinaryPredicate>
// make_default_searcher(ForwardIterator pat_first, ForwardIterator pat_last,
//                       BinaryPredicate pred = BinaryPredicate());


#include <experimental/algorithm>
#include <experimental/functional>
#include <cassert>

#include "test_iterators.h"

template <typename Iter1, typename Iter2>
void do_search(Iter1 b1, Iter1 e1, Iter2 b2, Iter2 e2, Iter1 result) {
    assert(result == std::experimental::search(b1, e1,
        std::experimental::make_default_searcher(b2, e2)));
}

template <class Iter1, class Iter2>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia),      Iter2(ia),     Iter1(ia));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia),      Iter2(ia+1),  Iter1(ia));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+1),    Iter2(ia+2),  Iter1(ia+1));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+2),    Iter2(ia+2),  Iter1(ia));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+2),    Iter2(ia+3),  Iter1(ia+2));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+2),    Iter2(ia+3),  Iter1(ia+2));
    do_search(Iter1(ia), Iter1(ia),      Iter2(ia+2),    Iter2(ia+3),  Iter1(ia));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+sa-1), Iter2(ia+sa), Iter1(ia+sa-1));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+sa-3), Iter2(ia+sa), Iter1(ia+sa-3));
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia),      Iter2(ia+sa), Iter1(ia));
    do_search(Iter1(ia), Iter1(ia+sa-1), Iter2(ia),      Iter2(ia+sa), Iter1(ia+sa-1));
    do_search(Iter1(ia), Iter1(ia+1),    Iter2(ia),      Iter2(ia+sa), Iter1(ia+1));
    int ib[] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int ic[] = {1};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(ic), Iter2(ic+1), Iter1(ib+1));
    int id[] = {1, 2};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(id), Iter2(id+2), Iter1(ib+1));
    int ie[] = {1, 2, 3};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(ie), Iter2(ie+3), Iter1(ib+4));
    int ig[] = {1, 2, 3, 4};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(ig), Iter2(ig+4), Iter1(ib+8));
    int ih[] = {0, 1, 1, 1, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    int ii[] = {1, 1, 2};
    do_search(Iter1(ih), Iter1(ih+sh), Iter2(ii), Iter2(ii+3), Iter1(ih+3));
    int ij[] = {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0};
    const unsigned sj = sizeof(ij)/sizeof(ij[0]);
    int ik[] = {0, 0, 0, 0, 1, 1, 1, 1, 0, 0};
    const unsigned sk = sizeof(ik)/sizeof(ik[0]);
    do_search(Iter1(ij), Iter1(ij+sj), Iter2(ik), Iter2(ik+sk), Iter1(ij+6));
}

int main() {
    test<forward_iterator<const int*>, forward_iterator<const int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<forward_iterator<const int*>, random_access_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<const int*> >();
    test<random_access_iterator<const int*>, forward_iterator<const int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<const int*> >();
}
