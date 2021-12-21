//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator Iter1, ForwardIterator Iter2,
//          Predicate<auto, Iter1::value_type, Iter2::value_type> Pred>
//   requires CopyConstructible<Pred>
//   constexpr Iter1  // constexpr after C++17
//   find_first_of(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Pred pred);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
constexpr bool test_constexpr() {
    int ia[] = {1, 2, 3};
    int ib[] = {7, 8, 9};
    int ic[] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3};
    typedef forward_iterator<int*>       FI;
    typedef bidirectional_iterator<int*> BI;
    typedef random_access_iterator<int*> RI;
    std::equal_to<int> eq{};
    return    (std::find_first_of(FI(std::begin(ic)), FI(std::end(ic)), FI(std::begin(ia)), FI(std::end(ia)), eq) == FI(ic+1))
           && (std::find_first_of(FI(std::begin(ic)), FI(std::end(ic)), FI(std::begin(ib)), FI(std::end(ib)), eq) == FI(std::end(ic)))
           && (std::find_first_of(BI(std::begin(ic)), BI(std::end(ic)), BI(std::begin(ia)), BI(std::end(ia)), eq) == BI(ic+1))
           && (std::find_first_of(BI(std::begin(ic)), BI(std::end(ic)), BI(std::begin(ib)), BI(std::end(ib)), eq) == BI(std::end(ic)))
           && (std::find_first_of(RI(std::begin(ic)), RI(std::end(ic)), RI(std::begin(ia)), RI(std::end(ia)), eq) == RI(ic+1))
           && (std::find_first_of(RI(std::begin(ic)), RI(std::end(ic)), RI(std::begin(ib)), RI(std::end(ib)), eq) == RI(std::end(ic)))
           ;
    }
#endif

int main()
{
    int ia[] = {0, 1, 2, 3, 0, 1, 2, 3};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ib[] = {1, 3, 5, 7};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    assert(std::find_first_of(input_iterator<const int*>(ia),
                              input_iterator<const int*>(ia + sa),
                              forward_iterator<const int*>(ib),
                              forward_iterator<const int*>(ib + sb),
                              std::equal_to<int>()) ==
                              input_iterator<const int*>(ia+1));
    int ic[] = {7};
    assert(std::find_first_of(input_iterator<const int*>(ia),
                              input_iterator<const int*>(ia + sa),
                              forward_iterator<const int*>(ic),
                              forward_iterator<const int*>(ic + 1),
                              std::equal_to<int>()) ==
                              input_iterator<const int*>(ia+sa));
    assert(std::find_first_of(input_iterator<const int*>(ia),
                              input_iterator<const int*>(ia + sa),
                              forward_iterator<const int*>(ic),
                              forward_iterator<const int*>(ic),
                              std::equal_to<int>()) ==
                              input_iterator<const int*>(ia+sa));
    assert(std::find_first_of(input_iterator<const int*>(ia),
                              input_iterator<const int*>(ia),
                              forward_iterator<const int*>(ic),
                              forward_iterator<const int*>(ic+1),
                              std::equal_to<int>()) ==
                              input_iterator<const int*>(ia));

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
