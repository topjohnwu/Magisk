//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// UNSUPPORTED: c++98, c++03, c++11

// default searcher
// template<class _ForwardIterator, class _BinaryPredicate = equal_to<>>
// class default_searcher {
// public:
//     default_searcher(_ForwardIterator __f, _ForwardIterator __l,
//                             _BinaryPredicate __p = _BinaryPredicate())
//         : __first_(__f), __last_(__l), __pred_(__p) {}
//
//     template <typename _ForwardIterator2>
//     pair<_ForwardIterator2, _ForwardIterator2>
//     operator () (_ForwardIterator2 __f, _ForwardIterator2 __l) const {
//         return std::search(__f, __l, __first_, __last_, __pred_);
//         }
//
// private:
//     _ForwardIterator __first_;
//     _ForwardIterator __last_;
//     _BinaryPredicate __pred_;
//     };


#include <experimental/algorithm>
#include <experimental/functional>
#include <cassert>

#include "test_iterators.h"

struct count_equal
{
    static unsigned count;
    template <class T>
    bool operator()(const T& x, const T& y) const
        {++count; return x == y;}
};

unsigned count_equal::count = 0;

template <typename Iter1, typename Iter2>
void do_search(Iter1 b1, Iter1 e1, Iter2 b2, Iter2 e2, Iter1 result, unsigned max_count) {
    std::experimental::default_searcher<Iter2, count_equal> s{b2, e2};
    count_equal::count = 0;
    assert(result == std::experimental::search(b1, e1, s));
    assert(count_equal::count <= max_count);
}

template <class Iter1, class Iter2>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia),      Iter2(ia),    Iter1(ia),      0);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia),      Iter2(ia+1),  Iter1(ia),      sa);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+1),    Iter2(ia+2),  Iter1(ia+1),    sa);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+2),    Iter2(ia+2),  Iter1(ia),      0);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+2),    Iter2(ia+3),  Iter1(ia+2),    sa);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+2),    Iter2(ia+3),  Iter1(ia+2),    sa);
    do_search(Iter1(ia), Iter1(ia),      Iter2(ia+2),    Iter2(ia+3),  Iter1(ia),      0);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+sa-1), Iter2(ia+sa), Iter1(ia+sa-1), sa);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia+sa-3), Iter2(ia+sa), Iter1(ia+sa-3), 3*sa);
    do_search(Iter1(ia), Iter1(ia+sa),   Iter2(ia),      Iter2(ia+sa), Iter1(ia),      sa*sa);
    do_search(Iter1(ia), Iter1(ia+sa-1), Iter2(ia),      Iter2(ia+sa), Iter1(ia+sa-1), (sa-1)*sa);
    do_search(Iter1(ia), Iter1(ia+1),    Iter2(ia),      Iter2(ia+sa), Iter1(ia+1),    sa);
    int ib[] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int ic[] = {1};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(ic), Iter2(ic+1), Iter1(ib+1), sb);
    int id[] = {1, 2};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(id), Iter2(id+2), Iter1(ib+1), sb*2);
    int ie[] = {1, 2, 3};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(ie), Iter2(ie+3), Iter1(ib+4), sb*3);
    int ig[] = {1, 2, 3, 4};
    do_search(Iter1(ib), Iter1(ib+sb), Iter2(ig), Iter2(ig+4), Iter1(ib+8), sb*4);
    int ih[] = {0, 1, 1, 1, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    int ii[] = {1, 1, 2};
    do_search(Iter1(ih), Iter1(ih+sh), Iter2(ii), Iter2(ii+3), Iter1(ih+3),  sh*3);
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
