//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template <class InputIterator, class Predicate>
//     constpexr bool       // constexpr after C++17
//     is_partitioned(InputIterator first, InputIterator last, Predicate pred);

#include <algorithm>
#include <functional>
#include <cstddef>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "counting_predicates.hpp"

struct is_odd {
  TEST_CONSTEXPR bool operator()(const int &i) const { return i & 1; }
};

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 4, 6};
    int ib[] = {1, 2, 3, 4, 5, 6};
    return     std::is_partitioned(std::begin(ia), std::end(ia), is_odd())
           && !std::is_partitioned(std::begin(ib), std::end(ib), is_odd());
    }
#endif


int main() {
  {
    const int ia[] = {1, 2, 3, 4, 5, 6};
    unary_counting_predicate<is_odd, int> pred((is_odd()));
    assert(!std::is_partitioned(input_iterator<const int *>(std::begin(ia)),
                                input_iterator<const int *>(std::end(ia)),
                                std::ref(pred)));
    assert(static_cast<std::ptrdiff_t>(pred.count()) <=
           std::distance(std::begin(ia), std::end(ia)));
  }
  {
    const int ia[] = {1, 3, 5, 2, 4, 6};
    unary_counting_predicate<is_odd, int> pred((is_odd()));
    assert(std::is_partitioned(input_iterator<const int *>(std::begin(ia)),
                               input_iterator<const int *>(std::end(ia)),
                               std::ref(pred)));
    assert(static_cast<std::ptrdiff_t>(pred.count()) <=
           std::distance(std::begin(ia), std::end(ia)));
  }
  {
    const int ia[] = {2, 4, 6, 1, 3, 5};
    unary_counting_predicate<is_odd, int> pred((is_odd()));
    assert(!std::is_partitioned(input_iterator<const int *>(std::begin(ia)),
                                input_iterator<const int *>(std::end(ia)),
                                std::ref(pred)));
    assert(static_cast<std::ptrdiff_t>(pred.count()) <=
           std::distance(std::begin(ia), std::end(ia)));
  }
  {
    const int ia[] = {1, 3, 5, 2, 4, 6, 7};
    unary_counting_predicate<is_odd, int> pred((is_odd()));
    assert(!std::is_partitioned(input_iterator<const int *>(std::begin(ia)),
                                input_iterator<const int *>(std::end(ia)),
                                std::ref(pred)));
    assert(static_cast<std::ptrdiff_t>(pred.count()) <=
           std::distance(std::begin(ia), std::end(ia)));
  }
  {
    const int ia[] = {1, 3, 5, 2, 4, 6, 7};
    unary_counting_predicate<is_odd, int> pred((is_odd()));
    assert(std::is_partitioned(input_iterator<const int *>(std::begin(ia)),
                               input_iterator<const int *>(std::begin(ia)),
                               std::ref(pred)));
    assert(static_cast<std::ptrdiff_t>(pred.count()) <=
           std::distance(std::begin(ia), std::begin(ia)));
  }
  {
    const int ia[] = {1, 3, 5, 7, 9, 11, 2};
    unary_counting_predicate<is_odd, int> pred((is_odd()));
    assert(std::is_partitioned(input_iterator<const int *>(std::begin(ia)),
                               input_iterator<const int *>(std::end(ia)),
                               std::ref(pred)));
    assert(static_cast<std::ptrdiff_t>(pred.count()) <=
           std::distance(std::begin(ia), std::end(ia)));
  }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
