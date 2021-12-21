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
//   constexpr bool     // constexpr after c++17
//   equal(Iter1 first1, Iter1 last1, Iter2 first2, Pred pred);
//
// Introduced in C++14:
// template<InputIterator Iter1, InputIterator Iter2,
//          Predicate<auto, Iter1::value_type, Iter2::value_type> Pred>
//   requires CopyConstructible<Pred>
//   constexpr bool     // constexpr after c++17
//   equal(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Pred pred);


#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool eq(int a, int b) { return a == b; }

TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 6, 7};
    int ib[] = {1, 3};
    int ic[] = {1, 3, 5, 7};
    typedef input_iterator<int*>         II;
    typedef bidirectional_iterator<int*> BI;

    return !std::equal(std::begin(ia), std::end(ia), std::begin(ic)              , eq)
        && !std::equal(std::begin(ia), std::end(ia), std::begin(ic), std::end(ic), eq)
        &&  std::equal(std::begin(ib), std::end(ib), std::begin(ic)              , eq)
        && !std::equal(std::begin(ib), std::end(ib), std::begin(ic), std::end(ic), eq)

        &&  std::equal(II(std::begin(ib)), II(std::end(ib)), II(std::begin(ic))                  , eq)
        && !std::equal(BI(std::begin(ib)), BI(std::end(ib)), BI(std::begin(ic)), BI(std::end(ic)), eq)
        ;
    }
#endif


int comparison_count = 0;
template <typename T>
bool counting_equals ( const T &a, const T &b ) {
    ++comparison_count;
    return a == b;
}

int main()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned s = sizeof(ia)/sizeof(ia[0]);
    int ib[s] = {0, 1, 2, 5, 4, 5};
    assert(std::equal(input_iterator<const int*>(ia),
                      input_iterator<const int*>(ia+s),
                      input_iterator<const int*>(ia),
                      std::equal_to<int>()));
#if TEST_STD_VER >= 14
    assert(std::equal(input_iterator<const int*>(ia),
                      input_iterator<const int*>(ia+s),
                      input_iterator<const int*>(ia),
                      input_iterator<const int*>(ia+s),
                      std::equal_to<int>()));
    assert(std::equal(random_access_iterator<const int*>(ia),
                      random_access_iterator<const int*>(ia+s),
                      random_access_iterator<const int*>(ia),
                      random_access_iterator<const int*>(ia+s),
                      std::equal_to<int>()));

    comparison_count = 0;
    assert(!std::equal(input_iterator<const int*>(ia),
                      input_iterator<const int*>(ia+s),
                      input_iterator<const int*>(ia),
                      input_iterator<const int*>(ia+s-1),
                      counting_equals<int>));
    assert(comparison_count > 0);
    comparison_count = 0;
    assert(!std::equal(random_access_iterator<const int*>(ia),
                      random_access_iterator<const int*>(ia+s),
                      random_access_iterator<const int*>(ia),
                      random_access_iterator<const int*>(ia+s-1),
                      counting_equals<int>));
    assert(comparison_count == 0);
#endif
    assert(!std::equal(input_iterator<const int*>(ia),
                       input_iterator<const int*>(ia+s),
                       input_iterator<const int*>(ib),
                       std::equal_to<int>()));
#if TEST_STD_VER >= 14
    assert(!std::equal(input_iterator<const int*>(ia),
                       input_iterator<const int*>(ia+s),
                       input_iterator<const int*>(ib),
                       input_iterator<const int*>(ib+s),
                       std::equal_to<int>()));
    assert(!std::equal(random_access_iterator<const int*>(ia),
                       random_access_iterator<const int*>(ia+s),
                       random_access_iterator<const int*>(ib),
                       random_access_iterator<const int*>(ib+s),
                       std::equal_to<int>()));
#endif

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
