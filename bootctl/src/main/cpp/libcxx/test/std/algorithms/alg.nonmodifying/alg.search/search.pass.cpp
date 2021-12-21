//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter1, ForwardIterator Iter2>
//   requires HasEqualTo<Iter1::value_type, Iter2::value_type>
//   constexpr Iter1     // constexpr after C++17
//   search(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2);
//
//   template<class ForwardIterator, class Searcher>
//   ForwardIterator search(ForwardIterator first, ForwardIterator last,
//                          const Searcher& searcher); // C++17

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct MySearcherC {
    template <typename Iterator>
    std::pair<Iterator, Iterator>
    TEST_CONSTEXPR operator() (Iterator b, Iterator e) const
    {
        return std::make_pair(b, e);
    }
};

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {0, 1, 2, 3};
    int ib[] = {0, 1, 5, 3};
    int ic[] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
    return    (std::search(std::begin(ic), std::end(ic), std::begin(ia), std::end(ia)) == ic+3)
           && (std::search(std::begin(ic), std::end(ic), std::begin(ib), std::end(ib)) == std::end(ic))
           && (std::search(std::begin(ic), std::end(ic), MySearcherC()) == std::begin(ic))
           ;
    }
#endif

int searcher_called = 0;

struct MySearcher {
    template <typename Iterator>
    std::pair<Iterator, Iterator>
    operator() (Iterator b, Iterator e) const
    {
        ++searcher_called;
        return std::make_pair(b, e);
    }
};

template <class Iter1, class Iter2>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia)) == Iter1(ia));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia+1)) == Iter1(ia));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+1), Iter2(ia+2)) == Iter1(ia+1));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+2), Iter2(ia+2)) == Iter1(ia));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+2), Iter2(ia+3)) == Iter1(ia+2));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+2), Iter2(ia+3)) == Iter1(ia+2));
    assert(std::search(Iter1(ia), Iter1(ia), Iter2(ia+2), Iter2(ia+3)) == Iter1(ia));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+sa-1), Iter2(ia+sa)) == Iter1(ia+sa-1));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+sa-3), Iter2(ia+sa)) == Iter1(ia+sa-3));
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia+sa)) == Iter1(ia));
    assert(std::search(Iter1(ia), Iter1(ia+sa-1), Iter2(ia), Iter2(ia+sa)) == Iter1(ia+sa-1));
    assert(std::search(Iter1(ia), Iter1(ia+1), Iter2(ia), Iter2(ia+sa)) == Iter1(ia+1));
    int ib[] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int ic[] = {1};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(ic), Iter2(ic+1)) == Iter1(ib+1));
    int id[] = {1, 2};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(id), Iter2(id+2)) == Iter1(ib+1));
    int ie[] = {1, 2, 3};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(ie), Iter2(ie+3)) == Iter1(ib+4));
    int ig[] = {1, 2, 3, 4};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(ig), Iter2(ig+4)) == Iter1(ib+8));
    int ih[] = {0, 1, 1, 1, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    int ii[] = {1, 1, 2};
    assert(std::search(Iter1(ih), Iter1(ih+sh), Iter2(ii), Iter2(ii+3)) == Iter1(ih+3));
    int ij[] = {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0};
    const unsigned sj = sizeof(ij)/sizeof(ij[0]);
    int ik[] = {0, 0, 0, 0, 1, 1, 1, 1, 0, 0};
    const unsigned sk = sizeof(ik)/sizeof(ik[0]);
    assert(std::search(Iter1(ij), Iter1(ij+sj), Iter2(ik), Iter2(ik+sk)) == Iter1(ij+6));
}

int main()
{
    test<forward_iterator<const int*>, forward_iterator<const int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<forward_iterator<const int*>, random_access_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<const int*> >();
    test<random_access_iterator<const int*>, forward_iterator<const int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<const int*> >();

#if TEST_STD_VER > 14
{
    typedef int * RI;
    static_assert((std::is_same<RI, decltype(std::search(RI(), RI(), MySearcher()))>::value), "" );

    RI it(nullptr);
    assert(it == std::search(it, it, MySearcher()));
    assert(searcher_called == 1);
}
#endif

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
