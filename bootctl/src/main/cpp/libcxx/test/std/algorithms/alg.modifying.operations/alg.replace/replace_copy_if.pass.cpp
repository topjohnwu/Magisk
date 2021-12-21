//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter, typename OutIter,
//          Predicate<auto, InIter::value_type> Pred, class T>
//   requires OutputIterator<OutIter, InIter::reference>
//         && OutputIterator<OutIter, const T&>
//         && CopyConstructible<Pred>
//   constexpr OutIter      // constexpr after C++17
//   replace_copy_if(InIter first, InIter last, OutIter result, Pred pred, const T& new_value);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

TEST_CONSTEXPR bool equalToTwo(int v) { return v == 2; }


#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {0, 1, 2, 3, 4};
    int ib[] = {0, 0, 0, 0, 0, 0}; // one bigger
    const int expected[] = {0, 1, 5, 3, 4};

    auto it = std::replace_copy_if(std::begin(ia), std::end(ia), std::begin(ib), equalToTwo, 5);

    return it == (std::begin(ib) + std::size(ia))
        && *it == 0 // don't overwrite the last value in the output array
        && std::equal(std::begin(ib), it, std::begin(expected), std::end(expected))
        ;
    }
#endif

template <class InIter, class OutIter>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ib[sa] = {0};
    OutIter r = std::replace_copy_if(InIter(ia), InIter(ia+sa),
                                     OutIter(ib), equalToTwo, 5);
    assert(base(r) == ib + sa);
    assert(ib[0] == 0);
    assert(ib[1] == 1);
    assert(ib[2] == 5);
    assert(ib[3] == 3);
    assert(ib[4] == 4);
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

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
