//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator Iter, class T>
//   requires HasEqualTo<Iter::value_type, T>
//   constexpr Iter   // constexpr after C++17
//   find(Iter first, Iter last, const T& value);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 4, 6};
    int ib[] = {1, 2, 3, 4, 5, 6};
    return    (std::find(std::begin(ia), std::end(ia), 5) == ia+2)
           && (std::find(std::begin(ib), std::end(ib), 9) == ib+6)
           ;
    }
#endif

int main()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned s = sizeof(ia)/sizeof(ia[0]);
    input_iterator<const int*> r = std::find(input_iterator<const int*>(ia),
                                             input_iterator<const int*>(ia+s), 3);
    assert(*r == 3);
    r = std::find(input_iterator<const int*>(ia), input_iterator<const int*>(ia+s), 10);
    assert(r == input_iterator<const int*>(ia+s));

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
