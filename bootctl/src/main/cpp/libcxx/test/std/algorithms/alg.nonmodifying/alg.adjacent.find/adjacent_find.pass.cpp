//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter>
//   requires EqualityComparable<Iter::value_type>
//   constexpr Iter  // constexpr after C++17
//   adjacent_find(Iter first, Iter last);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {0, 1, 2, 2, 0, 1, 2, 3};
    int ib[] = {0, 1, 2, 7, 0, 1, 2, 3};

    return  (std::adjacent_find(std::begin(ia), std::end(ia)) == ia+2)
         && (std::adjacent_find(std::begin(ib), std::end(ib)) == std::end(ib))
         ;
    }
#endif

int main()
{
    int ia[] = {0, 1, 2, 2, 0, 1, 2, 3};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    assert(std::adjacent_find(forward_iterator<const int*>(ia),
                              forward_iterator<const int*>(ia + sa)) ==
                              forward_iterator<const int*>(ia+2));
    assert(std::adjacent_find(forward_iterator<const int*>(ia),
                              forward_iterator<const int*>(ia)) ==
                              forward_iterator<const int*>(ia));
    assert(std::adjacent_find(forward_iterator<const int*>(ia+3),
                              forward_iterator<const int*>(ia + sa)) ==
                              forward_iterator<const int*>(ia+sa));

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
