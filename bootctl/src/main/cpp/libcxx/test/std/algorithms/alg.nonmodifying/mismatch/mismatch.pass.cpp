//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator Iter1, InputIterator Iter2>
//   requires HasEqualTo<Iter1::value_type, Iter2::value_type>
//   constexpr pair<Iter1, Iter2>   // constexpr after c++17
//   mismatch(Iter1 first1, Iter1 last1, Iter2 first2);
//
// template<InputIterator Iter1, InputIterator Iter2Pred>
//   constexpr pair<Iter1, Iter2>   // constexpr after c++17
//   mismatch(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2); // C++14

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 6, 7};
    int ib[] = {1, 3};
    int ic[] = {1, 3, 5, 7};
    typedef input_iterator<int*>         II;
    typedef bidirectional_iterator<int*> BI;

    auto p1 = std::mismatch(std::begin(ia), std::end(ia), std::begin(ic));
    if (p1.first != ia+2 || p1.second != ic+2)
        return false;

    auto p2 = std::mismatch(std::begin(ia), std::end(ia), std::begin(ic), std::end(ic));
    if (p2.first != ia+2 || p2.second != ic+2)
        return false;

    auto p3 = std::mismatch(std::begin(ib), std::end(ib), std::begin(ic));
    if (p3.first != ib+2 || p3.second != ic+2)
        return false;

    auto p4 = std::mismatch(std::begin(ib), std::end(ib), std::begin(ic), std::end(ic));
    if (p4.first != ib+2 || p4.second != ic+2)
        return false;

    auto p5 = std::mismatch(II(std::begin(ib)), II(std::end(ib)), II(std::begin(ic)));
    if (p5.first != II(ib+2) || p5.second != II(ic+2))
        return false;
    auto p6 = std::mismatch(BI(std::begin(ib)), BI(std::end(ib)), BI(std::begin(ic)), BI(std::end(ic)));
    if (p6.first != BI(ib+2) || p6.second != BI(ic+2))
        return false;

    return true;
    }
#endif

int main()
{
    int ia[] = {0, 1, 2, 2, 0, 1, 2, 3};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ib[] = {0, 1, 2, 3, 0, 1, 2, 3};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]); ((void)sb); // unused in C++11

    typedef input_iterator<const int*> II;
    typedef random_access_iterator<const int*>  RAI;

    assert(std::mismatch(II(ia), II(ia + sa), II(ib))
            == (std::pair<II, II>(II(ia+3), II(ib+3))));

    assert(std::mismatch(RAI(ia), RAI(ia + sa), RAI(ib))
            == (std::pair<RAI, RAI>(RAI(ia+3), RAI(ib+3))));

#if TEST_STD_VER > 11 // We have the four iteration version
    assert(std::mismatch(II(ia), II(ia + sa), II(ib), II(ib+sb))
            == (std::pair<II, II>(II(ia+3), II(ib+3))));

    assert(std::mismatch(RAI(ia), RAI(ia + sa), RAI(ib), RAI(ib+sb))
            == (std::pair<RAI, RAI>(RAI(ia+3), RAI(ib+3))));


    assert(std::mismatch(II(ia), II(ia + sa), II(ib), II(ib+2))
            == (std::pair<II, II>(II(ia+2), II(ib+2))));
#endif

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
