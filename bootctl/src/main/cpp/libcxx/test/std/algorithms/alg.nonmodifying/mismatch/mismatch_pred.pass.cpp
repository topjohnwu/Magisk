//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator Iter1, InputIterator Iter2,
//          Predicate<auto, Iter1::value_type, Iter2::value_type> Pred>
//   requires CopyConstructible<Pred>
//   constexpr pair<Iter1, Iter2>   // constexpr after c++17
//   mismatch(Iter1 first1, Iter1 last1, Iter2 first2, Pred pred);
//
// template<InputIterator Iter1, InputIterator Iter2, Predicate Pred>
//   constexpr pair<Iter1, Iter2>   // constexpr after c++17
//   mismatch(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Pred pred); // C++14

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "counting_predicates.hpp"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool eq(int a, int b) { return a == b; }

TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 6, 7};
    int ib[] = {1, 3};
    int ic[] = {1, 3, 5, 7};
    typedef input_iterator<int*>         II;
    typedef bidirectional_iterator<int*> BI;

    auto p1 = std::mismatch(std::begin(ia), std::end(ia), std::begin(ic), eq);
    if (p1.first != ia+2 || p1.second != ic+2)
        return false;

    auto p2 = std::mismatch(std::begin(ia), std::end(ia), std::begin(ic), std::end(ic), eq);
    if (p2.first != ia+2 || p2.second != ic+2)
        return false;

    auto p3 = std::mismatch(std::begin(ib), std::end(ib), std::begin(ic), eq);
    if (p3.first != ib+2 || p3.second != ic+2)
        return false;

    auto p4 = std::mismatch(std::begin(ib), std::end(ib), std::begin(ic), std::end(ic), eq);
    if (p4.first != ib+2 || p4.second != ic+2)
        return false;

    auto p5 = std::mismatch(II(std::begin(ib)), II(std::end(ib)), II(std::begin(ic)), eq);
    if (p5.first != II(ib+2) || p5.second != II(ic+2))
        return false;
    auto p6 = std::mismatch(BI(std::begin(ib)), BI(std::end(ib)), BI(std::begin(ic)), BI(std::end(ic)), eq);
    if (p6.first != BI(ib+2) || p6.second != BI(ic+2))
        return false;

    return true;
    }
#endif


#if TEST_STD_VER > 11
#define HAS_FOUR_ITERATOR_VERSION
#endif

int main()
{
    int ia[] = {0, 1, 2, 2, 0, 1, 2, 3};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ib[] = {0, 1, 2, 3, 0, 1, 2, 3};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]); ((void)sb); // unused in C++11

    typedef input_iterator<const int*> II;
    typedef random_access_iterator<const int*>  RAI;
    typedef std::equal_to<int> EQ;

    assert(std::mismatch(II(ia), II(ia + sa), II(ib), EQ())
            == (std::pair<II, II>(II(ia+3), II(ib+3))));
    assert(std::mismatch(RAI(ia), RAI(ia + sa), RAI(ib), EQ())
            == (std::pair<RAI, RAI>(RAI(ia+3), RAI(ib+3))));

    binary_counting_predicate<EQ, int> bcp((EQ()));
    assert(std::mismatch(RAI(ia), RAI(ia + sa), RAI(ib), std::ref(bcp))
            == (std::pair<RAI, RAI>(RAI(ia+3), RAI(ib+3))));
    assert(bcp.count() > 0 && bcp.count() < sa);
    bcp.reset();

#if TEST_STD_VER >= 14
    assert(std::mismatch(II(ia), II(ia + sa), II(ib), II(ib + sb), EQ())
            == (std::pair<II, II>(II(ia+3), II(ib+3))));
    assert(std::mismatch(RAI(ia), RAI(ia + sa), RAI(ib), RAI(ib + sb), EQ())
            == (std::pair<RAI, RAI>(RAI(ia+3), RAI(ib+3))));

    assert(std::mismatch(II(ia), II(ia + sa), II(ib), II(ib + sb), std::ref(bcp))
            == (std::pair<II, II>(II(ia+3), II(ib+3))));
    assert(bcp.count() > 0 && bcp.count() < std::min(sa, sb));
#endif

    assert(std::mismatch(ia, ia + sa, ib, EQ()) ==
           (std::pair<int*,int*>(ia+3,ib+3)));

#if TEST_STD_VER >= 14
    assert(std::mismatch(ia, ia + sa, ib, ib + sb, EQ()) ==
           (std::pair<int*,int*>(ia+3,ib+3)));
    assert(std::mismatch(ia, ia + sa, ib, ib + 2, EQ()) ==
           (std::pair<int*,int*>(ia+2,ib+2)));
#endif

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
