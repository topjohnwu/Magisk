//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator Iter, Predicate<auto, Iter::value_type> Pred>
//   requires CopyConstructible<Pred>
//   constexpr Iter   // constexpr after C++17
//   find_if_not(Iter first, Iter last, Pred pred);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct ne {
    TEST_CONSTEXPR ne (int val) : v(val) {}
    TEST_CONSTEXPR bool operator () (int v2) const { return v != v2; }
    int v;
    };

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 4, 6};
    int ib[] = {1, 2, 3, 7, 5, 6};
    ne c(4);
    return    (std::find_if_not(std::begin(ia), std::end(ia), c) == ia+4)
           && (std::find_if_not(std::begin(ib), std::end(ib), c) == ib+6)
           ;
    }
#endif

int main()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned s = sizeof(ia)/sizeof(ia[0]);
    input_iterator<const int*> r = std::find_if_not(input_iterator<const int*>(ia),
                                                    input_iterator<const int*>(ia+s),
                                                    ne(3));
    assert(*r == 3);
    r = std::find_if_not(input_iterator<const int*>(ia),
                         input_iterator<const int*>(ia+s),
                         ne(10));
    assert(r == input_iterator<const int*>(ia+s));

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
