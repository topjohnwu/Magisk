//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<BidirectionalIterator InIter, BidirectionalIterator OutIter>
//   requires OutputIterator<OutIter, InIter::reference>
//   constexpr OutIter   // constexpr after C++17
//   copy_backward(InIter first, InIter last, OutIter result);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "user_defined_integral.hpp"

// #if TEST_STD_VER > 17
// TEST_CONSTEXPR bool test_constexpr() {
//     int ia[] = {1, 2, 3, 4, 5};
//     int ic[] = {6, 6, 6, 6, 6, 6, 6};
//
//     size_t N = std::size(ia);
//     auto p = std::copy_backward(std::begin(ia), std::end(ia), std::begin(ic) + N);
//     return std::equal(std::begin(ic), p, std::begin(ia))
//         && std::all_of(p, std::end(ic), [](int a){return a == 6;})
//         ;
//     }
// #endif

template <class InIter, class OutIter>
void
test()
{
    const unsigned N = 1000;
    int ia[N];
    for (unsigned i = 0; i < N; ++i)
        ia[i] = i;
    int ib[N] = {0};

    OutIter r = std::copy_backward(InIter(ia), InIter(ia+N), OutIter(ib+N));
    assert(base(r) == ib);
    for (unsigned i = 0; i < N; ++i)
        assert(ia[i] == ib[i]);
}

int main()
{
    test<bidirectional_iterator<const int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<const int*>, int*>();

    test<random_access_iterator<const int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<int*> >();
    test<random_access_iterator<const int*>, int*>();

    test<const int*, bidirectional_iterator<int*> >();
    test<const int*, random_access_iterator<int*> >();
    test<const int*, int*>();

// #if TEST_STD_VER > 17
//     static_assert(test_constexpr());
// #endif
}
