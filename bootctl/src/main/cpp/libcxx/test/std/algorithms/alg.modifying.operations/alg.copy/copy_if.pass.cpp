//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter, OutputIterator<auto, InIter::reference> OutIter,
//          Predicate<auto, InIter::value_type> Pred>
//   requires CopyConstructible<Pred>
//   constexpr OutIter   // constexpr after C++17
//   copy_if(InIter first, InIter last, OutIter result, Pred pred);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

// #if TEST_STD_VER > 17
// TEST_CONSTEXPR bool test_constexpr() {
//     int ia[] = {2, 4, 6, 8, 6};
//     int ic[] = {0, 0, 0, 0, 0, 0};
//
//     auto p = std::copy_if(std::begin(ia), std::end(ia), std::begin(ic), is6);
//     return std::all_of(std::begin(ic), p, [](int a){return a == 6;})
//         && std::all_of(p, std::end(ic),   [](int a){return a == 0;})
//         ;
//     }
// #endif

struct Pred
{
    bool operator()(int i) {return i % 3 == 0;}
};

template <class InIter, class OutIter>
void
test()
{
    const unsigned N = 1000;
    int ia[N];
    for (unsigned i = 0; i < N; ++i)
        ia[i] = i;
    int ib[N] = {0};

    OutIter r = std::copy_if(InIter(ia), InIter(ia+N), OutIter(ib), Pred());
    assert(base(r) == ib+N/3+1);
    for (unsigned i = 0; i < N/3+1; ++i)
        assert(ib[i] % 3 == 0);
}

int main()
{
    test<input_iterator<const int*>, output_iterator<int*> >();
    test<input_iterator<const int*>, input_iterator<int*> >();
    test<input_iterator<const int*>, forward_iterator<int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<int*> >();
    test<input_iterator<const int*>, random_access_iterator<int*> >();
    test<input_iterator<const int*>, int*>();

    test<forward_iterator<const int*>, output_iterator<int*> >();
    test<forward_iterator<const int*>, input_iterator<int*> >();
    test<forward_iterator<const int*>, forward_iterator<int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<int*> >();
    test<forward_iterator<const int*>, random_access_iterator<int*> >();
    test<forward_iterator<const int*>, int*>();

    test<bidirectional_iterator<const int*>, output_iterator<int*> >();
    test<bidirectional_iterator<const int*>, input_iterator<int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<const int*>, int*>();

    test<random_access_iterator<const int*>, output_iterator<int*> >();
    test<random_access_iterator<const int*>, input_iterator<int*> >();
    test<random_access_iterator<const int*>, forward_iterator<int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<int*> >();
    test<random_access_iterator<const int*>, int*>();

    test<const int*, output_iterator<int*> >();
    test<const int*, input_iterator<int*> >();
    test<const int*, forward_iterator<int*> >();
    test<const int*, bidirectional_iterator<int*> >();
    test<const int*, random_access_iterator<int*> >();
    test<const int*, int*>();

// #if TEST_STD_VER > 17
//     static_assert(test_constexpr());
// #endif
}
