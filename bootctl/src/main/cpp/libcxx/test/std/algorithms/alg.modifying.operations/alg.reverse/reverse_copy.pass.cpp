//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<BidirectionalIterator InIter, OutputIterator<auto, InIter::reference> OutIter>
//   constexpr OutIter          // constexpr after C++17
//   reverse_copy(InIter first, InIter last, OutIter result);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 5, 6};
    int ib[std::size(ia)] = {0};

    auto it = std::reverse_copy(std::begin(ia), std::end(ia), std::begin(ib));

    return std::distance(std::begin(ib), it) == std::size(ia)
        && std::equal   (std::begin(ia), std::end(ia), std::rbegin(ib))
           ;
    }
#endif

template <class InIter, class OutIter>
void
test()
{
    const int ia[] = {0};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ja[sa] = {-1};
    OutIter r = std::reverse_copy(InIter(ia), InIter(ia), OutIter(ja));
    assert(base(r) == ja);
    assert(ja[0] == -1);
    r = std::reverse_copy(InIter(ia), InIter(ia+sa), OutIter(ja));
    assert(ja[0] == 0);

    const int ib[] = {0, 1};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int jb[sb] = {-1};
    r = std::reverse_copy(InIter(ib), InIter(ib+sb), OutIter(jb));
    assert(base(r) == jb+sb);
    assert(jb[0] == 1);
    assert(jb[1] == 0);

    const int ic[] = {0, 1, 2};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    int jc[sc] = {-1};
    r = std::reverse_copy(InIter(ic), InIter(ic+sc), OutIter(jc));
    assert(base(r) == jc+sc);
    assert(jc[0] == 2);
    assert(jc[1] == 1);
    assert(jc[2] == 0);

    int id[] = {0, 1, 2, 3};
    const unsigned sd = sizeof(id)/sizeof(id[0]);
    int jd[sd] = {-1};
    r = std::reverse_copy(InIter(id), InIter(id+sd), OutIter(jd));
    assert(base(r) == jd+sd);
    assert(jd[0] == 3);
    assert(jd[1] == 2);
    assert(jd[2] == 1);
    assert(jd[3] == 0);
}

int main()
{
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

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
